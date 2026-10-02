#include "../../source.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "Movement.hpp"
#include "../../Utils/Math.h"
#include "../Rage/AntiAim.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../Utils/InputSys.hpp"
#include "../Rage/FakeLag.hpp"
#include "../Rage/Ragebot.hpp"
#include "../Rage/Zeusbot.hpp"
#include "../Rage/AntiAim.hpp"
#include "../Rage/KnifeBot.hpp"
#include "../Game/Prediction.hpp"
#include "../../SDK/Displacement.hpp"
#include "../Game/SetupBones.hpp"
#include "../../Utils/Threading/threading.h"
#include "../Rage/TickbaseShift.hpp"
#include "../Visuals/EventLogger.hpp"

// todo: move this
C_AnimationLayer FakeAnimLayers[ 13 ];

extern Vector AutoPeekPos;

extern matrix3x4_t HeadBone;

const int GetNextUpdate() {
	C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

	if (!LocalPlayer || LocalPlayer->IsDead())
		return 0;

	auto current_tick = TIME_TO_TICKS(LocalPlayer->m_nTickBase() * Interfaces::m_pGlobalVars->interval_per_tick);
	auto update_tick = TIME_TO_TICKS(g_Vars.globals.m_flBodyPred);

	return update_tick - current_tick;
}

void SimulateMovement( C_SimulationData& data ) {
	if( !( data.m_iFlags & FL_ONGROUND ) ) {
		data.m_vecVeloctity.z -= ( Interfaces::m_pGlobalVars->interval_per_tick * g_Vars.sv_gravity->GetFloat( ) * 0.5f );
	}
	else if( data.m_bJumped ) {
		data.m_bJumped = false;
		data.m_vecVeloctity.z = g_Vars.sv_jump_impulse->GetFloat( );
		data.m_iFlags &= ~FL_ONGROUND;
	}

	// can't step up onto very steep slopes
	static const float MIN_STEP_NORMAL = 0.7f;

	if( !data.m_vecVeloctity.IsZero( ) ) {
		auto collidable = data.m_player->GetCollideable( );
		const Vector mins = collidable->OBBMins( );
		const Vector max = collidable->OBBMaxs( );

		const Vector src = data.m_vecOrigin;
		Vector end = src + ( data.m_vecVeloctity * Interfaces::m_pGlobalVars->interval_per_tick );

		Ray_t ray;
		ray.Init( src, end, mins, max );

		CGameTrace trace;
		CTraceFilter filter;
		filter.pSkip = data.m_player;

		Interfaces::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

		// CGameMovement::TryPlayerMove
		if( trace.fraction != 1.f ) {
			// BUGFIXME: is it should be 4? ( not 2 )
			for( int i = 0; i < 2; i++ ) {
				// decompose velocity into plane
				data.m_vecVeloctity -= trace.plane.normal * data.m_vecVeloctity.Dot( trace.plane.normal );

				const float dot = data.m_vecVeloctity.Dot( trace.plane.normal );
				if( dot < 0.f ) { // moving against plane
					data.m_vecVeloctity.x -= dot * trace.plane.normal.x;
					data.m_vecVeloctity.y -= dot * trace.plane.normal.y;
					data.m_vecVeloctity.z -= dot * trace.plane.normal.z;
				}

				end = trace.endpos + ( data.m_vecVeloctity * ( Interfaces::m_pGlobalVars->interval_per_tick * ( 1.f - trace.fraction ) ) );

				ray.Init( trace.endpos, end, mins, max );
				Interfaces::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );
				if( trace.fraction == 1.f )
					break;
			}
		}

		data.m_vecOrigin = trace.endpos;
		end = trace.endpos;
		end.z -= 2.f;

		ray.Init( data.m_vecOrigin, end, mins, max );
		Interfaces::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &trace );

		data.m_iFlags &= ~FL_ONGROUND;

		if( trace.DidHit( ) && trace.plane.normal.z >= MIN_STEP_NORMAL ) {
			data.m_iFlags |= FL_ONGROUND;
		}

		if( data.m_iFlags & FL_ONGROUND )
			data.m_vecVeloctity.z = 0.0f;
		else
			data.m_vecVeloctity.z -= Interfaces::m_pGlobalVars->interval_per_tick * g_Vars.sv_gravity->GetFloat( ) * 0.5f;
	}
}

