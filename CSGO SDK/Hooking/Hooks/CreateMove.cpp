#include "../Hooked.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../../Features/Game/Prediction.hpp"
#include "../../Features/Miscellaneous/Movement.hpp"
#include <intrin.h>
#include "../../Features/Rage/Ragebot.hpp"
#include "../../Features/Miscellaneous/Miscellaneous.hpp"
#include "../../Utils/InputSys.hpp"
#include "../../SDK/Classes/Exploits.hpp"
#include "../../Features/Rage/FakeLag.hpp"
#include "../../Features/Rage/LagCompensation.hpp"
#include "../../Utils/Threading/threading.h"
#include "../../SDK/Classes/CCSGO_HudDeathNotice.hpp"
#include "../../Features/Rage/ShotInformation.hpp"
#include <thread>
#include "../../Features/Visuals/GrenadePrediction.hpp"
#include "../../Features/Rage/TickbaseShift.hpp"
#include "../../Features/Game/SetupBones.hpp"
#include "../../Features/Visuals/Hitmarker.hpp"
#include "../../Features/Rage/AntiAim.hpp"
#include "../../Features/Rage/Resolver.hpp"
#include "../../ShittierMenu/MenuNew.h"
#include "../../Features/Miscellaneous/walkbot.h"

extern float fl_Override;
extern bool g_Override;

Vector AutoPeekPos;

int LastShotTime = 0;
int OutgoingTickcount = 0;

void sunsetmode()
{
	auto local = C_CSPlayer::GetLocalPlayer();

	if (!g_Vars.globals.HackIsReady || !local || !Interfaces::m_pEngine->IsInGame() || !Interfaces::m_pEngine->IsConnected())
		return;

	static ConVar* cl_csm_shadows = g_Vars.cl_csm_shadows;

	if (!g_Vars.esp.sunset_enable)
	{
		if (cl_csm_shadows->GetInt() != 0)
			cl_csm_shadows->SetValue(0);

		return;
	}

	cl_csm_shadows->SetValue(1);

	static ConVar* cl_csm_max_shadow_dist = Interfaces::m_pCvar->FindVar(XorStr("cl_csm_max_shadow_dist"));
	cl_csm_max_shadow_dist->SetValue(800);

	static ConVar* cl_csm_rot_override = Interfaces::m_pCvar->FindVar(XorStr("cl_csm_rot_override"));
	cl_csm_rot_override->SetValue(800);

	static ConVar* cl_csm_rot_x = Interfaces::m_pCvar->FindVar(XorStr("cl_csm_rot_x"));
	cl_csm_rot_x->SetValue(g_Vars.esp.sunset_rot_x);

	static ConVar* cl_csm_rot_y = Interfaces::m_pCvar->FindVar(XorStr("cl_csm_rot_y"));
	cl_csm_rot_y->SetValue(g_Vars.esp.sunset_rot_y);
}

void PreserveKillfeed() {
	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || !Interfaces::m_pEngine->IsInGame() || !Interfaces::m_pEngine->IsConnected()) {
		return;
	}

	static auto status = false;
	static float m_spawn_time = local->m_flSpawnTime();

	auto set = false;
	if (m_spawn_time != local->m_flSpawnTime() || status != g_Vars.esp.preserve_killfeed) {
		set = true;
		status = g_Vars.esp.preserve_killfeed;
		m_spawn_time = local->m_flSpawnTime();
	}

	for (int i = 0; i < Interfaces::g_pDeathNotices->m_vecDeathNotices.Count(); i++) {
		auto cur = &Interfaces::g_pDeathNotices->m_vecDeathNotices[i];
		if (!cur) {
			continue;
		}

		if (local->IsDead() || set) {
			if (cur->set != 1.f && !set) {
				continue;
			}

			cur->m_flStartTime = Interfaces::m_pGlobalVars->curtime;
			cur->m_flStartTime -= local->m_iHealth() <= 0 ? 2.f : 7.5f;
			cur->set = 2.f;

			continue;
		}

		if (cur->set == 2.f) {
			continue;
		}

		if (!status) {
			cur->set = 1.f;
			return;
		}

		if (cur->set == 1.f) {
			continue;
		}

		if (cur->m_flLifeTimeModifier == 1.5f) {
			cur->m_flStartTime = FLT_MAX;
		}

		cur->set = 1.f;
	}
}

