#include "AnimationSystem.hpp"
#include "../../SDK/displacement.hpp"
#include "../../Utils/Math.h"
#include "../Game/SetupBones.hpp"
#include "../../Utils/Threading/threading.h"
#include "LagCompensation.hpp"
#include "Resolver.hpp"

#define MT_SETUP_BONES

void recalculate_velocity(Encrypted_t<Engine::C_AnimationData> anim_data, Encrypted_t<Engine::C_AnimationRecord> record, C_BasePlayer* m_player, Encrypted_t<Engine::C_AnimationRecord> previous)
{
	static /*const*/ ConVar* sv_gravity = g_Vars.sv_gravity;
	static /*const*/ ConVar* sv_jump_impulse = g_Vars.sv_jump_impulse;
	static /*const*/ ConVar* sv_enablebunnyhopping = Interfaces::m_pCvar->FindVar(XorStr("sv_enablebunnyhopping"));

	/* fix z velocity if enemy is in air. */
	/* https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1697 */

	//auto& old_origin = *(Vector*)(uintptr_t(m_player) + 0x3A4);

	if (record->m_fFlags & FL_ONGROUND
		&& record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flWeight > 0.0f
		&& record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flWeight < 1.0f)
	{
		// float val = clamp ( ( speed - 0.55f ) / ( 0.9f - 0.55f), 0.f, 1.f );
		// layer11_weight = 1.f - val;
		auto val = (1.0f - record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flWeight) * 0.35f;

		if (val > 0.0f && val < 1.0f)
			record->animation_speed = val + 0.55f;
		else
			record->animation_speed = -1.f;
	}

	if (_fdtest(&record->m_vecVelocity.x) > 0
		|| _fdtest(&record->m_vecVelocity.y) > 0
		|| _fdtest(&record->m_vecVelocity.z) > 0)
		record->m_vecVelocity.Init();

	if (!record->m_dormant && previous.IsValid() && !previous->dormant())
	{
		//
		//	calculate new velocity based on (new_origin - old_origin) / (new_time - old_time) formula.
		//
		if (record->m_iChokeTicks > 1 && record->m_iChokeTicks <= 20)
			record->m_vecVelocity = (record->m_vecOrigin - previous->m_vecOrigin) / record->m_flChokeTime;

		if (abs(record->m_vecVelocity.x) < 0.001f)
			record->m_vecVelocity.x = 0.0f;
		if (abs(record->m_vecVelocity.y) < 0.001f)
			record->m_vecVelocity.y = 0.0f;
		if (abs(record->m_vecVelocity.z) < 0.001f)
			record->m_vecVelocity.z = 0.0f;

		if (_fdtest(&record->m_vecVelocity.x) > 0
			|| _fdtest(&record->m_vecVelocity.y) > 0
			|| _fdtest(&record->m_vecVelocity.z) > 0)
			record->m_vecVelocity.Init();

		auto curr_direction = RAD2DEG(std::atan2f(record->m_vecVelocity.y, record->m_vecVelocity.x));
		auto prev_direction = !previous.IsValid() ? FLT_MAX : RAD2DEG(std::atan2f(previous->m_vecVelocity.y, previous->m_vecVelocity.x));

		auto delta = Math::normalize_angle(curr_direction - prev_direction);

		//if (record->m_vecVelocity.Length2D() > 0.1f) {
		//	if (previous->m_vecVelocity.Length2D() > 0.1f && abs(delta) >= 60.f)
		//		r_log->last_time_changed_direction = csgo.m_globals()->realtime;
		//}
		//else
		//	r_log->last_time_changed_direction = 0;

		//
		// these requirements pass only when layer[6].weight is accurate to normalized velocity.
		//
		if (record->m_fFlags & FL_ONGROUND
			&& record->m_vecVelocity.Length2D() >= 0.1f
			&& std::abs(delta) < 1.0f
			&& std::abs(record->m_flDuckAmount - previous->m_flDuckAmount) <= 0.0f
			&& record->m_serverAnimOverlays[6].m_flPlaybackRate > previous->m_serverAnimOverlays[6].m_flPlaybackRate
			&& record->m_serverAnimOverlays[6].m_flWeight > previous->m_serverAnimOverlays[6].m_flWeight)
		{
			auto weight_speed = record->m_serverAnimOverlays[6].m_flWeight;

			if (weight_speed <= 0.7f && weight_speed > 0.0f)
			{
				if (record->m_serverAnimOverlays[6].m_flPlaybackRate == 0.0f)
					record->m_vecVelocity.Init();
				else
				{
					const auto m_post_velocity_lenght = record->m_vecVelocity.Length2D();

					if (m_post_velocity_lenght != 0.0f)
					{
						float mult = 1;
						if (record->m_fFlags & 6)
							mult = 0.34f;
						else if (m_player->m_bIsWalking())
							mult = 0.52f;

						record->m_vecVelocity.x = (record->m_vecVelocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
						record->m_vecVelocity.y = (record->m_vecVelocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
					}
				}
			}
		}

		//
		// fix velocity with fakelag.
		//
		if (record->m_fFlags & FL_ONGROUND && record->m_vecVelocity.Length2D() > 0.1f && record->m_iChokeTicks > 1)
		{
			//
			// get velocity lenght from 11th layer calc.
			//
			if (record->animation_speed > 0) {
				const auto m_pre_velocity_lenght = record->m_vecVelocity.Length2D();
				C_WeaponCSBaseGun* weapon = (C_WeaponCSBaseGun*)m_player->m_hActiveWeapon().Get();

				if (weapon) {
					auto wdata = weapon->GetCSWeaponData();
					if (wdata.IsValid()) {
						auto adjusted_velocity = (record->animation_speed * record->max_current_speed) / m_pre_velocity_lenght;
						record->m_vecVelocity.x *= adjusted_velocity;
						record->m_vecVelocity.y *= adjusted_velocity;
					}
				}
			}

			/*if (record->entity_flags & FL_ONGROUND && (sv_enablebunnyhopping && !sv_enablebunnyhopping->GetBool() || previous->entity_flags & FL_ONGROUND)) {
				auto max_speed = record->max_current_speed;

				if (record->entity_flags & 6)
					max_speed *= 0.34f;
				else if (record->fake_walking)
					max_speed *= 0.52f;

				if (max_speed < m_pre_velocity_lenght)
					record->velocity *= (max_speed / m_pre_velocity_lenght);

				if (previous->entity_flags & FL_ONGROUND)
					record->velocity.z = 0.f;
			}*/
		}

		if (anim_data->m_AnimationRecord.size() > 2 && record->m_iChokeTicks > 1 && !record->dormant()
			&& previous->m_vecVelocity.Length() > 0 && !(record->m_fFlags & FL_ONGROUND && previous->m_fFlags & FL_ONGROUND))
		{
			auto pre_pre_record = &anim_data->m_AnimationRecord.at(2);

			if (!pre_pre_record->dormant()) {
				//if (record->velocity.Length2D() > (record->max_current_speed * 0.52f) && previous->velocity.Length2D() > (record->max_current_speed * 0.52f)
				//	|| record->velocity.Length2D() <= (record->max_current_speed * 0.52f) && previous->velocity.Length2D() <= (record->max_current_speed * 0.52f))
				//{
				//	auto manually_calculated = log->tick_records[(log->records_count - 2) & 63].stop_to_full_run_frac;
				//	manually_calculated += (record->velocity.Length2D() > (record->max_current_speed * 0.52f) ? (2.f * previous->time_delta) : -(2.f * previous->time_delta));

				//	manually_calculated = Math::clamp(manually_calculated, 0, 1);

				//	if (abs(manually_calculated - previous->stop_to_full_run_frac) >= 0.1f)// {
				//		m_player->get_animation_state()->m_walk_run_transition = manually_calculated;
				//}

				const auto prev_direction = RAD2DEG(std::atan2f(previous->m_vecVelocity.y, previous->m_vecVelocity.x));

				auto real_velocity = record->m_vecVelocity.Length2D();

				float delta = curr_direction - prev_direction;

				if (delta <= 180.0f)
				{
					if (delta < -180.0f)
						delta = delta + 360.0f;
				}
				else
				{
					delta = delta - 360.0f;
				}

				float v63 = delta * 0.5f + curr_direction;

				auto direction = (v63 + 90.f) * 0.017453292f;

				record->m_vecVelocity.x = sinf(direction) * real_velocity;
				record->m_vecVelocity.y = cosf(direction) * real_velocity;
			}
		}

		//bool is_jumping = record->entity_flags & FL_ONGROUND && previous && previous->data_filled && !previous->dormant && !(previous->entity_flags & FL_ONGROUND);

		/*if (is_jumping && record->ground_accel_last_time != record->simulation_time)
		{
			if (sv_enablebunnyhopping->GetInt() == 0) {

				// 260 x 1.1 = 286 units/s.
				float max = m_player->m_flMaxSpeed() * 1.1f;

				// get current velocity.
				float speed = record->velocity.Length();

				// reset velocity to 286 units/s.
				if (max > 0.f && speed > max)
					record->velocity *= (max / speed);
			}

			// assume the player is bunnyhopping here so set the upwards impulse.
			record->velocity.z = sv_jump_impulse->GetFloat();

			record->in_jump = true;
		}
		else */if (!(record->m_fFlags & FL_ONGROUND))
		{
			record->m_vecVelocity.z -= sv_gravity->GetFloat() * record->m_flChokeTime * 0.5f;

			//record->in_jump = true;
		}
	}
	else if (record->dormant())
	{
		auto weight_speed = record->m_serverAnimOverlays[6].m_flWeight;

		if (record->m_serverAnimOverlays[6].m_flPlaybackRate < 0.00001f)
			record->m_vecVelocity.Init();
		else
		{
			const auto m_post_velocity_lenght = record->m_vecVelocity.Length2D();

			if (m_post_velocity_lenght != 0.0f && weight_speed > 0.01f && weight_speed < 0.95f)
			{
				float mult = 1;
				if (record->m_fFlags & 6)
					mult = 0.34f;
				else if (m_player->m_bIsWalking())
					mult = 0.52f;

				record->m_vecVelocity.x = (record->m_vecVelocity.x / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
				record->m_vecVelocity.y = (record->m_vecVelocity.y / m_post_velocity_lenght) * (weight_speed * (record->max_current_speed * mult));
			}
		}

		if (record->m_fFlags & FL_ONGROUND)
			record->m_vecVelocity.z = 0;
	}

	if (_fdtest(&record->m_vecVelocity.x) > 0
		|| _fdtest(&record->m_vecVelocity.y) > 0
		|| _fdtest(&record->m_vecVelocity.z) > 0)
		record->m_vecVelocity.Init();
	//
	//	if server had 0 velocity at animation time -> reset velocity
	//
	//if (record->m_fFlags & FL_ONGROUND && record->m_iChokeTicks > 1 && record->m_vecVelocity.Length() > 0.1f && record->m_serverAnimOverlays[6].m_flPlaybackRate < 0.00001f)
	//	record->m_vecVelocity.Init();

	//m_player->invalidate_anims(4);

	/* apply proper velocity and force flags so game will not try to recalculate it. */
	//m_player->m_vecAbsVelocity() = record->velocity;
	//m_player->m_vecVelocity() = record->m_vecVelocity;
	//m_player->invalidate_anims(VELOCITY_CHANGED);

	//*(Vector*)(uintptr_t(m_player) + 0x114) = record->velocity;
}

void player_extrapolation(C_CSPlayer* e, Vector& vecorigin, Vector& vecvelocity, int& fFlags, bool bOnGround, int ticks)
{
	Vector start, end;
	Ray_t ray;
	CTraceFilterWorldAndPropsOnly filter;
	CGameTrace trace;
	// define trace start.
	start = vecorigin;

	// move trace end one tick into the future using predicted velocity.
	end = start + (vecvelocity * Interfaces::m_pGlobalVars->interval_per_tick);

	// trace.
	ray.Init(start, end, e->OBBMins(), e->OBBMaxs());
	Interfaces::m_pEngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	// we hit shit
	// we need to fix shit.
	if (trace.fraction != 1.f) {

		// fix sliding on planes.
		for (int i{ }; i < ticks; ++i) {
			vecvelocity -= trace.plane.normal * vecvelocity.Dot(trace.plane.normal);

			float adjust = vecvelocity.Dot(trace.plane.normal);
			if (adjust < 0.f)
				vecvelocity -= (trace.plane.normal * adjust);

			start = trace.endpos;
			end = start + (vecvelocity * (Interfaces::m_pGlobalVars->interval_per_tick * (1.f - trace.fraction)));

			ray.Init(start, end, e->OBBMins(), e->OBBMaxs());
			Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

			if (trace.fraction == 1.f)
				break;
		}
	}

	// set new final origin.
	start = end = vecorigin = trace.endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	ray.Init(start, end, e->OBBMins(), e->OBBMaxs());
	Interfaces::m_pEngineTrace->TraceRay(ray, CONTENTS_SOLID, &filter, &trace);

	// strip onground flag.
	fFlags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if (trace.fraction != 1.f && trace.plane.normal.z > 0.7f)
		fFlags |= FL_ONGROUND;
}

namespace Engine
{
	struct SimulationRestore {
		int m_fFlags;
		float m_flDuckAmount;
		float m_flFeetCycle;
		float m_flFeetYawRate;
		QAngle m_angEyeAngles;
		Vector m_vecOrigin;

		void Setup( C_CSPlayer* player ) {
			m_fFlags = player->m_fFlags( );
			m_flDuckAmount = player->m_flDuckAmount( );
			m_vecOrigin = player->m_vecOrigin( );
			m_angEyeAngles = player->m_angEyeAngles( );

			auto animState = player->m_PlayerAnimState( );
			m_flFeetCycle = animState->m_flFeetCycle;
			m_flFeetYawRate = animState->m_flFeetYawRate;
		}

		void Apply( C_CSPlayer* player ) const {
			player->m_fFlags( ) = m_fFlags;
			player->m_flDuckAmount( ) = m_flDuckAmount;
			player->m_vecOrigin( ) = m_vecOrigin;
			player->m_angEyeAngles( ) = m_angEyeAngles;

			auto animState = player->m_PlayerAnimState( );
			animState->m_flFeetCycle = m_flFeetCycle;
			animState->m_flFeetYawRate = m_flFeetYawRate;
		}
	};

	struct AnimationBackup {

		CCSGOPlayerAnimState anim_state;
		C_AnimationLayer layers[ 13 ];
		float pose_params[ 19 ];

		AnimationBackup( ) {

		}

		void Apply( C_CSPlayer* player ) const;
		void Setup( C_CSPlayer* player );
	};

	void AnimationBackup::Apply( C_CSPlayer* player ) const {
		*player->m_PlayerAnimState( ) = this->anim_state;
		std::memcpy( player->m_AnimOverlay( ).m_Memory.m_pMemory, layers, sizeof( layers ) );
		std::memcpy( player->m_flPoseParameter( ), pose_params, sizeof( pose_params ) );
	}

	void AnimationBackup::Setup( C_CSPlayer* player ) {
		this->anim_state = *player->m_PlayerAnimState( );
		std::memcpy( layers, player->m_AnimOverlay( ).m_Memory.m_pMemory, sizeof( layers ) );
		std::memcpy( pose_params, player->m_flPoseParameter( ), sizeof( pose_params ) );
	}

	inline void FixBonesRotations( C_CSPlayer* player, matrix3x4_t* bones ) {
		// copypasted from supremacy/fatality, no difference imo
		// also seen that in aimware multipoints, but was lazy to paste, kek
		auto studio_hdr = player->m_pStudioHdr( );
		if( studio_hdr ) {
			auto hdr = *( studiohdr_t** )studio_hdr;
			if( hdr ) {
				auto hitboxSet = hdr->pHitboxSet( player->m_nHitboxSet( ) );
				for( int i = 0; i < hitboxSet->numhitboxes; i++ ) {
					auto hitbox = hitboxSet->pHitbox( i );
					if( hitbox->m_angAngles.IsZero( ) )
						continue;

					matrix3x4_t hitboxTransform;
					hitboxTransform.AngleMatrix( hitbox->m_angAngles );
					bones[ hitbox->bone ] = bones[ hitbox->bone ].ConcatTransforms( hitboxTransform );
				}
			}
		}
	}

	class C_AnimationSystem : public AnimationSystem {
	public:
		virtual void CollectData( );
		virtual void Update( );

		virtual C_AnimationData* GetAnimationData( int index ) {
			if( m_AnimatedEntities.count( index ) < 1 )
				return nullptr;

			return &m_AnimatedEntities[ index ];
		}

		std::map<int, C_AnimationData> m_AnimatedEntities = { };

		C_AnimationSystem( ) { };
		virtual ~C_AnimationSystem( ) { };
	};

	Encrypted_t<AnimationSystem> AnimationSystem::Get( ) {
		static C_AnimationSystem instance;
		return &instance;
	}

	void C_AnimationSystem::CollectData( ) {
		if( !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if( !local || !g_Vars.globals.HackIsReady )
			return;

		for( int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; ++i ) {
			auto player = C_CSPlayer::GetPlayerByIndex( i );
			if( !player || player == local )
				continue;

			player_info_t player_info;
			if( !Interfaces::m_pEngine->GetPlayerInfo( player->m_entIndex, &player_info ) ) {
				continue;
			}

			this->m_AnimatedEntities[ i ].Collect( player );
		}
	}

	void C_AnimationSystem::Update( ) {
		if( !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->GetNetChannelInfo( ) ) {
			this->m_AnimatedEntities.clear( );
			return;
		}

		auto local = C_CSPlayer::GetLocalPlayer( );
		if( !local || !g_Vars.globals.HackIsReady )
			return;

		for( auto& [key, value] : this->m_AnimatedEntities ) {
			auto entity = C_CSPlayer::GetPlayerByIndex( key );
			if( !entity )
				continue;

			auto curtime = Interfaces::m_pGlobalVars->curtime;
			auto frametime = Interfaces::m_pGlobalVars->frametime;

			Interfaces::m_pGlobalVars->curtime = entity->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick;
			Interfaces::m_pGlobalVars->frametime = Interfaces::m_pGlobalVars->interval_per_tick;

			if( value.m_bUpdated )
				value.Update( );

			Interfaces::m_pGlobalVars->curtime = curtime;
			Interfaces::m_pGlobalVars->frametime = frametime;

			value.m_bUpdated = false;
		}

	}

	void C_AnimationData::Update( ) {
		if( !this->player || this->m_AnimationRecord.size( ) < 1 )
			return;

		C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
		if( !pLocal )
			return;

		auto pAnimationRecord = Encrypted_t<Engine::C_AnimationRecord>( &this->m_AnimationRecord.front( ) );
		Encrypted_t<Engine::C_AnimationRecord> pPreviousAnimationRecord( nullptr );
		if( this->m_AnimationRecord.size( ) > 1 ) {
			pPreviousAnimationRecord = &this->m_AnimationRecord.at( 1 );
		}

		this->player->m_vecVelocity( ) = pAnimationRecord->m_vecAnimationVelocity;

		auto weapon = ( C_BaseAttributableItem* )player->m_hActiveWeapon( ).Get( );
		auto weaponWorldModel = weapon ? ( C_CSPlayer* )( weapon )->m_hWeaponWorldModel( ).Get( ) : nullptr;

		auto animState = player->m_PlayerAnimState( );
		if( !animState )
			return;

		if (this->player->IsDormant()) {
			bool insert = true;

			if (!this->m_AnimationRecord.empty()) {
				C_AnimationRecord front = this->m_AnimationRecord.front();

				if (front.dormant())
					insert = false;
			}

			if (insert) {
				C_AnimationRecord current = this->m_AnimationRecord.front();

				current.m_dormant = true;
			}
		}

		// simulate animations
		SimulateAnimations( pAnimationRecord, pPreviousAnimationRecord );

		// update layers
		std::memcpy( player->m_AnimOverlay( ).Base( ), pAnimationRecord->m_serverAnimOverlays, 13 * sizeof( C_AnimationLayer ) );

		// generate aimbot matrix
		g_BoneSetup.SetupBonesRebuild( player, m_Bones, 128, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, player->m_flSimulationTime( ), BoneSetupFlags::UseCustomOutput );

		FixBonesRotations(player, m_Bones);

		// generate visual matrix
		g_BoneSetup.SetupBonesRebuild( player, nullptr, 128, 0x7FF00, player->m_flSimulationTime( ), BoneSetupFlags::ForceInvalidateBoneCache | BoneSetupFlags::AttachmentHelper );

		this->m_vecSimulationData.clear( );
	}

	void C_AnimationData::Collect( C_CSPlayer* player ) {
		auto local = C_CSPlayer::GetLocalPlayer();

		if( player->IsDead( ) )
			player = nullptr;

		if (!local)
			return;

		auto pThis = Encrypted_t<C_AnimationData>( this );

		if( pThis->player != player ) {
			pThis->m_flSpawnTime = 0.0f;
			pThis->m_flSimulationTime = 0.0f;
			pThis->m_flOldSimulationTime = 0.0f;
			pThis->m_iCurrentTickCount = 0;
			pThis->m_iOldTickCount = 0;
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_vecSimulationData.clear( );
			pThis->m_AnimationRecord.clear( );
			pThis->m_bIsDormant = pThis->m_bBonesCalculated = false;
			pThis->player = player;
			pThis->m_bIsAlive = false;
			pThis->m_old_sim = 0.0f;
			pThis->m_cur_sim = 0.0f;
		}

		if( !player )
			return;

		pThis->m_bIsAlive = true;
		pThis->m_flOldSimulationTime = pThis->m_flSimulationTime;
		pThis->m_flSimulationTime = pThis->player->m_flSimulationTime( );

		if (pThis->m_flSimulationTime == 0.0f || pThis->player->IsDormant()) {
			pThis->m_bIsDormant = true;
			Engine::g_ResolverData[player->EntIndex()].m_bWentDormant = true;
			Engine::g_ResolverData[player->EntIndex()].m_vecSavedOrigin = pThis->m_vecOrigin;
			return;
		}

		int player_updated = false;
		int invalid_simulation = false;

		if (player->m_flSimulationTime() != 0.0f)
		{
			if (player->GetAnimLayer(11).m_flCycle != pThis->m_sim_cycle
				|| player->GetAnimLayer(11).m_flPlaybackRate != pThis->m_sim_rate)
				player_updated = 1;
			else
			{
				player->m_flOldSimulationTime() = pThis->m_old_sim;
				invalid_simulation = 1;
				player->m_flSimulationTime() = pThis->m_cur_sim;
			}
		}
		else
			return;

		bool silent_update = false;

		auto update = 0;
		if (!invalid_simulation)
		{

			auto v23 = pThis->m_cur_sim;
			pThis->m_old_sim = v23;
			auto v24 = player->m_flSimulationTime();
			pThis->m_cur_sim = v24;
			if (player_updated || v24 != v23 && (pThis->m_cur_sim == 0))
				update = 1;

			if (player_updated && v24 == v23)
				silent_update = true;
		}

		bool should_update = update || silent_update;

		if (!update)
			return;

		//if (pThis->m_flOldSimulationTime == pThis->m_flSimulationTime) {
		//	return;
		//}

		pThis->m_sim_cycle = player->GetAnimLayer(11).m_flCycle;
		pThis->m_sim_rate = player->GetAnimLayer(11).m_flPlaybackRate;

		if( pThis->m_bIsDormant ) {
			pThis->m_iTicksAfterDormancy = 0;
			pThis->m_AnimationRecord.clear( );

			Engine::g_ResolverData[ player->EntIndex( ) ].m_bWentDormant = true;
		}

		pThis->ent_index = player->m_entIndex;

		pThis->m_bUpdated = true;
		pThis->m_bIsDormant = false;

		pThis->m_iOldTickCount = pThis->m_iCurrentTickCount;
		pThis->m_iCurrentTickCount = Interfaces::m_pGlobalVars->tickcount;

		if( pThis->m_flSpawnTime != pThis->player->m_flSpawnTime( ) ) {
			auto animState = pThis->player->m_PlayerAnimState( );
			if( animState ) {
				animState->m_Player = pThis->player;
				animState->Reset( );
			}

			pThis->m_flSpawnTime = pThis->player->m_flSpawnTime( );
		}

		int nTickRate = int( 1.0f / Interfaces::m_pGlobalVars->interval_per_tick );
		while( pThis->m_AnimationRecord.size( ) > nTickRate ) {
			pThis->m_AnimationRecord.pop_back( );
		}

		pThis->m_iTicksAfterDormancy++;

		Encrypted_t<C_AnimationRecord> previous_record = nullptr;
		Encrypted_t<C_AnimationRecord> penultimate_record = nullptr;

		if( pThis->m_AnimationRecord.size( ) > 0 ) {
			previous_record = &pThis->m_AnimationRecord.front( );
			if( pThis->m_AnimationRecord.size( ) > 1 ) {
				penultimate_record = &pThis->m_AnimationRecord.at( 1 );
			}
		}

		auto record = &pThis->m_AnimationRecord.emplace_front( );

		pThis->m_vecOrigin = pThis->player->m_vecOrigin( );

		record->m_vecOrigin = pThis->player->m_vecOrigin( );
		record->m_angEyeAngles = pThis->player->m_angEyeAngles( );
		record->m_flSimulationTime = pThis->m_flSimulationTime;
		record->m_anim_time = pThis->m_flOldSimulationTime + Interfaces::m_pGlobalVars->interval_per_tick;
		record->m_flLowerBodyYawTarget = pThis->player->m_flLowerBodyYawTarget( );
		//record->m_body = pThis->player->m_flLowerBodyYawTarget();

		auto weapon = ( C_WeaponCSBaseGun* )( player->m_hActiveWeapon( ).Get( ) );

		if( weapon ) {
			auto weaponWorldModel = ( C_CSPlayer* )( ( C_BaseAttributableItem* )weapon )->m_hWeaponWorldModel( ).Get( );

			for( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Element( i ).m_pOwner = player;
				player->m_AnimOverlay( ).Element( i ).m_pStudioHdr = player->m_pStudioHdr( );

				if( weaponWorldModel ) {
					if( player->m_AnimOverlay( ).Element( i ).m_nSequence < 2 || player->m_AnimOverlay( ).Element( i ).m_flWeight <= 0.0f )
						continue;

					using UpdateDispatchLayer = void( __thiscall* )( void*, C_AnimationLayer*, CStudioHdr*, int );
					Memory::VCall< UpdateDispatchLayer >( player, 241 )( player, &player->m_AnimOverlay( ).Element( i ),
						weaponWorldModel->m_pStudioHdr( ), player->m_AnimOverlay( ).Element( i ).m_nSequence );
				}
			}
		}

		std::memcpy( record->m_serverAnimOverlays, pThis->player->m_AnimOverlay( ).Base( ), sizeof( record->m_serverAnimOverlays ) );

		record->m_flFeetCycle = record->m_serverAnimOverlays[ 6 ].m_flCycle;
		record->m_flFeetYawRate = record->m_serverAnimOverlays[ 6 ].m_flWeight;

		record->m_fFlags = player->m_fFlags( );
		record->m_flDuckAmount = player->m_flDuckAmount( );

		record->m_bIsShoting = false;
		record->m_flShotTime = 0.0f;
		record->m_bFakeWalking = false;
		Engine::g_ResolverData[player->EntIndex()].fakewalking = false;

		// if record is marked as exploit. do not store simulation time and let aimbot ignore it.
		if (silent_update) {
			record->m_bTeleportDistance = true;
			//record->m_bIsInvalid = true;
		}

		int ticks_to_simulate = 1;

		if (previous_record.IsValid() && !previous_record->dormant())
		{
			int simulation_ticks = TIME_TO_TICKS(record->m_flSimulationTime - previous_record->m_flSimulationTime);

			if ((simulation_ticks - 1) > 31 || previous_record->m_flSimulationTime == 0.f)
				simulation_ticks = 1;

			auto layer_cycle = record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flCycle;
			auto previous_playback = previous_record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

			if (previous_playback > 0.f && record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate > 0.f
				&& previous_record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_nSequence == record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_nSequence
				/*&& m_player->get_animation_state()->m_weapon == m_player->get_animation_state()->m_weapon_last*/)
			{
				auto previous_cycle = previous_record->m_serverAnimOverlays[11].m_flCycle;
				simulation_ticks = 0;

				if (previous_cycle > layer_cycle)
					layer_cycle = layer_cycle + 1.0f;

				while (layer_cycle > previous_cycle)
				{
					const auto ticks_backup = simulation_ticks;
					const auto playback_mult_ipt = Interfaces::m_pGlobalVars->interval_per_tick * previous_playback;

					previous_cycle = previous_cycle + (Interfaces::m_pGlobalVars->interval_per_tick * previous_playback);

					if (previous_cycle >= 1.0f)
						previous_playback = record->m_serverAnimOverlays[ANIMATION_LAYER_ALIVELOOP].m_flPlaybackRate;

					++simulation_ticks;

					if (previous_cycle > layer_cycle && (previous_cycle - layer_cycle) > (playback_mult_ipt * 0.5f))
						simulation_ticks = ticks_backup;
				}
			}

			ticks_to_simulate = simulation_ticks;

			//if (record->exploit)
			//	record->simulation_time = previous->simulation_time + TICKS_TO_TIME(simulation_ticks);
		}

		ticks_to_simulate = Math::Clamp(ticks_to_simulate, 1, 64);

		record->m_flChokeTime = float(ticks_to_simulate) * Interfaces::m_pGlobalVars->interval_per_tick;
		record->m_iChokeTicks = ticks_to_simulate;

		record->max_current_speed = player->GetMaxSpeed();

		record->max_current_speed = fmaxf(record->max_current_speed, 0.001f);

		record->animation_speed = -1.f;

		//if( previous_record.IsValid( ) ) {
		//	record->m_flChokeTime = pThis->m_flSimulationTime - pThis->m_flOldSimulationTime;
		//	record->m_iChokeTicks = TIME_TO_TICKS( record->m_flChokeTime );
		//}
		//else {
		//	record->m_flChokeTime = Interfaces::m_pGlobalVars->interval_per_tick;
		//	record->m_iChokeTicks = 1;
		//}

		if( !previous_record.IsValid( ) ) {
			record->m_bIsInvalid = true;
			record->m_vecVelocity.Init( );
			record->m_bIsShoting = false;
			record->m_bTeleportDistance = false;

			//auto animstate = player->m_PlayerAnimState( );
			//if( animstate )
			//	animstate->m_flAbsRotation = record->m_angEyeAngles.yaw;

			return;
		}

		auto flPreviousSimulationTime = previous_record->m_flSimulationTime;
		auto nTickcountDelta = pThis->m_iCurrentTickCount - pThis->m_iOldTickCount;
		auto nSimTicksDelta = record->m_iChokeTicks;
		auto nChokedTicksUnk = nSimTicksDelta;
		auto bShiftedTickbase = false;
		if( pThis->m_flOldSimulationTime > pThis->m_flSimulationTime ) {
			record->m_bShiftingTickbase = true;
			record->m_iChokeTicks = nTickcountDelta;
			record->m_flChokeTime = TICKS_TO_TIME( record->m_iChokeTicks );
			flPreviousSimulationTime = record->m_flSimulationTime - record->m_flChokeTime;
			nChokedTicksUnk = nTickcountDelta;
			bShiftedTickbase = true;
		}

		if( bShiftedTickbase || abs( nSimTicksDelta - nTickcountDelta ) <= 2 ) {
			if( nChokedTicksUnk ) {
				if( nChokedTicksUnk != 1 ) {
					pThis->m_iTicksUnknown = 0;
				}
				else {
					pThis->m_iTicksUnknown++;
				}
			}
			else {
				record->m_iChokeTicks = 1;
				record->m_flChokeTime = Interfaces::m_pGlobalVars->interval_per_tick;

				flPreviousSimulationTime = record->m_flSimulationTime - Interfaces::m_pGlobalVars->interval_per_tick;

				pThis->m_iTicksUnknown++;
			}
		}

		if( weapon ) {
			record->m_flShotTime = weapon->m_fLastShotTime( );
			record->m_bIsShoting = record->m_flSimulationTime >= record->m_flShotTime && record->m_flShotTime > previous_record->m_flSimulationTime;
		}

		record->m_bIsInvalid = false;

		// velocity fixa
		//if (record->m_iChokeTicks > 1 && !silent_update) {
		//	recalculate_velocity(pThis, record, player, previous_record);
		//}
		//else
		//	record->m_vecVelocity = player->m_vecVelocity();

		// thanks llama.
		auto animState = player->m_PlayerAnimState();

		if (record->m_fFlags & FL_ONGROUND) {
			if (animState) {
				// they are on ground.
				animState->m_bOnGround = true;
				// no they didnt land.
				animState->m_bHitground = false;
			}
		}

		if (!previous_record->dormant() && !silent_update) {
			auto was_in_air = (player->m_fFlags() & FL_ONGROUND) && (previous_record->m_fFlags & FL_ONGROUND);
			auto animation_speed = 0.0f;
			auto origin_delta = record->m_vecOrigin - previous_record->m_vecOrigin;

			Vector vecPreviousOrigin = previous_record->m_vecOrigin;
			int fPreviousFlags = previous_record->m_fFlags;
			auto time_difference = std::max(Interfaces::m_pGlobalVars->interval_per_tick, record->m_flSimulationTime - previous_record->m_flSimulationTime);

			// fix velocity
			// https://github.com/VSES/SourceEngine2007/blob/master/se2007/game/client/c_baseplayer.cpp#L659
			if (!origin_delta.IsZero() && TIME_TO_TICKS(time_difference) > 0) {
				//record->m_vecVelocity = (record->m_vecOrigin - previous_record->m_vecOrigin) * (1.f / record->m_flChokeTime);
				record->m_vecVelocity = origin_delta * (1.0f / time_difference);

				if (player->m_fFlags() & FL_ONGROUND && record->m_serverAnimOverlays[11].m_flWeight > 0.0f && record->m_serverAnimOverlays[11].m_flWeight < 1.0f && record->m_serverAnimOverlays[11].m_flCycle > previous_record->m_serverAnimOverlays[11].m_flCycle)
				{
					auto weapon = player->m_hActiveWeapon().Get();

					if (weapon)
					{
						auto max_speed = 260.0f;
						C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)player->m_hActiveWeapon().Get();
						auto weapon_info = Weapon->GetCSWeaponData();

						if (weapon_info.IsValid())
							max_speed = player->m_bIsScoped() ? weapon_info->m_flMaxSpeed2 : weapon_info->m_flMaxSpeed;

						auto modifier = 0.35f * (1.0f - record->m_serverAnimOverlays[11].m_flWeight);

						if (modifier > 0.0f && modifier < 1.0f)
							animation_speed = max_speed * (modifier + 0.55f);
					}
				}

				if (animation_speed > 0.0f)
				{
					animation_speed /= record->m_vecVelocity.Length2D();

					record->m_vecVelocity.x *= animation_speed;
					record->m_vecVelocity.y *= animation_speed;
				}

				if (pThis->m_AnimationRecord.size() >= 3 && time_difference > Interfaces::m_pGlobalVars->interval_per_tick)
				{
					auto previous_velocity = (previous_record->m_vecOrigin - pThis->m_AnimationRecord.at(2).m_vecOrigin) * (1.0f / time_difference);

					if (!previous_velocity.IsZero() && !was_in_air)
					{
						auto current_direction = Math::normalize_float(RAD2DEG(atan2(record->m_vecVelocity.y, record->m_vecVelocity.x)));
						auto previous_direction = Math::normalize_float(RAD2DEG(atan2(previous_velocity.y, previous_velocity.x)));

						auto average_direction = current_direction - previous_direction;
						average_direction = DEG2RAD(Math::normalize_float(current_direction + average_direction * 0.5f));

						auto direction_cos = cos(average_direction);
						auto dirrection_sin = sin(average_direction);

						auto velocity_speed = record->m_vecVelocity.Length2D();

						record->m_vecVelocity.x = direction_cos * velocity_speed;
						record->m_vecVelocity.y = dirrection_sin * velocity_speed;
					}
				}

				// fix CGameMovement::FinishGravity
				if (!(player->m_fFlags() & FL_ONGROUND)) {
					static auto sv_gravity = g_Vars.sv_gravity;
					auto fixed_timing = Math::Clamp(time_difference, Interfaces::m_pGlobalVars->interval_per_tick, 1.0f);
					record->m_vecVelocity.z -= sv_gravity->GetFloat() * fixed_timing * 0.5f;
				}
				else
					record->m_vecVelocity.z = 0.0f;

				if (((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(local->EntIndex())) < 199) {
					if (record->m_vecVelocity.Length2D() >= 100.f)
					{
						player_extrapolation(player, vecPreviousOrigin, record->m_vecVelocity, player->m_fFlags(), fPreviousFlags & FL_ONGROUND, 4);
					}
					if (record->m_vecVelocity.z > 0)
					{
						player_extrapolation(player, vecPreviousOrigin, record->m_vecVelocity, player->m_fFlags(), !(fPreviousFlags & FL_ONGROUND), 4);
					}
				}
			}
		}

		//printf("velocity: ");
		//printf(std::to_string(record->m_vecVelocity.Length2D()).c_str());
		//printf(" \n");

		//// detect fakewalking players
		if (record->m_vecVelocity.Length() > 0.1f
			//&& record->m_iChokeTicks >= 2
			&& record->m_serverAnimOverlays[12].m_flWeight == 0.0f
			&& record->m_serverAnimOverlays[6].m_flWeight == 0.0f
			&& record->m_serverAnimOverlays[6].m_flPlaybackRate < 0.0003f // For a semi-accurate result you could use this being weight being about smaller than 0.0001 (maybe would need to scale it up against faster fakewalks, won't have too big of a impact though.)
			&& (record->m_fFlags & FL_ONGROUND) && record->m_vecVelocity.Length() <= 85.f) {
			record->m_bFakeWalking = true;
			Engine::g_ResolverData[player->EntIndex()].fakewalking = true;
		}
		else {
			Engine::g_ResolverData[player->EntIndex()].fakewalking = false;
			record->m_bFakeWalking = false;
		}

		// detect players abusing micromovements or other trickery
		if (record->m_vecVelocity.Length() < 18.f
			&& record->m_serverAnimOverlays[6].m_flWeight != 1.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != 0.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != previous_record->m_serverAnimOverlays[6].m_flWeight
			&& (record->m_fFlags & FL_ONGROUND)) {
			record->m_bUnsafeVelocityTransition = true;
		}

		record->m_vecAnimationVelocity = record->m_vecVelocity;

		if( record->m_bFakeWalking ) {
			record->m_vecAnimationVelocity.Init( );
		}

		record->m_vecLastNonDormantOrig = record->m_vecOrigin;

		record->m_bTeleportDistance = (record->m_vecOrigin - previous_record->m_vecOrigin).Length2D() > 4096.f;

		//LinearExtrapolations(record);

		C_SimulationInfo& data = pThis->m_vecSimulationData.emplace_back( );
		data.m_flTime = previous_record->m_flSimulationTime + Interfaces::m_pGlobalVars->interval_per_tick;
		data.m_flDuckAmount = record->m_flDuckAmount;
		data.m_flLowerBodyYawTarget = record->m_flLowerBodyYawTarget;
		data.m_vecOrigin = record->m_vecOrigin;
		data.m_vecVelocity = record->m_vecAnimationVelocity;
		data.bOnGround = record->m_fFlags & FL_ONGROUND;

		//lets check if its been more than 1 tick, so we can fix jumpfall.
		if( record->m_iChokeTicks > 1 && !previous_record->dormant() && !silent_update ) {
			// TODO: calculate jump time
			// calculate landing time
			float flLandTime = 0.0f;
			bool bJumped = false;
			bool bLandedOnServer = false;
			if( record->m_serverAnimOverlays[ 4 ].m_flCycle < 0.5f && ( !( record->m_fFlags & FL_ONGROUND ) || !( previous_record->m_fFlags & FL_ONGROUND ) ) ) {
				// note - VIO;
				// well i guess when llama wrote v3, he was drunk or sum cuz this is incorrect. -> cuz he changed this in v4.
				// and alpha didn't realize this but i did, so its fine.
				// improper way to do this -> flLandTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4 ].m_flPlaybackRate * record->m_serverAnimOverlays[ 4 ].m_flCycle );
				// we need to divide instead of multiplication.
				flLandTime = record->m_flSimulationTime - float( record->m_serverAnimOverlays[ 4 ].m_flCycle / record->m_serverAnimOverlays[ 4 ].m_flPlaybackRate );
				bLandedOnServer = flLandTime >= record->m_flSimulationTime;
			}

			bool bOnGround = record->m_fFlags & FL_ONGROUND;
			// jump_fall fix
			if( bLandedOnServer && !bJumped ) {
				if( flLandTime < data.m_flTime ) {
					player->m_fFlags() &= ~FL_ONGROUND;
					bJumped = true;
					bOnGround = true;
					auto layer = &player->GetAnimLayer(4);
					layer->m_flCycle = 0;
					layer->m_flWeight = 0;
				}
			}

			if (record->m_fFlags & FL_ONGROUND) {
				player->m_fFlags() |= FL_ONGROUND;
			}

			data.bOnGround = bOnGround;

			// delta in time..
			//float time = record->m_flSimulationTime - previous_record->m_flSimulationTime;

			//if (!record->m_bFakeWalking) {
			//	// fix the velocity till the moment of animation.
			//	Vector velo = record->m_vecVelocity - previous_record->m_vecVelocity;

			//	// accel per tick.
			//	Vector accel = (velo / time) * Interfaces::m_pGlobalVars->interval_per_tick;

			//	// set the anim velocity to the previous velocity.
			//	// and predict one tick ahead.
			//	record->m_vecAnimationVelocity = previous_record->m_vecVelocity + accel;
			//}

			float duck_amount_per_tick = 0.0f;

			if (record->m_flDuckAmount != previous_record->m_flDuckAmount
				&& record->m_fFlags & FL_ONGROUND
				&& previous_record->m_fFlags & FL_ONGROUND)
			{
				//if (record->time_delta != 0.0f)
				//	record->duck_amount_per_tick = csgo.m_globals()->interval_per_tick / record->time_delta;
				auto v210 = record->m_flChokeTime - TICKS_TO_TIME(1);
				if (v210 != 0.f)
					duck_amount_per_tick = Interfaces::m_pGlobalVars->interval_per_tick / v210;
			}

			if (duck_amount_per_tick != 0.0f)
			{
				auto v208 = ((record->m_flDuckAmount - player->m_flDuckAmount()) * duck_amount_per_tick)
					+ player->m_flDuckAmount();

				player->m_flDuckAmount() = fminf(fmaxf(v208, 0.0f), 1.0f);
			}
		}
	}

	void C_AnimationData::SimulateAnimations( Encrypted_t<Engine::C_AnimationRecord> current, Encrypted_t<Engine::C_AnimationRecord> previous ) {
		auto UpdateAnimations = [ & ] ( C_CSPlayer* player, float flTime ) {
			auto curtime = Interfaces::m_pGlobalVars->curtime;
			auto frametime = Interfaces::m_pGlobalVars->frametime;
			auto realtime = Interfaces::m_pGlobalVars->realtime;
			auto absframetime = Interfaces::m_pGlobalVars->absoluteframetime;
			auto framecount = Interfaces::m_pGlobalVars->framecount;
			auto tickcount = Interfaces::m_pGlobalVars->tickcount;
			auto interpolation = Interfaces::m_pGlobalVars->interpolation_amount;

			// force to use correct abs origin and velocity ( no CalcAbsolutePosition and CalcAbsoluteVelocity calls )
			player->m_iEFlags( ) &= ~( EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY );

			int ticks = TIME_TO_TICKS( flTime );

			// calculate animations based on ticks aka server frames instead of render frames
			Interfaces::m_pGlobalVars->curtime = flTime;
			Interfaces::m_pGlobalVars->realtime = flTime;
			Interfaces::m_pGlobalVars->frametime = Interfaces::m_pGlobalVars->interval_per_tick;
			Interfaces::m_pGlobalVars->absoluteframetime = Interfaces::m_pGlobalVars->interval_per_tick;
			Interfaces::m_pGlobalVars->framecount = TIME_TO_TICKS(current->m_anim_time);
			Interfaces::m_pGlobalVars->tickcount = TIME_TO_TICKS(current->m_anim_time);
			Interfaces::m_pGlobalVars->interpolation_amount = 0.0f;

			auto animstate = player->m_PlayerAnimState( );
			if( animstate && animstate->m_nLastFrame >= Interfaces::m_pGlobalVars->framecount )
				animstate->m_nLastFrame = Interfaces::m_pGlobalVars->framecount - 1;

			for( int i = 0; i < player->m_AnimOverlay( ).Count( ); ++i ) {
				player->m_AnimOverlay( ).Base( )[ i ].m_pOwner = player;
				player->m_AnimOverlay( ).Base( )[ i ].m_pStudioHdr = player->m_pStudioHdr( );
			}

			static auto& EnableInvalidateBoneCache = **reinterpret_cast< bool** >( Memory::Scan( XorStr( "client.dll" ), XorStr( "C6 05 ? ? ? ? ? 89 47 70" ) ) + 2 );

			// make sure we keep track of the original invalidation state
			const auto oldInvalidationState = EnableInvalidateBoneCache;

			// is player bot?
			auto IsPlayerBot = [&]( ) -> bool {
				player_info_t info;
				if( Interfaces::m_pEngine->GetPlayerInfo( player->EntIndex( ), &info ) )
					return info.fakeplayer;

				return false;
			};

			if (!player->IsTeammate(C_CSPlayer::GetLocalPlayer())) { // plist overrides
				if (g_Vars.globals.player_list.override_pitch[player->EntIndex()]) {
					player->m_angEyeAngles().x = g_Vars.globals.player_list.override_pitch_slider[player->EntIndex()];
				}
			}

			// attempt to resolve the player	
			if( !player->IsTeammate( C_CSPlayer::GetLocalPlayer( ) ) && !IsPlayerBot( ) ) {
				// predict lby updates
				g_Resolver.ResolveAngles( player, current.Xor( ) );

				bool bValid = previous.Xor( );

				// we're sure that we resolved the player.
				if ( bValid )
				{
					current.Xor()->m_bResolved = current.Xor()->m_iResolverMode == Engine::RModes::MOVING || current.Xor()->m_iResolverMode == Engine::RModes::FLICK;
				}
				else
				{
					current.Xor()->m_bResolved = current.Xor()->m_iResolverMode == Engine::RModes::MOVING || current.Xor()->m_iResolverMode == Engine::RModes::FLICK;
				}

				bool bResolved = current.Xor( )->m_bResolved;
				if( g_Vars.rage.override_resolver_flicks ) {
					if( current.Xor( )->m_iResolverMode == Engine::RModes::FLICK)
						bResolved = false;
				}

				// if the enemy is resolved, why bother overriding?
				//g_Resolver.override_resolver(player, current.Xor());
				//g_Resolver.ResolveManual( player, current.Xor( ), bResolved );
			}

			player->UpdateClientSideAnimationEx( );

			// we don't want to enable cache invalidation by accident
			EnableInvalidateBoneCache = oldInvalidationState;

			Interfaces::m_pGlobalVars->curtime = curtime;
			Interfaces::m_pGlobalVars->frametime = frametime;
			Interfaces::m_pGlobalVars->realtime = realtime;
			Interfaces::m_pGlobalVars->absoluteframetime = absframetime;
			Interfaces::m_pGlobalVars->framecount = framecount;
			Interfaces::m_pGlobalVars->tickcount = tickcount;
			Interfaces::m_pGlobalVars->interpolation_amount = interpolation;
		};

		SimulationRestore SimulationRecordBackup;
		SimulationRecordBackup.Setup( player );

		auto animState = player->m_PlayerAnimState( );

		if( previous.IsValid( ) ) {
			animState->m_flFeetCycle = previous->m_flFeetCycle;
			animState->m_flFeetYawRate = previous->m_flFeetYawRate;
			*( float* )( uintptr_t( animState ) + 0x180 ) = previous->m_serverAnimOverlays[ 12 ].m_flWeight;

			std::memcpy( player->m_AnimOverlay( ).Base( ), previous->m_serverAnimOverlays, sizeof( previous->m_serverAnimOverlays ) );
		}
		else {
			animState->m_flFeetCycle = current->m_flFeetCycle;
			animState->m_flFeetYawRate = current->m_flFeetYawRate;
			*( float* )( uintptr_t( animState ) + 0x180 ) = current->m_serverAnimOverlays[ 12 ].m_flWeight;
		}

		if( current->m_iChokeTicks > 1 ) {
			for( auto it = this->m_vecSimulationData.begin( ); it < this->m_vecSimulationData.end( ); it++ ) {
				m_bForceVelocity = true;
				const auto& simData = *it;

				player->m_vecOrigin( ) = simData.m_vecOrigin;
				player->m_flDuckAmount( ) = simData.m_flDuckAmount;
				player->m_vecVelocity( ) = simData.m_vecVelocity;
				player->SetAbsVelocity( simData.m_vecVelocity );
				player->SetAbsOrigin( simData.m_vecOrigin );
				player->m_flLowerBodyYawTarget( ) = simData.m_flLowerBodyYawTarget;

				UpdateAnimations( player, player->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick );

				m_bForceVelocity = false;
			}
		}
		else {
			m_bForceVelocity = true;
			this->player->SetAbsVelocity( current->m_vecAnimationVelocity );
			this->player->SetAbsOrigin( current->m_vecOrigin );
			this->player->m_flLowerBodyYawTarget( ) = current->m_flLowerBodyYawTarget;

			UpdateAnimations( player, player->m_flOldSimulationTime( ) + Interfaces::m_pGlobalVars->interval_per_tick );

			m_bForceVelocity = false;
		}

		SimulationRecordBackup.Apply( player );
		player->InvalidatePhysicsRecursive( 8 );
	}
}