void ExtrapolatePlayer( C_SimulationData& data, int simulationTicks, const Vector& wishvel, float wishspeed, float maxSpeed ) {
	for( int i = 0; i < simulationTicks; ++i ) {
		SimulateMovement( data );

		if( data.m_vecVeloctity.IsZero( ) )
			break;

		auto Accelerate = [ &data ] ( const Vector& wishdir, float wishspeed, float maxSpeed ) {
			float addspeed, accelspeed, currentspeed;

			// See if we are changing direction a bit
			currentspeed = data.m_vecVeloctity.Dot( wishdir );

			// Reduce wishspeed by the amount of veer.
			addspeed = wishspeed - currentspeed;

			// If not going to add any speed, done.
			if( addspeed <= 0 )
				return;

			// Determine amount of accleration.
			static int playerSurfaceFrictionOffset = SDK::Memory::FindInDataMap( data.m_player->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
			float playerSurfaceFriction = *( float* )( uintptr_t( data.m_player ) + playerSurfaceFrictionOffset );
			accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Interfaces::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

			// Cap at addspeed
			if( accelspeed > addspeed )
				accelspeed = addspeed;

			// Adjust velocity.
			for( int i = 0; i < 3; i++ ) {
				data.m_vecVeloctity[ i ] += accelspeed * wishdir[ i ];
			}

			float len = data.m_vecVeloctity.Length2D( );
			if( len > ( ( data.m_iFlags & FL_ONGROUND ) ? ( maxSpeed ) : 320.0f ) ) {
				float z = data.m_vecVeloctity.z;
				data.m_vecVeloctity.z = 0.0f;
				data.m_vecVeloctity = data.m_vecVeloctity.Normalized( ) * ( ( data.m_iFlags & FL_ONGROUND ) ? ( maxSpeed ) : 320.0f );
				data.m_vecVeloctity.z = z;
			}
		};

		Accelerate( wishvel, wishspeed, maxSpeed );
	}
}

void RotateMovement( Encrypted_t<CUserCmd> cmd, QAngle wish_angle, QAngle old_angles ) {
	// aimware movement fix, reversed by ph4ge/senator for gucci
	if( old_angles.x != wish_angle.x || old_angles.y != wish_angle.y || old_angles.z != wish_angle.z ) {
		Vector wish_forward, wish_right, wish_up, cmd_forward, cmd_right, cmd_up;

		auto viewangles = old_angles;
		auto movedata = Vector( cmd->forwardmove, cmd->sidemove, cmd->upmove );
		viewangles.Normalize( );

		if( viewangles.z != 0.f ) {
			auto pLocal = C_CSPlayer::GetLocalPlayer( );

			if( pLocal && !( pLocal->m_fFlags( ) & FL_ONGROUND ) )
				movedata.y = 0.f;
		}

		wish_forward = wish_angle.ToVectors( &wish_right, &wish_up );
		cmd_forward = viewangles.ToVectors( &cmd_right, &cmd_up );

		auto v8 = sqrt( wish_forward.x * wish_forward.x + wish_forward.y * wish_forward.y ), v10 = sqrt( wish_right.x * wish_right.x + wish_right.y * wish_right.y ), v12 = sqrt( wish_up.z * wish_up.z );

		Vector wish_forward_norm( 1.0f / v8 * wish_forward.x, 1.0f / v8 * wish_forward.y, 0.f ),
			wish_right_norm( 1.0f / v10 * wish_right.x, 1.0f / v10 * wish_right.y, 0.f ),
			wish_up_norm( 0.f, 0.f, 1.0f / v12 * wish_up.z );

		auto v14 = sqrt( cmd_forward.x * cmd_forward.x + cmd_forward.y * cmd_forward.y ), v16 = sqrt( cmd_right.x * cmd_right.x + cmd_right.y * cmd_right.y ), v18 = sqrt( cmd_up.z * cmd_up.z );

		Vector cmd_forward_norm( 1.0f / v14 * cmd_forward.x, 1.0f / v14 * cmd_forward.y, 1.0f / v14 * 0.0f ),
			cmd_right_norm( 1.0f / v16 * cmd_right.x, 1.0f / v16 * cmd_right.y, 1.0f / v16 * 0.0f ),
			cmd_up_norm( 0.f, 0.f, 1.0f / v18 * cmd_up.z );

		auto v22 = wish_forward_norm.x * movedata.x, v26 = wish_forward_norm.y * movedata.x, v28 = wish_forward_norm.z * movedata.x, v24 = wish_right_norm.x * movedata.y, v23 = wish_right_norm.y * movedata.y, v25 = wish_right_norm.z * movedata.y, v30 = wish_up_norm.x * movedata.z, v27 = wish_up_norm.z * movedata.z, v29 = wish_up_norm.y * movedata.z;

		cmd->forwardmove = cmd_forward_norm.x * v24 + cmd_forward_norm.y * v23 + cmd_forward_norm.z * v25 + ( cmd_forward_norm.x * v22 + cmd_forward_norm.y * v26 + cmd_forward_norm.z * v28 ) + ( cmd_forward_norm.y * v30 + cmd_forward_norm.x * v29 + cmd_forward_norm.z * v27 );
		cmd->sidemove = cmd_right_norm.x * v24 + cmd_right_norm.y * v23 + cmd_right_norm.z * v25 + ( cmd_right_norm.x * v22 + cmd_right_norm.y * v26 + cmd_right_norm.z * v28 ) + ( cmd_right_norm.x * v29 + cmd_right_norm.y * v30 + cmd_right_norm.z * v27 );
		cmd->upmove = cmd_up_norm.x * v23 + cmd_up_norm.y * v24 + cmd_up_norm.z * v25 + ( cmd_up_norm.x * v26 + cmd_up_norm.y * v22 + cmd_up_norm.z * v28 ) + ( cmd_up_norm.x * v30 + cmd_up_norm.y * v29 + cmd_up_norm.z * v27 );

		cmd->forwardmove = Math::Clamp( cmd->forwardmove, -g_Vars.cl_forwardspeed->GetFloat( ), g_Vars.cl_forwardspeed->GetFloat( ) );
		cmd->sidemove = Math::Clamp( cmd->sidemove, -g_Vars.cl_sidespeed->GetFloat( ), g_Vars.cl_sidespeed->GetFloat( ) );
		cmd->upmove = Math::Clamp( cmd->upmove, -g_Vars.cl_upspeed->GetFloat( ), g_Vars.cl_upspeed->GetFloat( ) );
	}
}

namespace Interfaces
{
	struct MovementData {
		Encrypted_t<CUserCmd> m_pCmd = nullptr;
		C_CSPlayer* m_pLocal = nullptr;
		bool* m_pSendPacket = nullptr;
		bool* m_pFinalPacket = nullptr;

		uintptr_t* m_pCLMoveReturn = nullptr;

		bool m_bInRecursion = false;
		int m_iRecursionTicks = 0;

		QAngle m_angRenderViewangles{ };
		QAngle m_angPreviousAngles{ };
		QAngle m_angMovementAngle{ };
		float m_flOldYaw = 0.0f;
		float m_flJumpFall = 0.0f;

		int nBhops = 0;
		int m_fPrePredFlags = 0;

		C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB;
		C_AnimationLayer* ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL;

		int   m_bStopPlayer = 0;
		bool  m_bMinimalSpeed = false;
		bool  m_bFastStop = false;
		bool  m_bStoppedTick = false;
		bool  m_bFakeducking = false;
		bool  m_bStopOk = false;
		bool  jumped_last_tick = false;
		bool  should_fake_jump = false;

		float m_flLowerBodyUpdateTime = 0.0f;

		int m_iLatency = 0;
	};

	static MovementData _move_data;

	class C_Movement : public Movement {
	public:
		virtual void PrePrediction( Encrypted_t<CUserCmd> cmd, C_CSPlayer* pLocal, bool* pSendPacket, bool* bFinalPacket, uintptr_t* cl_move );
		virtual void InPrediction( );
		virtual void PostPrediction( );
		virtual void InstantStop( CUserCmd* cmd = nullptr );
		virtual void ThirdPerson( );

		virtual float GetLBYUpdateTime( ) {
			return  m_movement_data->m_flLowerBodyUpdateTime;
		}

		virtual int GetButtons( ) {
			return m_movement_data->m_pCmd->buttons;
		}

		virtual QAngle& GetMovementAngle( ) {
			return m_movement_data->m_angMovementAngle;
		}

		virtual bool StopPlayer( ) {
			if( m_movement_data->m_bStoppedTick ) {
				return false;
			}

			return AutoStopInternal( );
		}

		virtual void StopPlayerAtMinimalSpeed( ) {
			m_movement_data->m_bMinimalSpeed = true;
		}

		virtual void AutoStop( int ticks = 1 );
		virtual bool GetStopState( );
		virtual bool CreateMoveRecursion( );

		C_Movement( ) : m_movement_data( &_move_data ) { ; };
		virtual ~C_Movement( ) { };

	private:
		Encrypted_t<MovementData> m_movement_data;

		void MoveExploit( );
		bool AutoStopInternal( );
		void AutoJump( );
		void AutoStrafe( );
		void SlowWalk( float speed, bool force_accurate = false );
	};

	Movement* Movement::Get( ) {
		static C_Movement movement;
		return &movement;
	}

	void C_Movement::PrePrediction( Encrypted_t<CUserCmd> cmd, C_CSPlayer* pLocal, bool* pSendPacket, bool* bFinalPacket, uintptr_t* cl_move ) {
		m_movement_data->m_pCmd = cmd;
		m_movement_data->m_pLocal = pLocal;
		m_movement_data->m_pSendPacket = pSendPacket;
		m_movement_data->m_pFinalPacket = bFinalPacket;
		m_movement_data->m_angRenderViewangles = m_movement_data->m_pCmd->viewangles;

		m_movement_data->m_pCLMoveReturn = cl_move;
		m_movement_data->m_bStopOk = false;
		m_movement_data->m_bStoppedTick = false;

		// static int autostop_ticks = 0;

		if( !pLocal || pLocal->IsDead( ) )
			return;

		{
			float delta = std::fabsf( Interfaces::m_pEngine->GetNetChannelInfo( )->GetLatency( FLOW_OUTGOING ) - Interfaces::m_pEngine->GetNetChannelInfo( )->GetAvgLatency( FLOW_OUTGOING ) );
			m_movement_data->m_iLatency = TIME_TO_TICKS( delta ) + 1 - **( int** )Engine::Displacement.Data.m_uHostFrameTicks;
		}

		m_movement_data->m_angMovementAngle = m_movement_data->m_pCmd->viewangles;
		m_movement_data->m_fPrePredFlags = m_movement_data->m_pLocal->m_fFlags( );

		if( g_Vars.misc.fastduck )
			m_movement_data->m_pCmd->buttons |= IN_BULLRUSH;

		if( g_Vars.misc.duckjump ) {
			if( m_movement_data->m_pCmd->buttons & IN_JUMP ) {
				if( !( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) )
					m_movement_data->m_pCmd->buttons |= IN_DUCK;
			}
		}

		if( g_Vars.misc.minijump && !( ( m_movement_data->m_pCmd->buttons & IN_DUCK ) ) ) {
			if( !( m_movement_data->m_pCmd->buttons & IN_JUMP ) || ( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) || m_movement_data->m_pLocal->m_vecVelocity( ).z >= -140.0f )
				m_movement_data->m_pCmd->buttons &= ~IN_DUCK;
			else
				m_movement_data->m_pCmd->buttons |= IN_DUCK;
		}

		g_Vars.globals.Fakewalking = false;

		if( g_Vars.misc.autojump )
			AutoJump( );

		if( g_Vars.misc.autostrafer )
			AutoStrafe( );

		bool instant_stop = ( g_Vars.misc.instant_stop && g_Vars.misc.instant_stop_key.enabled );
		if( instant_stop || ( (g_Vars.misc.quickstop || g_Vars.antiaim.anti_lastmove) && !g_Vars.globals.WasShootingInPeek && m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND && !( m_movement_data->m_pCmd->buttons & IN_JUMP ) && m_movement_data->m_pLocal->m_vecVelocity( ).Length( ) >= 1.2f ) ) {
			if( instant_stop || ( !( m_movement_data->m_pCmd->buttons & IN_JUMP ) && m_movement_data->m_pCmd->forwardmove == m_movement_data->m_pCmd->sidemove && m_movement_data->m_pCmd->sidemove == 0.0f ) ) {
				if (g_Vars.antiaim.anti_lastmove) {
					g_Vars.globals.need_break_lastmove = true;
				}

				m_movement_data->m_bStopPlayer = 1;
				m_movement_data->m_bMinimalSpeed = false;
			}
		}

		if (!g_Vars.antiaim.anti_lastmove)
			g_Vars.globals.need_break_lastmove = false;

		if (g_TickbaseController.Using() && g_Vars.rage.double_tap_duck && (g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
			if (!(m_movement_data->m_pCmd->buttons & IN_BULLRUSH))
				m_movement_data->m_pCmd->buttons |= IN_BULLRUSH;

			m_movement_data->m_pCmd->buttons |= IN_DUCK;
		}

		bool peek = true;
		if( g_Vars.misc.autopeek && g_Vars.globals.WasShootingInPeek && !AutoPeekPos.IsZero( ) ) {
			cmd->buttons &= ~IN_JUMP;

			auto delta = AutoPeekPos - m_movement_data->m_pLocal->GetAbsOrigin( );
			m_movement_data->m_angMovementAngle = delta.ToEulerAngles( );

			peek = false;

			m_movement_data->m_pCmd->forwardmove = g_Vars.cl_forwardspeed->GetFloat( );
			m_movement_data->m_pCmd->sidemove = 0.0f;

			if( delta.Length2D( ) <= std::fmaxf( 11.f, m_movement_data->m_pLocal->m_vecVelocity( ).Length2D( ) * Interfaces::m_pGlobalVars->interval_per_tick ) ) {
				float maxSpeed = m_movement_data->m_pLocal->GetMaxSpeed( );
				static int playerSurfaceFrictionOffset = SDK::Memory::FindInDataMap( m_movement_data->m_pLocal->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
				float playerSurfaceFriction = *( float* )( uintptr_t( m_movement_data->m_pLocal ) + playerSurfaceFrictionOffset );
				float max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Interfaces::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;

				m_movement_data->m_bStopPlayer = int( m_movement_data->m_pLocal->m_vecVelocity( ).Length2D( ) / max_accelspeed ) - 1;
				m_movement_data->m_bMinimalSpeed = false;

				peek = true;
				g_Vars.globals.WasShootingInPeek = false;
			}
		}
		else {
			g_Vars.globals.WasShootingInPeek = false;
		}

		if( peek ) {
			if( m_movement_data->m_bStopPlayer && AutoStopInternal( ) ) {
				m_movement_data->m_bStoppedTick = true;
			}
		}

		if( !m_movement_data->m_bStoppedTick ) {
			C_WeaponCSBaseGun* Weapon = ( C_WeaponCSBaseGun* )m_movement_data->m_pLocal->m_hActiveWeapon( ).Get( );
			if( Weapon ) {
				auto weaponInfo = Weapon->GetCSWeaponData( );
				if( g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled ) {

					if( Weapon ) {
						if( weaponInfo.IsValid( ) ) {
							float flMaxSpeed = m_movement_data->m_pLocal->m_bIsScoped( ) > 0 ? weaponInfo.Xor( )->m_flMaxSpeed2 : weaponInfo.Xor( )->m_flMaxSpeed;
							float flDesiredSpeed = ( flMaxSpeed * 0.33000001 );
							if( g_Vars.misc.slow_walk_custom ) {
								SlowWalk( flDesiredSpeed * ( g_Vars.misc.slow_walk_speed / 100.f ) );
							}
							else {
								SlowWalk( flDesiredSpeed );
							}
						}
					}
				}
				else if( g_Vars.misc.accurate_walk && m_movement_data->m_pCmd->buttons & IN_SPEED ) {
					if( Weapon ) {
						if( weaponInfo.IsValid( ) ) {
							float flMaxSpeed = m_movement_data->m_pLocal->m_bIsScoped( ) > 0 ? weaponInfo.Xor( )->m_flMaxSpeed2 : weaponInfo.Xor( )->m_flMaxSpeed;
							float flDesiredSpeed = ( flMaxSpeed * 0.33000001 );

							SlowWalk( flDesiredSpeed, true );
						}
					}
				}
			}
		}

		m_movement_data->m_bStopPlayer--;
		m_movement_data->m_bMinimalSpeed = false;
		if( m_movement_data->m_bStopPlayer < 0 )
			m_movement_data->m_bStopPlayer = 0;

		Interfaces::AntiAimbot::Get( )->PrePrediction( m_movement_data->m_pSendPacket, cmd );

		m_movement_data->m_pCmd->viewangles.Normalize( );

		if (g_Vars.misc.balls) {
			m_movement_data->m_pCmd->viewangles = pLocal->m_angEyeAngles();
			m_movement_data->m_angMovementAngle = pLocal->m_angEyeAngles();

			m_movement_data->m_pCmd->viewangles.Clamp();
			m_movement_data->m_angMovementAngle.Clamp();
		}

		RotateMovement( m_movement_data->m_pCmd, m_movement_data->m_angMovementAngle, m_movement_data->m_pCmd->viewangles );
	}

	void C_Movement::InPrediction( ) {
		if( !m_movement_data->m_pLocal || m_movement_data->m_pLocal->IsDead( ) || !m_movement_data->m_pSendPacket )
			return;

		if( g_Vars.misc.edgejump && g_Vars.misc.edgejump_bind.enabled &&
			Engine::Prediction::Instance( ).GetFlags( ) & FL_ONGROUND && !( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
			m_movement_data->m_pCmd->buttons |= IN_JUMP;
		}

		auto animState = m_movement_data->m_pLocal->m_PlayerAnimState( );
		if( !animState )
			return;

		m_movement_data->m_pLocal->SetAbsAngles( QAngle( 0.f, animState->m_flAbsRotation, 0.f ) );
		m_movement_data->m_pLocal->InvalidateBoneCache( );

		const float PitchPosBackup = *( float* )( uintptr_t( m_movement_data->m_pLocal ) + Engine::Displacement.DT_BaseAnimating.m_flPoseParameter + 48 );

		*( float* )( uintptr_t( m_movement_data->m_pLocal ) + Engine::Displacement.DT_BaseAnimating.m_flPoseParameter + 48 ) = 0.5f;

		g_BoneSetup.BuildBones( m_movement_data->m_pLocal, BONE_USED_BY_ANYTHING, BoneSetupFlags::None );


		//OLD EYEPOS
		//g_Vars.globals.m_vecFixedEyePosition = m_movement_data->m_pLocal->GetEyePosition( );

		*( float* )( uintptr_t( m_movement_data->m_pLocal ) + Engine::Displacement.DT_BaseAnimating.m_flPoseParameter + 48 ) = PitchPosBackup;

		bool bDontFakelag = g_Vars.misc.move_exploit_key.enabled && g_Vars.misc.move_exploit && g_Vars.globals.bMoveExploiting;

		//MoveExploit( );

		if( g_TickbaseController.Using( ) ) {
			bool bShouldLag = g_TickbaseController.s_nExtraProcessingTicks > 1;
			*m_movement_data->m_pSendPacket = !bShouldLag;
		}
		else if( !g_TickbaseController.Building( ) && g_Vars.fakelag.enabled ) {
			if( !bDontFakelag )
				Interfaces::FakeLag::Get( )->Main( m_movement_data->m_pSendPacket, m_movement_data->m_pCmd );
		}
	
		g_Vars.globals.bInRagebot = Interfaces::Ragebot::Get( )->Run( m_movement_data->m_pCmd, m_movement_data->m_pLocal, m_movement_data->m_pSendPacket );

		// don't lag when shooting, this way events are instant
		if( g_Vars.globals.m_bOldShot && !g_Vars.globals.Fakewalking && !(g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit) ) {
			*m_movement_data->m_pSendPacket = true;
		}
		
		Interfaces::AntiAimbot::Get( )->Main( m_movement_data->m_pSendPacket, m_movement_data->m_pFinalPacket, m_movement_data->m_pCmd, g_Vars.globals.bInRagebot );

		g_Vars.globals.m_StoredAngle = m_movement_data->m_pCmd->viewangles;

		Interfaces::KnifeBot::Get( )->Main( m_movement_data->m_pCmd, m_movement_data->m_pSendPacket );
		Interfaces::ZeusBot::Get( )->Main( m_movement_data->m_pCmd, m_movement_data->m_pSendPacket );
		g_Vars.globals.m_bInCreateMove = false;
	}

	void C_Movement::PostPrediction( ) {
		if( !m_movement_data->m_pLocal )
			return;

		if( !m_movement_data->m_pCmd.IsValid( ) )
			return;

		m_movement_data->m_pCmd->forwardmove = Math::Clamp( m_movement_data->m_pCmd->forwardmove, -g_Vars.cl_forwardspeed->GetFloat( ), g_Vars.cl_forwardspeed->GetFloat( ) );
		m_movement_data->m_pCmd->sidemove = Math::Clamp( m_movement_data->m_pCmd->sidemove, -g_Vars.cl_sidespeed->GetFloat( ), g_Vars.cl_sidespeed->GetFloat( ) );
		m_movement_data->m_pCmd->upmove = Math::Clamp( m_movement_data->m_pCmd->upmove, -g_Vars.cl_upspeed->GetFloat( ), g_Vars.cl_upspeed->GetFloat( ) );
		m_movement_data->m_pCmd->viewangles.Normalize( );
		m_movement_data->m_pCmd->viewangles.Clamp( );

		float delta_x = std::remainderf( m_movement_data->m_pCmd->viewangles.x - m_movement_data->m_angPreviousAngles.x, 360.0f );
		float delta_y = std::remainderf( m_movement_data->m_pCmd->viewangles.y - m_movement_data->m_angPreviousAngles.y, 360.0f );

		if( delta_x != 0.0f ) {
			float mouse_y = -( ( delta_x / g_Vars.m_pitch->GetFloat( ) ) / g_Vars.sensitivity->GetFloat( ) );
			short mousedy;
			if( mouse_y <= 32767.0f ) {
				if( mouse_y >= -32768.0f ) {
					if( mouse_y >= 1.0f || mouse_y < 0.0f ) {
						if( mouse_y <= -1.0f || mouse_y > 0.0f )
							mousedy = static_cast< short >( mouse_y );
						else
							mousedy = -1;
					}
					else {
						mousedy = 1;
					}
				}
				else {
					mousedy = 0x8000u;
				}
			}
			else {
				mousedy = 0x7FFF;
			}

			m_movement_data->m_pCmd->mousedy = mousedy;
		}

		if( delta_y != 0.0f ) {
			float mouse_x = -( ( delta_y / g_Vars.m_yaw->GetFloat( ) ) / g_Vars.sensitivity->GetFloat( ) );
			short mousedx;
			if( mouse_x <= 32767.0f ) {
				if( mouse_x >= -32768.0f ) {
					if( mouse_x >= 1.0f || mouse_x < 0.0f ) {
						if( mouse_x <= -1.0f || mouse_x > 0.0f )
							mousedx = static_cast< short >( mouse_x );
						else
							mousedx = -1;
					}
					else {
						mousedx = 1;
					}
				}
				else {
					mousedx = 0x8000u;
				}
			}
			else {
				mousedx = 0x7FFF;
			}

			m_movement_data->m_pCmd->mousedx = mousedx;
		}

		RotateMovement( m_movement_data->m_pCmd, m_movement_data->m_angRenderViewangles, m_movement_data->m_pCmd->viewangles );

		static int nGroundTicks = 0;
		bool bhopped = ( m_movement_data->m_fPrePredFlags & FL_ONGROUND ) && !( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND );
		if( ( m_movement_data->m_fPrePredFlags & FL_ONGROUND ) && ( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
			nGroundTicks++;

			if( nGroundTicks > 1 ) {
				if( m_movement_data->nBhops > 0 ) {
					m_movement_data->nBhops = 0;
				}
			}
		}
		else {
			if( bhopped ) {
				m_movement_data->nBhops++;
			}

			nGroundTicks = 0;
		}

		if( m_movement_data->nBhops > 1 ) {
			// mad? nn
			g_Vars.globals.m_flJumpFall += ( 1.0f / 0.85f ) * Interfaces::m_pGlobalVars->frametime;
		}
		else {
			g_Vars.globals.m_flJumpFall = 0.f;
		}

		g_Vars.globals.m_flJumpFall = std::clamp<float>( g_Vars.globals.m_flJumpFall, 0.f, 1.0f );

		m_movement_data->m_angPreviousAngles = m_movement_data->m_pCmd->viewangles;
	}

	void C_Movement::InstantStop( CUserCmd* cmd ) {
		float maxSpeed = m_movement_data->m_pLocal->GetMaxSpeed( );
		Vector velocity = m_movement_data->m_pLocal->m_vecVelocity( );
		velocity.z = 0.0f;

		CUserCmd* ucmd = cmd ? cmd : m_movement_data->m_pCmd.Xor( );

		float speed = velocity.Length2D( );

		static int playerSurfaceFrictionOffset = SDK::Memory::FindInDataMap( m_movement_data->m_pLocal->GetPredDescMap( ), XorStr( "m_surfaceFriction" ) );
		float playerSurfaceFriction = *( float* )( uintptr_t( m_movement_data->m_pLocal ) + playerSurfaceFrictionOffset );
		float max_accelspeed = g_Vars.sv_accelerate->GetFloat( ) * Interfaces::m_pGlobalVars->interval_per_tick * maxSpeed * playerSurfaceFriction;
		if( speed - max_accelspeed <= -1.f ) {
			m_movement_data->m_bStopPlayer = 0;
			ucmd->forwardmove = speed / max_accelspeed;
		}
		else {
			ucmd->forwardmove = g_Vars.cl_forwardspeed->GetFloat( );
		}

		ucmd->sidemove = 0.0f;

		QAngle move_dir = m_movement_data->m_angMovementAngle;

		float direction = atan2( velocity.y, velocity.x );
		move_dir.yaw = std::remainderf( ToDegrees( direction ) + 180.0f, 360.0f );
		RotateMovement( ucmd, move_dir, ucmd->viewangles );
	}

	bool C_Movement::AutoStopInternal( ) {
		if( !( g_Vars.misc.instant_stop && g_Vars.misc.instant_stop_key.enabled ) && ( ( m_movement_data->m_pCmd->buttons & IN_JUMP ) || !( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) )
			return false;

		CVariables::RAGE* rbot = nullptr;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return false;

		auto weapon = (C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get();
		if (!weapon)
			return false;

		auto weaponInfo = weapon->GetCSWeaponData();
		if (!weaponInfo.IsValid())
			return false;

		auto id = weapon->m_iItemDefinitionIndex();
		if (id == WEAPON_ZEUS)
			return false;

		switch (weaponInfo->m_iWeaponType) {
		case WEAPONTYPE_PISTOL:
			if (id == WEAPON_DEAGLE || id == WEAPON_REVOLVER)
				rbot = &g_Vars.rage_heavypistols;
			else
				rbot = &g_Vars.rage_pistols;
			break;
		case WEAPONTYPE_SUBMACHINEGUN:
			rbot = &g_Vars.rage_smgs;
			break;
		case WEAPONTYPE_RIFLE:
			rbot = &g_Vars.rage_rifles;
			break;
		case WEAPONTYPE_SHOTGUN:
			rbot = &g_Vars.rage_shotguns;
			break;
		case WEAPONTYPE_SNIPER_RIFLE:
			if (id == WEAPON_G3SG1 || id == WEAPON_SCAR20)
				rbot = &g_Vars.rage_autosnipers;
			else
				rbot = (id == WEAPON_AWP) ? &g_Vars.rage_awp : &g_Vars.rage_scout;
			break;
		case WEAPONTYPE_MACHINEGUN:
			rbot = &g_Vars.rage_heavys;
			break;
		default:
			rbot = &g_Vars.rage_default;
			break;
		}

		float maxSpeed = m_movement_data->m_pLocal->GetMaxSpeed( );
		Vector velocity = Engine::Prediction::Instance( )->GetVelocity( );
		velocity.z = 0.0f;

		float speed = velocity.Length2D( );

		//bool fullstop = true;
		//if( m_movement_data->m_bMinimalSpeed && ( m_movement_data->m_pCmd->forwardmove != 0.f || m_movement_data->m_pCmd->sidemove != 0.f ) && speed <= maxSpeed * 0.33000001 ) {
		//	fullstop = false;
		//}

		//fullstop = speed > ( maxSpeed * 0.33000001 );

		//if( !( g_Vars.misc.instant_stop && g_Vars.misc.instant_stop_key.enabled ) && fullstop && speed <= 15.0f && ( m_movement_data->m_pCmd->forwardmove != 0.f || m_movement_data->m_pCmd->sidemove != 0.f ) ) {
		//	m_movement_data->m_pCmd->forwardmove = 0.f;
		//	m_movement_data->m_pCmd->sidemove = 0.f;
		//	return true;
		//}

		bool pressing_move_keys = ( m_movement_data->m_pCmd->buttons & IN_FORWARD || m_movement_data->m_pCmd->buttons & IN_MOVELEFT || m_movement_data->m_pCmd->buttons & IN_BACK || m_movement_data->m_pCmd->buttons & IN_MOVERIGHT );
		if( ( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) && g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled && pressing_move_keys ) {
			return false;
		}

		if (rbot->between_shots && !local->CanShoot(false))
			return false;

		if (rbot->always_stop || m_movement_data->m_bStopPlayer == 1 || g_Vars.globals.need_break_lastmove) {
			InstantStop();
			return true;
		}
		else if (speed <= maxSpeed) {
			SlowWalk(maxSpeed * 0.33000001, true);
		}

		return true;
	}

	void C_Movement::AutoJump( ) {
		if( m_movement_data->m_pLocal->m_MoveType( ) == MOVETYPE_LADDER || m_movement_data->m_pLocal->m_MoveType( ) == MOVETYPE_NOCLIP )
			return;

		if( g_Vars.rage.enabled ) {
			if( !( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) ) {
				m_movement_data->m_pCmd->buttons &= ~IN_JUMP;
			}
			return;
		}

		if( !m_movement_data->jumped_last_tick && m_movement_data->should_fake_jump ) {
			m_movement_data->should_fake_jump = false;
			m_movement_data->m_pCmd->buttons |= IN_JUMP;
		}
		else if( m_movement_data->m_pCmd->buttons & IN_JUMP ) {
			if( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) {
				m_movement_data->jumped_last_tick = true;
				m_movement_data->should_fake_jump = true;
			}
			else {
				m_movement_data->m_pCmd->buttons &= ~IN_JUMP;
				m_movement_data->jumped_last_tick = false;
			}
		}
		else {
			m_movement_data->jumped_last_tick = false;
			m_movement_data->should_fake_jump = false;
		}
	}

	void C_Movement::AutoStrafe( ) {
		if( ( g_Vars.misc.instant_stop && g_Vars.misc.instant_stop_key.enabled ) )
			return;

		if( ( m_movement_data->m_pLocal->m_fFlags( ) & FL_ONGROUND ) && !( m_movement_data->m_pCmd->buttons & IN_JUMP ) )
			return;

		if( m_movement_data->m_pLocal->m_MoveType( ) != MOVETYPE_WALK )
			return;

		if (g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled) {
			InstantStop();
			return;
		}

		static auto side = 1.0f;
		side = -side;

		auto velocity = m_movement_data->m_pLocal->m_vecVelocity( );
		velocity.z = 0.0f;

		auto speed = velocity.Length2D( );
		auto ideal_strafe = Math::Clamp( ToDegrees( atan( 15.f / speed ) ), 0.0f, 90.0f );

		if( g_Vars.misc.autostrafer_wasd && ( m_movement_data->m_pCmd->forwardmove != 0.0f || m_movement_data->m_pCmd->sidemove != 0.0f ) ) {
			// took this idea from stacker, thank u !!!!
			enum EDirections {
				FORWARDS = 0,
				BACKWARDS = 180,
				LEFT = 90,
				RIGHT = -90,
				BACK_LEFT = 135,
				BACK_RIGHT = -135
			};

			float wish_dir{ };

			// get our key presses.
			bool holding_w = m_movement_data->m_pCmd->buttons & IN_FORWARD;
			bool holding_a = m_movement_data->m_pCmd->buttons & IN_MOVELEFT;
			bool holding_s = m_movement_data->m_pCmd->buttons & IN_BACK;
			bool holding_d = m_movement_data->m_pCmd->buttons & IN_MOVERIGHT;

			// move in the appropriate direction.
			if( holding_w ) {
				//	forward left
				if( holding_a ) {
					wish_dir += ( EDirections::LEFT / 2 );
				}
				//	forward right
				else if( holding_d ) {
					wish_dir += ( EDirections::RIGHT / 2 );
				}
				//	forward
				else {
					wish_dir += EDirections::FORWARDS;
				}
			}
			else if( holding_s ) {
				//	back left
				if( holding_a ) {
					wish_dir += EDirections::BACK_LEFT;
				}
				//	back right
				else if( holding_d ) {
					wish_dir += EDirections::BACK_RIGHT;
				}
				//	back
				else {
					wish_dir += EDirections::BACKWARDS;
				}

				// cancel out any forwardmove values.
				m_movement_data->m_pCmd->forwardmove = 0.f;
			}
			else if( holding_a ) {
				//	left
				wish_dir += EDirections::LEFT;
			}
			else if( holding_d ) {
				//	right
				wish_dir += EDirections::RIGHT;
			}

			m_movement_data->m_angMovementAngle.yaw += std::remainderf( wish_dir, 360.f );

			m_movement_data->m_pCmd->sidemove = 0.f;
		}

		// cancel out any forwardmove values.
		m_movement_data->m_pCmd->forwardmove = 0.f;

		auto yaw_delta = std::remainderf( m_movement_data->m_angMovementAngle.yaw - m_movement_data->m_flOldYaw, 360.0f );
		auto abs_angle_delta = abs( yaw_delta );
		m_movement_data->m_flOldYaw = m_movement_data->m_angMovementAngle.yaw;

		if( abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f ) {
			auto velocity_direction = velocity.ToEulerAngles( );
			auto velocity_delta = std::remainderf( m_movement_data->m_angMovementAngle.yaw - velocity_direction.yaw, 360.0f );
			if( velocity_delta <= ideal_strafe || speed <= 15.0f ) {
				if( -( ideal_strafe ) <= velocity_delta || speed <= 15.0f ) {
					m_movement_data->m_angMovementAngle.yaw += side * ideal_strafe;
					m_movement_data->m_pCmd->sidemove = 450 * side;
				}
				else {
					m_movement_data->m_angMovementAngle.yaw = velocity_direction.yaw - ideal_strafe;
					m_movement_data->m_pCmd->sidemove = 450;
				}
			}
			else {
				m_movement_data->m_angMovementAngle.yaw = velocity_direction.yaw + ideal_strafe;
				m_movement_data->m_pCmd->sidemove = -450;
			}
		}
		else if( yaw_delta > 0.0f ) {
			m_movement_data->m_pCmd->sidemove = -450;
		}
		else if( yaw_delta < 0.0f ) {
			m_movement_data->m_pCmd->sidemove = 450;
		}

		m_movement_data->m_angMovementAngle.Normalize( );
	}

	void C_Movement::SlowWalk( float speed, bool force_accurate ) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

		if( !LocalPlayer || LocalPlayer->IsDead( ) )
			return;

		// not on ground dont fakewalk.
		if (!(m_movement_data->m_pLocal->m_fFlags() & FL_ONGROUND)) {
			return;
		}

		if (!(g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit) && !force_accurate && !(g_Vars.misc.autopeek && g_Vars.misc.autopeek_bind.enabled)) {

			Vector velocity{ LocalPlayer->m_vecVelocity() };
			int    ticks{ };

			auto sv_friction = Interfaces::m_pCvar->FindVar(XorStr("sv_friction"));
			auto sv_stopspeed = Interfaces::m_pCvar->FindVar(XorStr("sv_stopspeed"));

			//if( g_TickbaseController.s_nExtraProcessingTicks ) {
			//	g_Vars.globals.Fakewalking = false;
			//	return;
			//}

			// calculate friction.
			float friction = sv_friction->GetFloat() * LocalPlayer->m_surfaceFriction();

			// check for the amount of ticks when breaking LBY.
			int pred_ticks = GetNextUpdate() - 1;

			//if (!(g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
			*m_movement_data->m_pSendPacket = Interfaces::m_pClientState->m_nChokedCommands() > g_Vars.misc.slow_walk_speed;
			//}

			if (Interfaces::m_pClientState->m_nChokedCommands() > g_Vars.misc.slow_walk_speed) {
				*m_movement_data->m_pSendPacket = true;
				g_Vars.globals.bCanWeaponFire = false;
				//printf("sent packet\n");
			}

			m_movement_data->m_pCmd->buttons &= ~IN_SPEED;

			for (ticks; ticks < g_Vars.misc.slow_walk_speed; ++ticks) {
				// calculate speed.
				float speed = velocity.Length();

				// if too slow return.
				if (speed <= 0.1f)
					break;

				// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
				float control = std::max(speed, sv_stopspeed->GetFloat());

				// calculate the drop amount.
				float drop = control * friction * Interfaces::m_pGlobalVars->interval_per_tick;

				// scale the velocity.
				float newspeed = std::max(0.f, speed - drop);

				if (newspeed != speed) {
					// determine proportion of old speed we are using.
					newspeed /= speed;

					// adjust velocity according to proportion.
					velocity *= newspeed;
				}
			}

			g_Vars.globals.Fakewalking = true;

			const int max = std::min<int>(g_Vars.misc.slow_walk_speed, pred_ticks);
			const int choked = Interfaces::m_pClientState->m_nChokedCommands();
			int ticks_to_pred = max - choked;

			if (choked < max || ticks) {
				*m_movement_data->m_pSendPacket = false;
			}

			// zero forwardmove and sidemove.
			if (ticks > ticks_to_pred || !Interfaces::m_pClientState->m_nChokedCommands() || *m_movement_data->m_pSendPacket) {
				InstantStop();
				g_Vars.globals.updatingPacket = true;
			}
			else
				g_Vars.globals.updatingPacket = false;
		}
		else {
			C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)LocalPlayer->m_hActiveWeapon().Get();
			auto weapon_info = Weapon->GetCSWeaponData();

			if (weapon_info.IsValid()) {
				auto modifier = g_Vars.misc.slowwalk_speed == -1.0f ? 0.3f : g_Vars.misc.slowwalk_speed; //-V550
				auto max_speed = modifier * (LocalPlayer->m_bIsScoped() ? weapon_info->m_flMaxSpeed2 : weapon_info->m_flMaxSpeed);

				auto move_length = sqrt(m_movement_data->m_pCmd->sidemove * m_movement_data->m_pCmd->sidemove + m_movement_data->m_pCmd->forwardmove * m_movement_data->m_pCmd->forwardmove);

				auto forwardmove = m_movement_data->m_pCmd->forwardmove / move_length;
				auto sidemove = m_movement_data->m_pCmd->sidemove / move_length;

				if (move_length > max_speed)
				{
					if (max_speed + 1.0f < LocalPlayer->m_vecVelocity().Length2D())
					{
						m_movement_data->m_pCmd->forwardmove = 0.0f;
						m_movement_data->m_pCmd->sidemove = 0.0f;
					}
					else
					{
						m_movement_data->m_pCmd->sidemove = max_speed * sidemove;
						m_movement_data->m_pCmd->forwardmove = max_speed * forwardmove;
					}
				}
			}

		}
	}

	void C_Movement::ThirdPerson( ) {
		if( !m_movement_data->m_pLocal )
			return;

		static float progress{ };

		auto local = C_CSPlayer::GetLocalPlayer( );
		if( !local || local != m_movement_data->m_pLocal )
			return;

		// for whatever reason overrideview also gets called from the main menu.
		if( !Interfaces::m_pEngine->IsInGame( ) )
			return;

		// check if local player is alive.
		bool alive = local->IsAlive( );

		if (alive) {
			C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)m_movement_data->m_pLocal->m_hActiveWeapon().Get();

			if (!Weapon)
				return;

			auto weaponInfo = Weapon->GetCSWeaponData();
			if (!weaponInfo.IsValid())
				return;

			if (weaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE && g_Vars.misc.third_person_on_grenade) {
				Interfaces::m_pInput->CAM_ToFirstPerson();
				Interfaces::m_pInput->m_fCameraInThirdPerson = false;
				return;
			}
		}

		if (!alive) {
			if (Interfaces::m_pInput->CAM_IsThirdPerson()) {
				Interfaces::m_pInput->CAM_ToFirstPerson();
				Interfaces::m_pInput->m_vecCameraOffset.z = 0.f;
				Interfaces::m_pInput->m_fCameraInThirdPerson = false;
			}

			return;
		}

		// camera should be in thirdperson.
		if( g_Vars.misc.third_person && g_Vars.misc.third_person_bind.enabled ) {

			// if alive and not in thirdperson already switch to thirdperson.
			if( alive && !Interfaces::m_pInput->CAM_IsThirdPerson( ) ) {
				Interfaces::m_pInput->CAM_ToThirdPerson( );
			}

			// if dead and spectating in firstperson switch to thirdperson.
			else if( m_movement_data->m_pLocal->m_iObserverMode( ) == 4 ) {

				// if in thirdperson, switch to firstperson.
				// we need to disable thirdperson to spectate properly.
				if( Interfaces::m_pInput->CAM_IsThirdPerson( ) ) {
					Interfaces::m_pInput->CAM_ToFirstPerson( );
					Interfaces::m_pInput->m_vecCameraOffset.z = 0.f;
					Interfaces::m_pInput->m_fCameraInThirdPerson = false;
				}

				m_movement_data->m_pLocal->m_iObserverMode( ) = 5;
			}
		}

		// camera should be in firstperson.
		else {
			// animate backwards.
			progress -= Interfaces::m_pGlobalVars->frametime * 7.f + (progress / 100);

			// clamp.
			Math::clampSupremacy(progress, 0.f, 1.f);
			Interfaces::m_pInput->m_vecCameraOffset.z = g_Vars.misc.third_person_dist * progress;

			// set to first person.
			if (!progress)
				Interfaces::m_pInput->CAM_ToFirstPerson();
		}

		// if after all of this we are still in thirdperson.
		if( Interfaces::m_pInput->CAM_IsThirdPerson( ) ) {
			// get camera angles.
			QAngle offset;
			Interfaces::m_pEngine->GetViewAngles( offset );

			// get our viewangle's forward directional vector.
			Vector forward;
			Math::AngleVectors( offset, forward );

			if(g_Vars.misc.third_person_bind.enabled)
				progress += Interfaces::m_pGlobalVars->frametime * 7.f + (progress / 100);

			Math::clampSupremacy(progress, 0.f, 1.f);

			offset.z = g_Vars.misc.third_person_dist * progress;

			Vector offsetd = m_movement_data->m_pLocal->m_vecViewOffset( );

			// start pos.
			Vector origin = m_movement_data->m_pLocal->GetAbsOrigin( ) + offsetd;

			// setup trace filter and trace.
			CTraceFilterWorldAndPropsOnly filter;
			CGameTrace tr;

			Interfaces::m_pEngineTrace->TraceRay(
				Ray_t( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
				MASK_NPCWORLDSTATIC,
				( ITraceFilter* )&filter,
				&tr
			);

			// adapt distance to travel time.
			Math::Clamp( tr.fraction, 0.f, 1.f );
			offset.z *= tr.fraction;

			// override camera angles.
			Interfaces::m_pInput->m_vecCameraOffset = { offset.x, offset.y, offset.z };

			m_movement_data->m_pLocal->UpdateVisibilityAllEntities( );
		}
	}

	void C_Movement::AutoStop( int ticks ) {
		m_movement_data->m_bStopPlayer = ticks;
	}

	bool C_Movement::GetStopState( ) {
		return m_movement_data->m_bStopOk;
	}

	bool C_Movement::CreateMoveRecursion( ) {
		if( !m_movement_data->m_bInRecursion || m_movement_data->m_iRecursionTicks <= 0 ) {
			m_movement_data->m_bInRecursion = false;
			m_movement_data->m_iRecursionTicks = 0;
			return false;
		}

		m_movement_data->m_iRecursionTicks--;
		return true;
	}

	void C_Movement::MoveExploit( ) {
		static bool bDisallow = false;
		static bool bSaveOrigin = true;

		if( !g_Vars.misc.move_exploit ) {
			g_Vars.globals.vecExploitOrigin.Init();
			bDisallow = false;
			bSaveOrigin = true;
			g_Vars.globals.bMoveExploiting = false;
			return;
		}

		if( !g_Vars.misc.move_exploit_key.enabled ) {
			g_Vars.globals.vecExploitOrigin.Init( );
			bDisallow = false;
			bSaveOrigin = true;
			g_Vars.globals.bMoveExploiting = false;
			return;
		}

		if( m_movement_data->m_pLocal->m_vecVelocity( ).Length2D( ) < m_movement_data->m_pLocal->GetMaxSpeed( ) - 20.f ) {
			g_Vars.globals.vecExploitOrigin.Init( );
			g_Vars.globals.bMoveExploiting = false;
			bSaveOrigin = true;
			return;
		}

		if( Interfaces::m_pClientState->m_nChokedCommands( ) < 100 ) {
			if( !bDisallow ) {
				*m_movement_data->m_pSendPacket = false;
				g_Vars.globals.bMoveExploiting = true;
			}

			if( bSaveOrigin ) {
				g_Vars.globals.vecExploitOrigin = m_movement_data->m_pLocal->GetEyePosition( ) + ( m_movement_data->m_pLocal->m_vecVelocity( ) * 100 * Interfaces::m_pGlobalVars->interval_per_tick );
				bSaveOrigin = false;
			}
		}
		else {
			bSaveOrigin = true;
			g_Vars.globals.bMoveExploiting = false;
		}
	}
}