void UserCmdDoubletap(CUserCmd* cmd) {
	static bool bCharged = false;

	if (!(g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit) || g_Vars.rage.double_tap_type != 1) {
		bCharged = false;
		return;
	}

	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || !local->IsAlive())
		return;

	auto weapon = (C_WeaponCSBaseGun*)(local->m_hActiveWeapon().Get());

	if (!weapon)
		return;

	if (g_TickbaseController.s_bBuilding && !bCharged && g_TickbaseController.s_nExtraProcessingTicks > 0) {
		//printf("pre-shift\n");

		if (!(cmd->buttons & IN_ATTACK) && weapon->m_fLastShotTime() + 0.5f < Interfaces::m_pGlobalVars->curtime) {

			if (!(cmd->buttons & IN_ATTACK)) {

				g_Vars.globals.shift_amount = 14;
				//printf("shift\n");
				cmd->tick_count == INT_MAX;
				bCharged = true;
			}
		}
	}

	if ((cmd->buttons & IN_ATTACK || g_TickbaseController.m_bSupressRecharge) && bCharged) {
		g_Vars.globals.shift_amount = 13;
		cmd->tick_count == INT_MAX;
		//printf("release\n");
		bCharged = false;
	}
}

namespace Hooked
{
	inline float anglemod(float a)
	{
		a = (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
		return a;
	}

	// BUGBUG: Why doesn't this call angle diff?!?!?
	float ApproachAngle(float target, float value, float speed)
	{
		target = anglemod(target);
		value = anglemod(value);

		float delta = target - value;

		// Speed is assumed to be positive
		if (speed < 0)
			speed = -speed;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		if (delta > speed)
			value += speed;
		else if (delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}


	// BUGBUG: Why do we need both of these?
	float AngleDiff(float destAngle, float srcAngle)
	{
		float delta;

		delta = fmodf(destAngle - srcAngle, 360.0f);
		if (destAngle > srcAngle)
		{
			if (delta >= 180)
				delta -= 360;
		}
		else
		{
			if (delta <= -180)
				delta += 360;
		}
		return delta;
	}

	class NetPos {
	public:
		float  m_time;
		Vector m_pos;

	public:
		__forceinline NetPos() : m_time{ }, m_pos{ } {};
		__forceinline NetPos(float time, Vector pos) : m_time{ time }, m_pos{ pos } {};
	};

	int m_real_spawntime;

	void UpdateInformation(CUserCmd* cmd, bool bSendPacket) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		CCSGOPlayerAnimState* state = local->m_PlayerAnimState();
		if (!state || local->IsDead() || local->IsDormant()) {
			m_real_spawntime = 0.f;
			return;
		}

		static auto lastTick = 0;
		if (Interfaces::m_pGlobalVars->tickcount % 16 == 0)
			lastTick = 0;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 1)
			lastTick = 1;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 2)
			lastTick = 2;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 3)
			lastTick = 3;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 4)
			lastTick = 4;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 5)
			lastTick = 5;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 6)
			lastTick = 6;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 7)
			lastTick = 7;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 8)
			lastTick = 8;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 9)
			lastTick = 9;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 10)
			lastTick = 10;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 11)
			lastTick = 11;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 12)
			lastTick = 12;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 13)
			lastTick = 13;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 14)
			lastTick = 14;
		else if (Interfaces::m_pGlobalVars->tickcount % 16 == 15)
			lastTick = 15;

		if (!(g_Vars.rage.dt_exploits && g_Vars.rage.exploit_standbychoke)) {
			if (Interfaces::m_pClientState->m_nChokedCommands() > 0)
				return;
		}
		else if (g_Vars.rage.dt_exploits && g_Vars.rage.exploit_standbychoke && (g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
			if (lastTick > 0)
				return;
		}
		else {
			if (Interfaces::m_pClientState->m_nChokedCommands() > 0)
				return;
		}

		// update time.
		g_Vars.globals.m_flAnimFrame = TICKS_TO_TIME(local->m_nTickBase()) - g_Vars.globals.m_flAnimTime;
		g_Vars.globals.m_flAnimTime = TICKS_TO_TIME(local->m_nTickBase());

		// current angle will be animated.
		g_Vars.globals.RegularAngles = cmd->viewangles;

		// fix landing anim.
		if (state->m_bHitground && g_Vars.globals.m_fFlags & FL_ONGROUND && local->m_fFlags() & FL_ONGROUND && g_Vars.esp.zeropitch)
			g_Vars.globals.RegularAngles.x = -12.f;

		Math::Clamp(g_Vars.globals.RegularAngles.x, -90.f, 90.f);
		g_Vars.globals.RegularAngles.Normalize();

		// write angles to model.
		Interfaces::m_pPrediction->SetLocalViewAngles(g_Vars.globals.RegularAngles);

		// set lby to predicted value.
		//local->m_flLowerBodyYawTarget( ) = g_Vars.globals.m_flBody;

		C_AnimationLayer old_anim_layers[13];
		std::memcpy(old_anim_layers, local->m_AnimOverlay().m_Memory.m_pMemory, 13 * sizeof(C_AnimationLayer));

		if (local->m_flSpawnTime() != m_real_spawntime || state->m_Player != local)
		{
			local->UpdateClientSideAnimationEx();

			std::memcpy(local->m_AnimOverlay().m_Memory.m_pMemory, old_anim_layers, 13 * sizeof(C_AnimationLayer));
			m_real_spawntime = local->m_flSpawnTime();
		}

		const auto old_pose_params = local->m_flPoseParameter();

		state->m_flLastUpdateIncrement = fmaxf(Interfaces::m_pGlobalVars->curtime - state->m_flLastUpdateTime, 0.f);

		if (state->m_ActiveWeapon != state->m_LastActiveWeapon)
		{
			for (int i = 0; i < 13; i++)
			{
				C_AnimationLayer pLayer = local->GetAnimLayer(i);

				pLayer.m_pStudioHdr = NULL;
				pLayer.m_nDispatchSequence = -1;
				pLayer.m_nDispatchSequence_2 = -1;
			}
		}

		float flNewDuckAmount;

		flNewDuckAmount = Math::Clamp(local->m_flDuckAmount() + state->m_fLandingDuckAdditiveSomething, 0.0f, 1.0f);
		flNewDuckAmount = Math::ApproachAngle(flNewDuckAmount, state->m_fDuckAmount, state->m_flLastUpdateIncrement * 6.0f);
		flNewDuckAmount = Math::Clamp(flNewDuckAmount, 0.0f, 1.0f);

		state->m_fDuckAmount = flNewDuckAmount;

		// CCSGOPlayerAnimState::Update, bypass already animated checks.
		if (state->m_nLastFrame == Interfaces::m_pGlobalVars->framecount)
			state->m_nLastFrame -= 1;

		local->m_iEFlags() &= ~(EFL_DIRTY_ABSTRANSFORM | EFL_DIRTY_ABSVELOCITY);

		state->m_flFeetYawRate = 0.f;

		local->UpdateClientSideAnimationEx();

		local->m_PlayerAnimState()->m_flFeetCycle = old_anim_layers[6].m_flCycle;
		local->m_PlayerAnimState()->m_flFeetYawRate = old_anim_layers[6].m_flWeight;

		for (int i = 0; i < 13; ++i)
		{
			C_AnimationLayer layer = local->GetAnimLayer(i);
			if (!layer.m_nSequence && layer.m_pOwner && layer.m_flWeight != 0.0f)
			{
				((C_CSPlayer*)layer.m_pOwner)->InvalidatePhysicsRecursive(10);
				layer.m_flWeight = 0.0f;
			}
		}

		auto flWeight12Backup = local->m_AnimOverlay().Element(12).m_flWeight;

		local->m_AnimOverlay().Element(12).m_flWeight = 0.f;

		if (local->m_flPoseParameter()) {
			local->m_flPoseParameter()[6] = g_Vars.globals.m_flJumpFall;
		}

		if (local->m_vecVelocity().Length() <= 1.0f) {
			if (state->m_bOnGround) {
				local->m_AnimOverlay().Element(3).m_flCycle = 0.0;
				local->m_AnimOverlay().Element(3).m_flWeight = 0.0;
			}
		}

		// pull the lower body direction towards the eye direction, but only when the player is moving
		if (state->m_bOnGround) {
			const float CSGO_ANIM_LOWER_CATCHUP_IDLE = 100.0f;
			const float CSGO_ANIM_LOWER_REALIGN_DELAY = 1.1f;

			if (state->m_velocity > 0.1f && !g_Vars.globals.Fakewalking) {
				g_Vars.globals.m_flBodyPred = g_Vars.globals.m_flAnimTime + (CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f);

				// we are moving n cant update.
				g_Vars.globals.m_bUpdate = false;
			}
			else if (state->m_velocity > 0.1f && g_Vars.globals.Fakewalking) {
				// we can no update our LBY.
				g_Vars.globals.m_bUpdate = true;

				if (g_Vars.globals.m_flAnimTime > g_Vars.globals.m_flBodyPred) {
					g_Vars.globals.m_flBodyPred = g_Vars.globals.m_flAnimTime + CSGO_ANIM_LOWER_REALIGN_DELAY;
					g_Vars.globals.m_flBody = g_Vars.globals.RegularAngles.y;
				}
			}
			else {
				// we can no update our LBY.
				g_Vars.globals.m_bUpdate = true;

				if (g_Vars.globals.m_flAnimTime > g_Vars.globals.m_flBodyPred) {
					g_Vars.globals.m_flBodyPred = g_Vars.globals.m_flAnimTime + CSGO_ANIM_LOWER_REALIGN_DELAY;
					g_Vars.globals.m_flBody = g_Vars.globals.RegularAngles.y;
				}
			}
		}

		// build bones at the end of everything
		{
			g_BoneSetup.BuildBones(local, BONE_USED_BY_ANYTHING, BoneSetupFlags::None);

			g_Vars.globals.flRealYaw = state->m_flAbsRotation;
			g_Vars.globals.angViewangles = cmd->viewangles;

			// copy real bone positions
			auto boneCount = local->m_CachedBoneData().Count();
			std::memcpy(g_Vars.globals.m_RealBonesPositions, local->m_vecBonePos(), boneCount * sizeof(Vector));
			std::memcpy(g_Vars.globals.m_RealBonesRotations, local->m_quatBoneRot(), boneCount * sizeof(Quaternion));

			local->m_AnimOverlay().Element(12).m_flWeight = flWeight12Backup;

			std::memcpy(local->m_AnimOverlay().m_Memory.m_pMemory, old_anim_layers, 13 * sizeof(C_AnimationLayer));
			std::memcpy(local->m_flPoseParameter(), old_pose_params, sizeof(local->m_flPoseParameter()));

			if (local->m_CachedBoneData().Base() != local->m_BoneAccessor().m_pBones) {
				std::memcpy(local->m_BoneAccessor().m_pBones, local->m_CachedBoneData().Base(), local->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
			}

			local->SetupBones(g_Vars.globals.LagPosition, 128, BONE_USED_BY_ANYTHING, Interfaces::m_pGlobalVars->curtime);
		}

		// save updated data.
		g_Vars.globals.m_bGround = state->m_bOnGround;
		g_Vars.globals.m_fFlags = local->m_fFlags();
	}

	std::deque< NetPos >   m_net_pos;
	bool CreateMoveHandler(float ft, CUserCmd* _cmd, bool* bSendPacket, bool* bFinalTick) {
		auto bRet = oCreateMove(ft, _cmd);

		g_Vars.globals.m_bInCreateMove = true;

		auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal || pLocal->IsDead()) {
			g_Vars.globals.WasShootingInPeek = false;
			AutoPeekPos.Set();

			Engine::Prediction::Instance().Invalidate();
			g_Vars.globals.m_bInCreateMove = false;
			return bRet;
		}

		auto weapon = (C_WeaponCSBaseGun*)(pLocal->m_hActiveWeapon().Get());
		if (!weapon) {
			Engine::Prediction::Instance().Invalidate();
			g_Vars.globals.m_bInCreateMove = false;

			return bRet;
		}

		g_Vars.globals.CurrentLocalViewAngles = _cmd->viewangles;

		g_Vars.globals.m_flCurtime = Interfaces::m_pGlobalVars->curtime;

		Encrypted_t<CUserCmd> cmd(_cmd);

		static auto m_iCrosshairData = Interfaces::m_pCvar->FindVar(XorStr("weapon_debug_spread_show"));
		if (g_Vars.esp.force_sniper_crosshair && m_iCrosshairData) {
			m_iCrosshairData->SetValue(!pLocal->m_bIsScoped() ? 3 : 0);
		}
		else {
			if (m_iCrosshairData)
				m_iCrosshairData->SetValue(0);
		}

		static auto g_GameRules = *(uintptr_t**)(Engine::Displacement.Data.m_GameRules);
		bool invalid = g_GameRules && *(bool*)(*(uintptr_t*)g_GameRules + 0x20) || (pLocal->m_fFlags() & (1 << 6));

		Encrypted_t<CVariables::GLOBAL> globals(&g_Vars.globals);

		static QAngle lockedAngles = QAngle();

		if (g_Vars.globals.WasShootingInChokeCycle)
			cmd->viewangles = lockedAngles;

		//if( g_Vars.rage.enabled )
			//cmd->tick_count += TIME_TO_TICKS( Engine::LagCompensation::Get( )->GetLerp( ) );

		auto movement = Interfaces::Movement::Get();

		if (Menu::opened) {
			// just looks nicer
			auto RemoveButtons = [&](int key) { cmd->buttons &= ~key; };
			RemoveButtons(IN_ATTACK);
			RemoveButtons(IN_ATTACK2);
			RemoveButtons(IN_USE);

			if (ImGui::GetCurrentContext() != NULL) {
				if (Menu::initialized && ImGui::GetIO().WantTextInput) {
					RemoveButtons(IN_MOVERIGHT);
					RemoveButtons(IN_MOVELEFT);
					RemoveButtons(IN_FORWARD);
					RemoveButtons(IN_BACK);

					movement->InstantStop(cmd.Xor());
				}
			}
		}

		g_Vars.globals.m_pCmd = cmd.Xor();

		auto weaponInfo = weapon->GetCSWeaponData();

		g_Vars.globals.bCanWeaponFire = pLocal->CanShoot();

		//g_TickbaseController.PreMovement( );

		//Engine::Prediction::Instance()->KeepCommunication(bSendPacket, cmd->command_number);

		g_Vars.globals.m_vecFixedEyePosition = pLocal->GetEyePosition();

		Engine::Prediction::Instance()->RunGamePrediction();

		auto& prediction = Engine::Prediction::Instance();

		Engine::g_ResolverData->m_player_fire = Interfaces::m_pGlobalVars->curtime >= C_CSPlayer::GetLocalPlayer()->m_flNextAttack() && !g_Vars.globals.IsRoundFreeze;

		movement->PrePrediction(cmd, pLocal, bSendPacket, bFinalTick, nullptr);
		prediction.Begin(cmd, bSendPacket, cmd->command_number);
		{
			g_Vars.globals.m_bAimbotShot = false;

			if (g_Vars.misc.autopeek && g_Vars.misc.autopeek_bind.enabled) {
				if ((pLocal->m_fFlags() & FL_ONGROUND)) {
					if (AutoPeekPos.IsZero()) {
						AutoPeekPos = pLocal->GetAbsOrigin();
					}
				}
			}
			else {
				AutoPeekPos = Vector();
			}

			movement->InPrediction();
			movement->PostPrediction();

			g_Vars.globals.m_vecVelocity = pLocal->m_vecVelocity();

			if (!g_Vars.misc.slide_walk) {
				if (pLocal->m_MoveType() != MOVETYPE_LADDER && pLocal->m_MoveType() != MOVETYPE_NOCLIP && pLocal->m_MoveType() != MOVETYPE_FLY)
					cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);
			}
			else {
				if (pLocal->m_MoveType() != MOVETYPE_LADDER && pLocal->m_fFlags() & FL_ONGROUND) {
					if (cmd->forwardmove > 0) {
						cmd->buttons |= IN_BACK;
						cmd->buttons &= ~IN_FORWARD;
					}

					if (cmd->forwardmove < 0) {
						cmd->buttons |= IN_FORWARD;
						cmd->buttons &= ~IN_BACK;
					}

					if (cmd->sidemove < 0) {
						cmd->buttons |= IN_MOVERIGHT;
						cmd->buttons &= ~IN_MOVELEFT;
					}

					if (cmd->sidemove > 0) {
						cmd->buttons |= IN_MOVELEFT;
						cmd->buttons &= ~IN_MOVERIGHT;
					}
				}
			}

			int nShotCmd = -1;

			if (weaponInfo.IsValid()) {
				if (cmd->buttons & IN_ATTACK
					&& weapon->m_iItemDefinitionIndex() != WEAPON_C4
					&& weaponInfo->m_iWeaponType >= WEAPONTYPE_KNIFE
					&& weaponInfo->m_iWeaponType <= WEAPONTYPE_MACHINEGUN
					&& pLocal->CanShoot())
				{
					nShotCmd = cmd->command_number;
					g_Vars.globals.m_iShotTick = cmd->tick_count;
					lockedAngles = cmd->viewangles;
					LastShotTime = Interfaces::m_pGlobalVars->tickcount;

					if (weaponInfo->m_iWeaponType != WEAPONTYPE_KNIFE && weaponInfo->m_iWeaponType != WEAPONTYPE_GRENADE) {
						g_Vars.globals.m_flLastShotTime = Interfaces::m_pGlobalVars->realtime;
						//if( g_Vars.globals.bInRagebot ) {
						//	g_Vars.globals.m_flLastShotTimeInRage = g_Vars.globals.m_flLastShotTime;
						//}
					}

					g_Vars.globals.WasShootingInChokeCycle = !(*bSendPacket);
					g_Vars.globals.WasShooting = true;

					if (weaponInfo->m_iWeaponType != WEAPONTYPE_KNIFE)
						g_Vars.globals.WasShootingInPeek = true;

					//g_Vars.globals.m_ShotAngle = Interfaces::m_pInput->m_pCommands[ nShotCmd % 150 ].viewangles;

				}
				else {
					g_Vars.globals.WasShooting = false;
				}
			}

			g_Vars.globals.iWeaponIndex = weapon->m_iItemDefinitionIndex();

			g_Vars.globals.m_flPreviousDuckAmount = pLocal->m_flDuckAmount();

			Engine::C_ShotInformation::Get()->CorrectSnapshots(*bSendPacket);

			UserCmdDoubletap(cmd.Xor());

			UpdateInformation(cmd.Xor(), *bSendPacket);

			//walkbot::Instance().move( cmd.Xor( ) );

			g_Vars.globals.m_bOldShot = g_Vars.globals.m_bAimbotShot;
			g_Vars.globals.m_bOldPacket = *bSendPacket;

			//	g_TickbaseController.PostMovement( bSendPacket, cmd.Xor( ) );
		}
		prediction.End();

		if (!g_Vars.misc.slide_walk)
			if (pLocal->m_MoveType() != MOVETYPE_LADDER && pLocal->m_MoveType() != MOVETYPE_NOCLIP && pLocal->m_MoveType() != MOVETYPE_FLY)
				cmd->buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT);

		if (g_Vars.antiaim.enabled && g_Vars.antiaim.manual && g_Vars.antiaim.mouse_override.enabled) {
			pLocal->pl().v_angle = globals->PreviousViewangles;
		}

		if (*bSendPacket) {
			g_Vars.globals.WasShootingInChokeCycle = false;

			g_Vars.globals.LastChokedCommands = Interfaces::m_pClientState->m_nChokedCommands();

			if (g_Vars.globals.FixCycle) {
				g_Vars.globals.FixCycle = false;
				g_Vars.globals.UnknownCycleFix = true;
			}

			OutgoingTickcount = Interfaces::m_pGlobalVars->tickcount;

			// TODO: make this lag compensated
			g_Vars.globals.m_iNetworkedTick = pLocal->m_nTickBase();
			g_Vars.globals.m_vecNetworkedOrigin = pLocal->m_vecOrigin();
		}

		if (!*bSendPacket || !*bFinalTick) {
			g_Vars.globals.RegularAngles = cmd->viewangles;
		}

		if (*bSendPacket) {
			//g_Vars.globals.cmds.push_back( cmd->command_number );

			Vector cur = pLocal->m_vecOrigin();
			Vector prev = m_net_pos.empty() ? cur : m_net_pos.front().m_pos;

			g_Vars.globals.bBrokeLC = (cur - prev).LengthSquared() > 4096.f;
			g_Vars.globals.delta = std::clamp((cur - prev).LengthSquared(), 0.f, 4096.f);

			m_net_pos.emplace_front(Interfaces::m_pGlobalVars->curtime, cur);
		}

		for (auto hk : g_luahookmanager.get_hooks(XorStr("createmove"))) hk.func(_cmd);

		g_Vars.globals.bFinalPacket = *bSendPacket;

		if (g_Vars.misc.anti_untrusted) {
			cmd->viewangles.Normalize();
			cmd->viewangles.Clamp();
		}

		auto& out = g_Vars.globals.cmds.emplace_back();

		//out.is_outgoing = *bSendPacket;
		//out.is_used = false;
		//out.command_number = cmd->command_number;
		//out.previous_command_number = 0;

		while (g_Vars.globals.cmds.size() > (int)(1.0f / Interfaces::m_pGlobalVars->interval_per_tick))
			g_Vars.globals.cmds.pop_front();

		if (!*bSendPacket)
		{
			auto net_channel = Interfaces::m_pEngine->GetNetChannelInfo();

			if (net_channel) {

				if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
				{
					auto backup_choke = net_channel->m_nChokedPackets;
					net_channel->m_nChokedPackets = 0;

					net_channel->SendDatagram();
					--net_channel->m_nOutSequenceNr;

					net_channel->m_nChokedPackets = backup_choke;
				}
			}
			else
				g_Vars.globals.cmds.clear();
		}
		else {
			g_Vars.globals.cmds.push_back(cmd->command_number);
		}

		g_Vars.globals.m_bInCreateMove = false;

		return false;
	}

	bool __stdcall CreateMove(float ft, CUserCmd* _cmd) {
		g_Vars.globals.szLastHookCalled = XorStr("2");
		if (!_cmd || !_cmd->command_number)
			return oCreateMove(ft, _cmd);

		sunsetmode();

		if (g_Vars.engine_no_focus_sleep->GetInt() != 0)
			g_Vars.engine_no_focus_sleep->SetValue(0);

		PreserveKillfeed();

		Encrypted_t<uintptr_t> pAddrOfRetAddr((uintptr_t*)_AddressOfReturnAddress());
		bool* bFinalTick = reinterpret_cast<bool*>(uintptr_t(pAddrOfRetAddr.Xor()) + 0x15);
		bool* bSendPacket = reinterpret_cast<bool*>(uintptr_t(pAddrOfRetAddr.Xor()) + 0x14);

		if (!(*bSendPacket))
			*bSendPacket = true;

		if (!*bFinalTick)
			*bSendPacket = false;

		int iLagLimit = 16;
		g_Vars.fakelag.iLagLimit = std::clamp(iLagLimit, 0, 16);

		auto result = CreateMoveHandler(ft, _cmd, bSendPacket, bFinalTick);

		//Engine::Prediction::Instance( )->KeepCommunication( bSendPacket, _cmd->command_number );

		auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!g_Vars.globals.HackIsReady || !pLocal || !Interfaces::m_pEngine->IsInGame()) {
			Engine::Prediction::Instance().Invalidate();
			return oCreateMove(ft, _cmd);
		}

		if (g_TickbaseController.bShifting) {
			*bSendPacket = g_TickbaseController.s_nExtraProcessingTicks == 1; // Only send on the last shifted tick
			_cmd->buttons &= ~(IN_ATTACK | IN_ATTACK2);
		}

		return result;
	}

	bool __cdecl ReportHit(Hit_t* hit) {
		if ((g_Vars.esp.visualize_hitmarker_world || g_Vars.esp.visualize_damage) && hit) {
			Hitmarkers::AddWorldHitmarker(hit->x, hit->y, hit->z);
			//Hitmarkers::m_bShouldDrawDamage = false;
		}

		return oReportHit(hit);
	}

	bool __cdecl IsUsingStaticPropDebugMode()
	{
		if (Interfaces::m_pEngine.IsValid() && !Interfaces::m_pEngine->IsInGame())
			return oIsUsingStaticPropDebugMode();

		return g_Vars.esp.night_mode;
	}

	void __vectorcall CL_Move(bool bFinalTick, float accumulated_extra_samples) {
		g_TickbaseController.OnCLMove(bFinalTick, accumulated_extra_samples);
	}

	void __fastcall RunSimulation(void* this_, void*, int iCommandNumber, CUserCmd* pCmd, size_t local) {
		g_TickbaseController.OnRunSimulation(this_, iCommandNumber, pCmd, local);
	}

	void __fastcall PredictionUpdate(void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) {
		g_TickbaseController.OnPredictionUpdate(prediction, nullptr, startframe, validframe, incoming_acknowledged, outgoing_command);
	}
}
