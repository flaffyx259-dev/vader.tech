#include "AntiAim.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Miscellaneous/Movement.hpp"
#include "../../source.hpp"
#include "../../Utils/InputSys.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "LagCompensation.hpp"
#include "Autowall.h"
#include "../Game/SimulationContext.hpp"
#include "../../SDK/displacement.hpp"
#include "../../Renderer/Render.hpp"
#include "TickbaseShift.hpp"
#include "../Visuals/ESP.hpp"
#include <random>
#include "Resolver.hpp"
#include "Ragebot.hpp"
#include "FakeLag.hpp"

#define min2(a, b) (((a) < (b)) ? (a) : (b))

float quick_normalize(float degree, const float min, const float max) {
	while (degree < min)
		degree += max - min;
	while (degree > max)
		degree -= max - min;

	return degree;
}

bool trace_to_exit_short(Vector& point, Vector& dir, const float step_size, float max_distance)
{
	float flDistance = 0;

	while (flDistance <= max_distance)
	{
		flDistance += step_size;

		point += dir * flDistance;
		int point_contents = Interfaces::m_pEngineTrace->GetPointContents(point, MASK_SHOT_HULL);
		if (!(point_contents & MASK_SHOT_HULL))
		{
			// found first free point
			return true;
		}
	}

	return false;
}

float get_thickness(Vector& start, Vector& end, float distance) {
	Vector dir = end - start;
	Vector step = start;
	dir /= dir.Length();
	CTraceFilterWorldOnly filter;
	CGameTrace trace;
	Ray_t ray;
	float thickness = 0;
	while (true) {
		ray.Init(step, end);
		Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

		if (!trace.DidHit())
			break;

		const Vector lastStep = trace.endpos;
		step = trace.endpos;

		if ((end - start).Length() <= (step - start).Length())
			break;

		if (!trace_to_exit_short(step, dir, 5, 90))
			return FLT_MAX;

		thickness += (step - lastStep).Length();
	}
	return thickness;
}


//__m128 sub_384B9090(float* fl)
//{
//	float* v1; // edx
//	bool v3; // cc
//	float* v4; // eax
//	float* v5; // ecx
//	float* v6; // eax
//	float v7; // xmm0_4
//	__m128d v8; // xmm1
//	float v9; // xmm2_4
//	__m128 v10; // xmm3
//	float* v11; // eax
//	int v13; // [esp+0h] [ebp-Ch] BYREF
//	int v14; // [esp+4h] [ebp-8h] BYREF
//	int v15; // [esp+8h] [ebp-4h] BYREF
//
//	v1 = (float*)&v15;
//	v14 = 1065353216;
//	v3 = fl[63] >= 1.0;
//	v4 = fl + 63;
//	v15 = 1065353216;
//	v5 = fl + 62;
//	v13 = 0;
//	if (!v3)
//		v1 = v4;
//	v6 = (float*)&v14;
//	if (*v5 <= 1.0)
//		v6 = v5;
//	v7 = *v6;
//	if (*v6 <= 0.0)
//		v7 = 0.0;
//	v8 = { 0L };
//	v9 = fl[41];
//	v8.m128d_f64[0] = v7 * (fl[71] * -0.30000001 - 0.19999999) + 1.0;
//	v10 = _mm_cvtpd_ps(v8);
//	if (v9 > 0.0)
//	{
//		v11 = (float*)&v13;
//		if (*v1 > 0.0)
//			v11 = v1;
//		v10.m128_f32[0] = v10.m128_f32[0] + (float)((float)(v9 * *v11) * (float)(0.5 - v10.m128_f32[0]));
//	}
//	v10.m128_f32[0] = v10.m128_f32[0] * fl[205];
//	return v10;
//}
//
//void RetardedLBYAntiaim(CUserCmd* __pCmd, int a1, int somePassedValue, float LowerBodyYawTarget)
//{
//	float* v7; // edx
//	float v8; // xmm3_4
//	float insub_LowerBodyYaw; // xmm4_4
//	int v10; // eax
//	float flMicromoveToAdd; // xmm0_4
//	int v12; // ecx
//	CUserCmd* pCmd; // eax
//	float addM; // xmm3_4
//	float toAdd; // xmm0_4
//	float addYaw; // xmm0_4
//	float addY; // xmm0_4
//	float flForwardMove; // xmm2_4
//	float flSideMove; // xmm1_4
//	float v20; // xmm0_4
//	float v21; // xmm3_4
//	int v22; // edx
//	int v23; // ecx
//	char v24; // cl
//	float v25; // xmm0_4
//	float v26; // xmm0_4
//	float v27; // xmm1_4
//	CUserCmd* v28; // eax
//	float v29; // xmm0_4
//	bool v30; // cl
//	float v31; // xmm4_4
//	float v32; // xmm1_4
//	float v33; // [esp+4h] [ebp-28h]
//	float v36; // [esp+18h] [ebp-14h]
//	float v37; // [esp+18h] [ebp-14h]
//	bool v38 = true; // [esp+1Fh] [ebp-Dh]
//	static bool bSomeFlippingFlag = false;
//	int dword_385B8A58 = Interfaces::m_pClientState->m_nChokedCommands();
//	static DWORD dword_3859532C = NULL; // This is literally never set. Gigabrain devs btw
//
//	auto localPlayer = C_CSPlayer::GetLocalPlayer();
//
//	if (!localPlayer)
//		return;
//
//	if (!(localPlayer->m_fFlags() & FL_ONGROUND) /*|| (v38 = 1, (dword_385958A0 & 2) != 0)*/ /*This is some retarded prediction shit that you don't need*/)
//		v38 = false;
//	v7 = *(float**)(((DWORD)localPlayer) + 14452);
//	if (v7/**(DWORD*)(Globals::pLocal + 14452)*/)
//	{
//		v8 = sub_384B9090(v7).m128_u32[0];
//		if (false /*I believe this is a settings thing, so make your own options for it*/ /*byte_386D8C09 == 2 || byte_386D8C09 == 3*/)
//		{
//			insub_LowerBodyYaw = LowerBodyYawTarget;
//			if (a1)
//			{
//				insub_LowerBodyYaw = fabs(LowerBodyYawTarget);
//				if (a1 == 2)
//				{
//					if (true/*byte_386D8C09 != 2*/)
//						insub_LowerBodyYaw = -insub_LowerBodyYaw;
//				}
//				else if (true/*byte_386D8C09 == 2*/)
//				{
//					insub_LowerBodyYaw = -insub_LowerBodyYaw;
//				}
//			}
//		}
//		else
//		{
//			insub_LowerBodyYaw = LowerBodyYawTarget;
//		}
//		if (false /*this is some setting*/ /*byte_385B89FF*/)
//		{
//			v10 = dword_3859532C;
//			if (!dword_3859532C)
//			{
//				if ((float)(fabs(__pCmd->forwardmove) + fabs(__pCmd->sidemove)) >= 1.0 || !v38)
//					return;
//				if ((__pCmd->command_number & 1) != 0)
//					flMicromoveToAdd = 1.1;
//				else
//					flMicromoveToAdd = -1.1;
//				__pCmd->sidemove = __pCmd->sidemove + flMicromoveToAdd;
//				v10 = dword_3859532C;
//			}
//
//			// This is not used in this feature so idk why it's here
//			// if (v10 == 1 && dword_385B8A50 < 3)
//			//     dword_385B8A50 = 3;
//		}
//		else
//		{
//			switch (somePassedValue)
//			{
//			case 1:
//				v12 = dword_3859532C;
//				pCmd = __pCmd;
//				addM = 1.1;
//				// v7 = v38;
//				// LOBYTE(v7) = v38;
//				if (!dword_3859532C)
//				{
//					if ((float)(fabs(__pCmd->forwardmove) + fabs(__pCmd->sidemove)) < 1.0 && v38)
//					{
//						if ((__pCmd->command_number & 1) != 0)
//							toAdd = 1.1;
//						else
//							toAdd = -1.1;
//						__pCmd->sidemove = __pCmd->sidemove + toAdd;
//					}
//					if (insub_LowerBodyYaw <= 0.0)
//						addYaw = __pCmd->viewangles.y - fabs(insub_LowerBodyYaw);
//					else
//						addYaw = insub_LowerBodyYaw + __pCmd->viewangles.y;
//					__pCmd->viewangles.y = addYaw;
//					v12 = dword_3859532C;
//				}
//				if (v12 == 1)
//				{
//					if (dword_385B8A58)
//					{
//						if (insub_LowerBodyYaw <= 0.0)
//							v20 = __pCmd->viewangles.y + -110.0;
//						else
//							v20 = __pCmd->viewangles.y + 110.0;
//						__pCmd->viewangles.y = v20;
//					}
//					else
//					{
//						if (insub_LowerBodyYaw >= 0.0)
//							addY = -110.0;
//						else
//							addY = 110.0;
//						flForwardMove = __pCmd->forwardmove;
//						flSideMove = fabs(__pCmd->sidemove);
//						__pCmd->viewangles.y = addY + __pCmd->viewangles.y;
//						if ((float)(flSideMove + flForwardMove) < 1.f && v38)
//						{
//							if ((__pCmd->command_number & 1) == 0)
//								addM = -1.1;
//							__pCmd->forwardmove = flForwardMove + addM;
//						}
//					}
//				}
//				else
//				{
//					if (v12 == 2)
//					{
//						v21 = Math::normalize_float(140.f /*Your LBY factor here I think*/);
//						// v21 = sub_384E72B0(2, v7).m128_f32[0];
//						int whatever = (((DWORD)localPlayer) + 14452);
//						whatever = *(DWORD*)(whatever + 128);
//						if (fabs(v21 - Math::normalize_float(whatever) >= 10.0))
//							v24 = bSomeFlippingFlag;
//						else
//						{
//							v24 = bSomeFlippingFlag == 0;
//							bSomeFlippingFlag = !bSomeFlippingFlag;
//						}
//
//						if (!v24)
//							insub_LowerBodyYaw = -insub_LowerBodyYaw;
//					}
//					__pCmd->viewangles.y = insub_LowerBodyYaw + __pCmd->viewangles.y;
//				}
//				break;
//			case 2:
//				if ((float)(fabs(__pCmd->forwardmove) + fabs(__pCmd->sidemove)) < 1.0 && v38)
//				{
//					if ((__pCmd->command_number & 1) != 0)
//						v25 = 1.1;
//					else
//						v25 = -1.1;
//					__pCmd->sidemove = __pCmd->sidemove + v25;
//				}
//				v33 = fabs(insub_LowerBodyYaw);
//				if (bSomeFlippingFlag)
//				{
//					v36 = RandomFloat(0.f, v33);
//					v26 = v36;
//				}
//				else
//				{
//					v37 = RandomFloat(0.f, v33);
//					v26 = -v37;
//				}
//				v27 = fabs(v26) + v8;
//				if (v26 <= 0.0)
//					__pCmd->viewangles.y = __pCmd->viewangles.y - v27;
//				else
//					__pCmd->viewangles.y = v27 + __pCmd->viewangles.y;
//				if (!dword_385B8A58)
//					bSomeFlippingFlag = !bSomeFlippingFlag;
//				break;
//			case 3:
//				v28 = __pCmd;
//				if ((float)(fabs(__pCmd->forwardmove) + fabs(__pCmd->sidemove)) < 1.0 && v38)
//				{
//					if ((__pCmd->command_number & 1) != 0)
//						v29 = 1.1;
//					else
//						v29 = -1.1;
//					__pCmd->sidemove = __pCmd->sidemove + v29;
//				}
//				v30 = bSomeFlippingFlag;
//				v31 = fabs(insub_LowerBodyYaw);
//				if (!bSomeFlippingFlag)
//					v31 = -v31;
//				v32 = fabs(v31) + v8;
//				if (v31 <= 0.0)
//					v28->viewangles.y = v28->viewangles.y - v32;
//				else
//					v28->viewangles.y = v32 + v28->viewangles.y;
//				if (!dword_385B8A58)
//					bSomeFlippingFlag = !v30;
//				break;
//			}
//		}
//	}
//}

namespace Interfaces
{
	class C_AntiAimbot : public AntiAimbot {
	public:
		void fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
		void Main(bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot) override;
		void PrePrediction(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
		void ImposterBreaker(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) override;
	private:
		virtual float GetAntiAimX(Encrypted_t<CVariables::ANTIAIM_STATE> settings);
		virtual float GetAntiAimY(Encrypted_t<CVariables::ANTIAIM_STATE> settings, Encrypted_t<CUserCmd> cmd);

		virtual void Distort(Encrypted_t<CUserCmd> cmd);

		enum class Directions : int {
			YAW_RIGHT = -1,
			YAW_BACK,
			YAW_LEFT,
			YAW_NONE,
		};
		virtual Directions HandleDirection(Encrypted_t<CUserCmd> cmd);

		bool DoEdgeAntiAim(C_CSPlayer* player, QAngle& out);

		void AutoDirection(Encrypted_t<CUserCmd> cmd);

		void freestanding();

		void FakeFlick(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool airstuck();

		//void fake_flick(Encrypted_t<CUserCmd> cmd);

		void lby_prediction(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool lby_update(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		bool do_lby(Encrypted_t<CUserCmd> cmd, bool* bSendPacket);

		virtual bool IsEnabled(Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings);

		bool m_bNegate = false;
		float m_flLowerBodyUpdateTime = 0.f;
		float m_flLowerBodyUpdateYaw = FLT_MAX;

		bool   EdgeFlick = false;
		float  m_auto;
		float  m_auto_dist;
		float  m_auto_last;
		float  m_view;
		float  m_auto_time;

		bool broke_lby = false;
		float next_lby_update = 0.f;
		bool update_lby = false;
		float initial_lby = 0.f;
		float target_lby = 0.f;
		bool firstupdate = true;
		bool secondupdate = true;
	};

	static float next_lby_update_time = 0;

	bool is_safe_angle(float yaw)
	{
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || !local->IsAlive())
			return false;

		// best target.
		struct AutoTarget_t { float fov; C_CSPlayer* player; };
		AutoTarget_t target{ 180.f + 1.f, nullptr };
		// iterate players, for closest distance.
		for (int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);
			if (!player || player->IsDormant() || player == local || player->IsDead() || player->m_iTeamNum() == local->m_iTeamNum())
				continue;

			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
				auto direction = end - start;
				auto aimAngles = direction.ToEulerAngles();
				auto delta = aimAngles - angles;
				delta.Normalize();

				return sqrtf(delta.x * delta.x + delta.y * delta.y);
			};

			float fov = AngleDistance(g_Vars.globals.CurrentLocalViewAngles, local->GetEyePosition(), player->WorldSpaceCenter());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		if (!target.player) {
			return false;
		}

		C_CSPlayer* threat_id = target.player;
		if (!threat_id)
			return false;

		auto weapon = (C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get();
		if (!weapon)
			return false;

		auto weaponData = weapon->GetCSWeaponData();
		if (!weaponData.IsValid())
			return false;

		float rad = Math::deg_to_rad(yaw);
		Vector head_pos_flick = local->GetEyePosition();

		head_pos_flick.x += cos(rad) * 25.f;
		head_pos_flick.y += sin(rad) * 25.f;

		//Interfaces::m_pDebugOverlay->AddBoxOverlay(head_pos_flick, Vector(-0.7f, -0.7f, -0.7f), Vector(0.7f, 0.7f, 0.7f), QAngle(0.f, 0.f, 0.f), 0, 255, 0, 100, Interfaces::m_pGlobalVars->interval_per_tick * 2);

		float flick_damage = 0.f;

		Autowall::C_FireBulletData data;
		CTraceFilterWorldOnly filter;

		data.m_bPenetration = true;
		data.m_Player = local;
		data.m_Weapon = weapon;
		data.m_WeaponData = weaponData.Xor();
		data.m_vecStart = head_pos_flick;
		data.m_bShouldIgnoreDistance = true;

		data.m_vecDirection = target.player->GetEyePosition() - head_pos_flick;
		data.m_flPenetrationDistance = data.m_vecDirection.Normalize();
		data.m_Filter = &filter;

		flick_damage = Autowall::FireBullets(&data);

		if (flick_damage < 10.f)
			return false; // safe

		return true; // no safe
	}


	void C_AntiAimbot::lby_prediction(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		auto localPlayer = C_CSPlayer::GetLocalPlayer();

		if (!localPlayer)
			return;

		const auto animstate = localPlayer->m_PlayerAnimState();
		if (!animstate)
			return;

		if (Interfaces::m_pClientState->m_nChokedCommands() || (g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick))
			return;

		if (g_Vars.antiaim.lby_disable_fakewalk && g_Vars.globals.Fakewalking)
			return;

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1 && g_Vars.antiaim.manual;

		if (g_Vars.antiaim.lby_disable_manual && bUsingManualAA)
			return;

		if (g_Vars.antiaim.lby_disable_unsafe && is_safe_angle(initial_lby)) {
			return;
		}

		if (animstate->m_velocity > 0.1f || localPlayer->m_vecVelocity().Length2D() > 0.1f)
		{
			g_Vars.globals.m_flBodyPred += 0.22f;
			firstupdate = true;
		}
		else if (Interfaces::m_pGlobalVars->curtime > g_Vars.globals.m_flBodyPred)
		{
			update_lby = true;
		}

		const auto get_add_by_choke = [&]() -> float
		{
			static auto max = 137.f;
			static auto min = 100.f;

			auto mult = 1.f / 0.2f * TICKS_TO_TIME(Interfaces::m_pClientState->m_nChokedCommands());

			return 100.f + (max - min) * mult;
		};

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		if (firstupdate && animstate->m_velocity <= 0.1f || firstupdate && animstate->m_vecVelocity.Length() <= 0.1f && settings->yaw == 1)
		{
			initial_lby = cmd->viewangles.y + g_Vars.antiaim.break_lby_first;

			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}
			secondupdate = true;
			firstupdate = false;
		}

		if (!firstupdate && Interfaces::m_pGlobalVars->curtime + TICKS_TO_TIME(Interfaces::m_pClientState->m_nChokedCommands() + 1) > g_Vars.globals.m_flBodyPred
			&& fabsf(Math::normalize_float(cmd->viewangles.y - initial_lby)) < get_add_by_choke() && g_Vars.antiaim.lby_breaker)
		{
			if (g_Vars.antiaim.optimal_adjust) {
				cmd->viewangles.y = initial_lby + get_add_by_choke();
			}
			else {
				cmd->viewangles.y = initial_lby;
			}

			//cmd->viewangles.y = initial_lby;

			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}

		}

	}


	bool C_AntiAimbot::lby_update(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		auto localPlayer = C_CSPlayer::GetLocalPlayer();

		if (!localPlayer)
			return false;

		if (Interfaces::m_pClientState->m_nChokedCommands() || !(localPlayer->m_fFlags() & FL_ONGROUND) || (g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick))
			return false;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		const auto updated = update_lby;

		if (update_lby && g_Vars.antiaim.lby_breaker)
		{

			auto angles = cmd->viewangles.y;
			if (g_Vars.globals.Fakewalking) {
				*bSendPacket = true;
			}
			target_lby = initial_lby;
			cmd->viewangles.y = initial_lby;
			cmd->viewangles.Clamp();
			update_lby = false;

			if (secondupdate || !g_Vars.antiaim.static_angle)
			{

				if (g_Vars.antiaim.static_angle)
				{
					initial_lby += -g_Vars.antiaim.break_lby_first + g_Vars.antiaim.break_lby;
				}
				else
				{
					initial_lby = angles + g_Vars.antiaim.break_lby;
				}

				secondupdate = false;
			}
		}

		return updated;
	}

	bool C_AntiAimbot::do_lby(Encrypted_t<CUserCmd> cmd, bool* bSendPacket)
	{
		lby_prediction(cmd, bSendPacket);

		return lby_update(cmd, bSendPacket);
	}

	void C_AntiAimbot::FakeFlick(Encrypted_t<CUserCmd> cmd, bool* bSendPacket) {
		if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
			auto localPlayer = C_CSPlayer::GetLocalPlayer();

			if (!localPlayer || !localPlayer->IsAlive())
				return;

			switch (g_Vars.misc.mind_trick_mode) {
			case 0: {
				if (localPlayer->m_vecVelocity().Length2D() < 14.f) {
					static bool FlickCheck = false;
					static bool FlickCheckSide = false;
					static bool FlickCheckSide2 = false;
					static bool MicroMoveSide = false;
					*bSendPacket = !(cmd->tick_count % 2 == 0);
					if (cmd->tick_count % 2 == 0) {
						if (FlickCheck) {
							FlickCheckSide2 = !FlickCheckSide2;
							cmd->viewangles.y -= FlickCheckSide2 ? 120 : -120;
							FlickCheck = false;
							if (cmd->forwardmove == 0) {
								MicroMoveSide = !MicroMoveSide;
								cmd->forwardmove = MicroMoveSide ? 13.37 : -13.37;
							}
							return;
						}
						if (cmd->tick_count % 8 == 0) {
							FlickCheck = true;
							FlickCheckSide = !FlickCheckSide;
							if (cmd->forwardmove == 0) {
								MicroMoveSide = !MicroMoveSide;
								cmd->forwardmove = MicroMoveSide ? 13.37 : -13.37;
							}
							cmd->viewangles.y += FlickCheckSide ? -115 : 115;
							return;
						}
					}
					else if (cmd->forwardmove == 0) {
						static bool retard = false;
						retard = !retard;
						cmd->forwardmove = retard ? 1.1 : -1.1;
					}
				}

				break;
			}
			case 1: {
				if (localPlayer->m_vecVelocity().Length2D() < 16.f && g_TickbaseController.s_nExtraProcessingTicks > 0) {
					static bool FlickCheck = false;
					static bool MicroMoveSide = false;
					*bSendPacket = !(cmd->tick_count % 2 == 0);
					if (cmd->tick_count % 2 == 0) {
						if (FlickCheck) {
							g_Vars.globals.shift_amount = 12;
							cmd->viewangles.y += 180;
							FlickCheck = false;
							if (cmd->forwardmove == 0) {
								MicroMoveSide = !MicroMoveSide;
								cmd->forwardmove = MicroMoveSide ? 13.37 : -13.37;
							}
							return;
						}
						if (cmd->tick_count % 18 == 0) {
							FlickCheck = true;
							if (cmd->forwardmove == 0) {
								MicroMoveSide = !MicroMoveSide;
								cmd->forwardmove = MicroMoveSide ? 13.37 : -13.37;
							}
							cmd->viewangles.y += 115; // -
							return;
						}
					}
					else if (cmd->forwardmove == 0) {
						static bool retard = false;
						retard = !retard;
						cmd->forwardmove = retard ? 1.1 : -1.1;
					}
				}

				break;
			}

			}
		}
	}

	void freestand(CUserCmd* cmd, QAngle& angle)
	{
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();
		if (!LocalPlayer || LocalPlayer->IsDead())
			return;

		static float last_real;
		bool no_active = true;
		float bestrotation = 0.f;
		float highestthickness = 0.f;
		Vector besthead;

		auto leyepos = LocalPlayer->GetEyePosition();
		auto headpos = LocalPlayer->GetHitboxPosition(HITBOX_HEAD);
		auto origin = LocalPlayer->GetAbsOrigin();

		auto checkWallThickness = [&](C_BasePlayer* pPlayer, Vector newhead) -> float
		{
			Vector endpos1, endpos2;
			Vector eyepos = pPlayer->GetEyePosition();

			Ray_t ray;
			ray.Init(newhead, eyepos);

			CTraceFilterSkipTwoEntities filter(pPlayer, LocalPlayer);

			CGameTrace trace1, trace2;
			Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

			if (trace1.DidHit())
				endpos1 = trace1.endpos;
			else
				return 0.f;

			ray.Init(eyepos, newhead);
			Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

			if (trace2.DidHit())
				endpos2 = trace2.endpos;

			float add = newhead.Distance(eyepos) - leyepos.Distance(eyepos) + 3.f;
			return endpos1.Distance(endpos2) + add / 3;
		};

		struct AutoTarget_t { float fov; C_CSPlayer* player; };
		AutoTarget_t target{ 180.f + 1.f, nullptr };
		// iterate players, for closest distance.
		for (int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);
			if (!player || player->IsDormant() || player == LocalPlayer || player->IsDead() || player->m_iTeamNum() == LocalPlayer->m_iTeamNum())
				continue;

			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
				auto direction = end - start;
				auto aimAngles = direction.ToEulerAngles();
				auto delta = aimAngles - angles;
				delta.Normalize();

				return sqrtf(delta.x * delta.x + delta.y * delta.y);
			};

			float fov = AngleDistance(g_Vars.globals.CurrentLocalViewAngles, LocalPlayer->GetEyePosition(), player->WorldSpaceCenter());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		float step = (2 * M_PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28

		float radius = fabs(Vector(headpos - origin).Length2D());

		if (!target.player)
		{
			no_active = true;
		}
		else
		{
			for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
			{
				Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

				float totalthickness = 0.f;

				no_active = false;

				totalthickness += checkWallThickness(target.player, newhead);

				if (totalthickness > highestthickness)
				{
					highestthickness = totalthickness;
					bestrotation = rotation;
					besthead = newhead;
				}
			}
		}

		int ServerTime = (float)LocalPlayer->m_nTickBase() * Interfaces::m_pGlobalVars->interval_per_tick;
		int value = ServerTime % 2;

		static int Ticks = 120;

		//auto air_value = fmod(Interfaces.pGlobalVars->curtime / 0.9f /*speed*/ * 130.0f, 90.0f); // first number is where the angle will end and how fast it goes about it, adding to the second number which is the starting angle.
		//  auto stand_value = fmod(Interfaces.pGlobalVars->curtime /*speed can go here too*/ * 120.0f, 70.0f); // again, first number is where the angle will end and how fast it goes about it, adding to the second number which is the starting angle.
		//pCmd->viewangles.y += Hacks.LocalPlayer->GetFlags() & FL_ONGROUND ? NextLBYUpdate() ? 165.0f - 35 : stand_value + 145.0f : 90 + air_value;

		if (no_active)
		{
			angle.y -= Ticks; // 180z using ticks
			Ticks += 4;

			if (Ticks > 240)
				Ticks = 120;
		}
		else
			angle.y = RAD2DEG(bestrotation);
	}

	bool C_AntiAimbot::IsEnabled(Encrypted_t<CUserCmd> cmd, Encrypted_t<CVariables::ANTIAIM_STATE> settings) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();
		if (!LocalPlayer || LocalPlayer->IsDead())
			return false;

		if (!(g_Vars.antiaim.bomb_activity && g_Vars.globals.BobmActivityIndex == LocalPlayer->EntIndex()) || !g_Vars.antiaim.bomb_activity)
			if ((cmd->buttons & IN_USE) && (!settings->desync_e_hold || LocalPlayer->m_bIsDefusing()))
				return false;

		if (LocalPlayer->m_MoveType() == MOVETYPE_NOCLIP)
			return false;

		static auto g_GameRules = *(uintptr_t**)(Engine::Displacement.Data.m_GameRules);
		if (g_GameRules && *(bool*)(*(uintptr_t*)g_GameRules + 0x20) || (LocalPlayer->m_fFlags() & (1 << 6)))
			return false;

		C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)LocalPlayer->m_hActiveWeapon().Get();

		if (!Weapon)
			return false;

		auto WeaponInfo = Weapon->GetCSWeaponData();
		if (!WeaponInfo.IsValid())
			return false;

		if (WeaponInfo->m_iWeaponType == WEAPONTYPE_GRENADE) {
			if (!Weapon->m_bPinPulled() || (cmd->buttons & (IN_ATTACK | IN_ATTACK2))) {
				float throwTime = Weapon->m_fThrowTime();
				if (throwTime > 0.f)
					return false;
			}
		}
		else {
			if ((WeaponInfo->m_iWeaponType == WEAPONTYPE_KNIFE && cmd->buttons & (IN_ATTACK | IN_ATTACK2)) || cmd->buttons & IN_ATTACK) {
				if (LocalPlayer->CanShoot())
					return false;
			}
		}

		if (LocalPlayer->m_MoveType() == MOVETYPE_LADDER)
			return false;

		return true;
	}

	Encrypted_t<AntiAimbot> AntiAimbot::Get() {
		static C_AntiAimbot instance;
		return &instance;
	}

	std::random_device random;
	std::mt19937 generator(random());

	void C_AntiAimbot::fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd)
	{
		int fakelag_limit = 16;
		int choked_goal = 7;
		bool should_crouch = Interfaces::m_pClientState->m_nChokedCommands() >= choked_goal;

		if (should_crouch)
			cmd->buttons |= IN_DUCK;
		else
			cmd->buttons &= ~IN_DUCK;

		*bSendPacket = Interfaces::m_pClientState->m_nChokedCommands() >= fakelag_limit;
	}

	bool boxhackbreaker_flip;

	void C_AntiAimbot::Main(bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer || LocalPlayer->IsDead())
			return;

		auto animState = LocalPlayer->m_PlayerAnimState();
		if (!animState)
			return;

		if (!g_Vars.antiaim.enabled)
			return;

		if (g_Vars.misc.balls)
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> m_mode = &g_Vars.antiaim_stand;

		if (g_Vars.antiaim.condition_specific) {
			if ((cmd->buttons & IN_JUMP) || !(LocalPlayer->m_fFlags() & FL_ONGROUND))
				m_mode = &g_Vars.antiaim_air;

			else if (LocalPlayer->m_vecVelocity().Length2D() > g_Vars.antiaim.stand_velocity_threshold)
				m_mode = &g_Vars.antiaim_move;
		}

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(m_mode);

		C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)LocalPlayer->m_hActiveWeapon().Get();

		if (!Weapon)
			return;

		auto WeaponInfo = Weapon->GetCSWeaponData();
		if (!WeaponInfo.IsValid())
			return;

		if (!IsEnabled(cmd, settings))
			return;

		m_auto_time = g_Vars.antiaim.timeout_time;

		static auto fakelagTrack = (int)g_Vars.fakelag.choke;

		static bool restoreFakelag = false;

        if (g_Vars.rage.dt_exploits && g_Vars.rage.key_dt.enabled) {

			//if (g_Vars.misc.mind_trick_test && g_Vars.misc.mind_trick_bind.enabled) {
			//	g_Vars.globals.shift_amount = Interfaces::m_pGlobalVars->tickcount % 16 ? 16 : 0;
			//}

			/*else*/ if (g_Vars.rage.exploit_lag) {
				g_Vars.globals.shift_amount = Interfaces::m_pGlobalVars->tickcount % 16 > 0 ? 16 : 0;
			}

			if (g_Vars.rage.exploit_lagcomp) {

				auto iChoked = Interfaces::m_pClientState->m_nChokedCommands();
				auto Speed2D = LocalPlayer->m_vecVelocity().Length2D();
				int shift_time = 0;
				bool bOnGround = LocalPlayer->m_fFlags() & FL_ONGROUND;


				if (Speed2D > 72)
					shift_time = bOnGround ? 1 : 5;

				if (++shift_time > 16)
					shift_time = 0;

				if (shift_time > iChoked)
					std::clamp(shift_time, 1, 4);

				if (shift_time > 0)
				{
					*bSendPacket = true;
					g_Vars.globals.shift_amount = shift_time > 0 ? 16 : 0;
				}

				//*bSendPacket = false;

				++shift_time = 14;
			}
		}

		if (LocalPlayer->m_MoveType() == MOVETYPE_LADDER) {
			auto eye_pos = LocalPlayer->GetEyePosition();

			CTraceFilterWorldAndPropsOnly filter;
			CGameTrace tr;
			Ray_t ray;
			float angle = 0.0f;
			while (true) {
				float cosa, sina;
				DirectX::XMScalarSinCos(&cosa, &sina, angle);

				Vector pos;
				pos.x = (cosa * 32.0f) + eye_pos.x;
				pos.y = (sina * 32.0f) + eye_pos.y;
				pos.z = eye_pos.z;

				ray.Init(eye_pos, pos,
					Vector(-1.0f, -1.0f, -4.0f),
					Vector(1.0f, 1.0f, 4.0f));
				Interfaces::m_pEngineTrace->TraceRay(ray, MASK_SOLID, &filter, &tr);
				if (tr.fraction < 1.0f)
					break;

				angle += DirectX::XM_PIDIV2;
				if (angle >= DirectX::XM_2PI) {
					return;
				}
			}

			float v23 = atan2(tr.plane.normal.x, std::fabsf(tr.plane.normal.y));
			float v24 = RAD2DEG(v23) + 90.0f;
			cmd->viewangles.pitch = 89.0f;
			if (v24 <= 180.0f) {
				if (v24 < -180.0f) {
					v24 = v24 + 360.0f;
				}
				cmd->viewangles.yaw = v24;
			}
			else {
				cmd->viewangles.yaw = v24 - 360.0f;
			}

			if (cmd->buttons & IN_BACK) {
				cmd->buttons |= IN_FORWARD;
				cmd->buttons &= ~IN_BACK;
			}
			else  if (cmd->buttons & IN_FORWARD) {
				cmd->buttons |= IN_BACK;
				cmd->buttons &= ~IN_FORWARD;
			}

			return;
		}

		bool move = LocalPlayer->m_vecVelocity().Length2D() > 0.1f && !g_Vars.globals.Fakewalking;

		// save view, depending if locked or not.
		if ((g_Vars.antiaim.freestand_lock && move) || !g_Vars.antiaim.freestand_lock)
			m_view = cmd->viewangles.y;

		cmd->viewangles.x = GetAntiAimX(settings);
		float flYaw = GetAntiAimY(settings, cmd);

		// do not allow 2 consecutive sendpacket true if faking angles.
		if (!(g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
			if (*bSendPacket && g_Vars.globals.m_bOldPacket)
				*bSendPacket = false;
		}

		auto prevTickCount = cmd->tick_count;

		auto tickAmount = INT_MAX / g_Vars.misc.move_exploit_intensity;

		if (g_Vars.misc.move_exploit && g_Vars.misc.move_exploit_key.enabled) {
			auto prevCommandNumber = cmd->command_number;

			if (*bSendPacket == false) {
				cmd->tick_count = tickAmount;
				cmd->command_number = prevCommandNumber;


				//*g_send_packet = true;
			}

			if (*bSendPacket)
			{
				auto curtime = Interfaces::m_pGlobalVars->curtime + .01f;
				if (Interfaces::m_pGlobalVars->curtime >= curtime) {
					cmd->tick_count = tickAmount;
					auto curtime2 = Interfaces::m_pGlobalVars->curtime + .015f;
					if (Interfaces::m_pGlobalVars->curtime >= curtime2) {
						cmd->tick_count = tickAmount;
						cmd->command_number = INT_MAX;
					}
					//g_cmd->command_number = tickAmount;
				}

			}
		}


		// https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/cl_main.cpp#L1877-L1881
		if (!*bSendPacket || !*bFinalPacket) {

			if (g_Vars.antiaim.desync_on_dt && g_TickbaseController.s_nExtraProcessingTicks > 0) {
				cmd->viewangles.y = cmd->viewangles.y + 180;
				if (cmd->sidemove == 0) {
					static bool boxhackbreaker_side_flip = false;
					cmd->sidemove = boxhackbreaker_side_flip ? 3.25 : -3.25;
					boxhackbreaker_side_flip = !boxhackbreaker_side_flip;
				}
				if (cmd->forwardmove == 0) {
					static bool boxhackbreaker_for_flip = false;
					cmd->forwardmove = boxhackbreaker_for_flip ? 3.25 : -3.25;
					boxhackbreaker_for_flip = !boxhackbreaker_for_flip;
				}
				boxhackbreaker_flip = !boxhackbreaker_flip;
				cmd->viewangles.y -= boxhackbreaker_flip ? (120 * (g_Vars.antiaim.desync_on_dt_invert.enabled ? -1 : 1)) : 0;
				g_Vars.globals.shift_amount = boxhackbreaker_flip ? 16 : 0;
			}
			else
				cmd->viewangles.y = flYaw;

			if (!(g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled)) {
				Distort(cmd);
			}

			FakeFlick(cmd, bSendPacket);

			//fake_flick(cmd);

		}
		else {
			//*bSendPacket = true;

			if (!(g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
				std::uniform_int_distribution random(-110, 110);

				cmd->viewangles.y = Math::AngleNormalize(flYaw + 180 + random(generator));
			}
			else {
				cmd->viewangles.y = flYaw;
			}

			//SendFakeFlick();
		}

		// one tick before the update.
		//if (!Interfaces::m_pClientState->m_nChokedCommands() && LocalPlayer->m_fFlags() & FL_ONGROUND && !move && Interfaces::m_pGlobalVars->curtime >= (g_Vars.globals.m_flBodyPred - g_Vars.globals.m_flAnimFrame) && Interfaces::m_pGlobalVars->curtime < g_Vars.globals.m_flBodyPred) {
		//	// z mode.
		//	if (settings->yaw == 3)
		//		cmd->viewangles.y -= 90.f;
		//}

		if (!(g_Vars.antiaim.desync_on_dt && g_TickbaseController.s_nExtraProcessingTicks > 0) && !Interfaces::m_pClientState->m_nChokedCommands() && g_Vars.antiaim.anti_lastmove && g_Vars.globals.need_break_lastmove && !g_Vars.globals.Fakewalking && !(g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled)) {
			Movement::Get()->StopPlayer();
			*bSendPacket = !(cmd->tick_count % 2 == 0);
			cmd->viewangles.y += g_Vars.antiaim.break_lby_first;
			g_Vars.globals.need_break_lastmove = false;
		}

		static int negative = false;
		static bool breaker = false;
		auto bSwitch = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) < Interfaces::m_pGlobalVars->interval_per_tick;
		auto bSwap = std::fabs(Interfaces::m_pGlobalVars->curtime - g_Vars.globals.m_flBodyPred) > 1.1 - (Interfaces::m_pGlobalVars->interval_per_tick * 5);
		//if (!Interfaces::m_pClientState->m_nChokedCommands()
		//	&& Interfaces::m_pGlobalVars->curtime >= g_Vars.globals.m_flBodyPred
		//	&& LocalPlayer->m_fFlags() & FL_ONGROUND && !move) {
		//	if (g_Vars.globals.Fakewalking) {
		//		*bSendPacket = true;
		//	}
		//	// fake yaw.
		//	switch (settings->yaw) {
		//	case 1: // static
		//		/*g_Vars.misc.mind_trick_bind.enabled ? cmd->viewangles.y += g_Vars.misc.mind_trick_lby :*/ cmd->viewangles.y += g_Vars.antiaim.break_lby;
		//		//if (!breaker) {
		//		if(g_Vars.antiaim.flickup)
		//			cmd->viewangles.x -= g_Vars.antiaim.break_lby;
		//		//	breaker = true;
		//		//}
		//		//else if (breaker)
		//		//	breaker = false;
		//		break;
		//	case 2: // twist
		//		negative ? cmd->viewangles.y += 110.f : cmd->viewangles.y -= 110.f;
		//		negative = !negative;
		//		break;
		//	case 3: // z
		//		cmd->viewangles.y += 90.f;
		//		break;
		//	default:
		//		break;
		//	}

		//	m_flLowerBodyUpdateYaw = LocalPlayer->m_flLowerBodyYawTarget();
		//}

		do_lby(cmd, bSendPacket);

		/*if ( g_Vars.antiaim.imposta ) {
			Interfaces::AntiAimbot::Get( )->ImposterBreaker( bSendPacket, cmd );
		}*/
	}

	void C_AntiAimbot::PrePrediction(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) {
		if (!g_Vars.antiaim.enabled)
			return;

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return;

		Encrypted_t<CVariables::ANTIAIM_STATE> settings(&g_Vars.antiaim_stand);

		//g_Vars.globals.m_bInverted = g_Vars.antiaim.desync_flip_bind.enabled;

		if (!IsEnabled(cmd, settings)) {
			g_Vars.globals.RegularAngles = cmd->viewangles;
			return;
		}
	}

	float C_AntiAimbot::GetAntiAimX(Encrypted_t<CVariables::ANTIAIM_STATE> settings) {
		if (!g_Vars.globals.m_rce_forceup) {
			switch (settings->pitch) {
			case 1: // down
				return 89.f;
			case 2: // up 
				return -89.f;
			case 3: // zero
				return 0.f;
			default:
				return FLT_MAX;
				break;
			}
		}
		else if (g_Vars.globals.m_rce_forceup) {
			return -89.f;
		}
	}



	float C_AntiAimbot::GetAntiAimY(Encrypted_t<CVariables::ANTIAIM_STATE> settings, Encrypted_t<CUserCmd> cmd) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return FLT_MAX;

		bool bUsingManualAA = g_Vars.globals.manual_aa != -1 && g_Vars.antiaim.manual;
		float flRetValue;

		if (g_Vars.antiaim.edge_aa && !bUsingManualAA && local->m_vecVelocity().Length() < 320.f) {
			QAngle ang;
			if (DoEdgeAntiAim(local, ang)) {
				flRetValue = ang.y;
				return flRetValue;
			}
		}

		float flViewAnlge = cmd->viewangles.y;

		auto GetTargetYaw = [local, flViewAnlge, settings]() -> float
		{
			float_t flBestDistance = FLT_MAX;

			C_CSPlayer* pFinalPlayer = nullptr;
			for (int32_t i = 1; i < 65; i++)
			{
				C_CSPlayer* pPlayer = C_CSPlayer::GetPlayerByIndex(i);
				if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->IsDead() || pPlayer->m_iTeamNum() == local->m_iTeamNum() || pPlayer->IsDormant())
					continue;

				if (pPlayer->m_fFlags() & FL_FROZEN)
					continue;

				float_t flDistanceToPlayer = local->m_vecOrigin().Distance(pPlayer->m_vecOrigin());
				if (flDistanceToPlayer > flBestDistance)
					continue;

				if (flDistanceToPlayer > 1250.0f)
					continue;

				flBestDistance = flDistanceToPlayer;
				pFinalPlayer = pPlayer;
			}

			if (!pFinalPlayer) {
				if (settings.Xor()->base_yaw == 0) {
					return flViewAnlge;
				}
				else
					return flViewAnlge + 180.f;
			}

			return Math::CalcAngle(local->GetAbsOrigin() + local->m_vecViewOffset(), pFinalPlayer->GetAbsOrigin()).yaw;
		};

		if (settings->base_yaw == 0) {
			flRetValue = (g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge);
		}
		else
			flRetValue = (g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge + 180.f);

		if (bUsingManualAA) {
			switch (g_Vars.globals.manual_aa) {
			case 0:
				flRetValue = flViewAnlge + 90.f;
				break;
			case 1:
				flRetValue = flViewAnlge + 180.f;
				break;
			case 2:
				flRetValue = flViewAnlge - 90.f;
				break;
			}
		}

		if(settings->yaw == 1) { // backwards - doing this above freestanding to not override it.
			if (!bUsingManualAA) {
				flRetValue = g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge + 180.f;
			}
		}

		float freestandingReturnYaw = std::numeric_limits< float >::max();

		if (g_Vars.antiaim.freestand) {
			bool DoFreestanding = true;
			QAngle ang;

			if (g_Vars.antiaim.freestand_disable_fakewalk && g_Vars.globals.Fakewalking)
				DoFreestanding = false;

			if (g_Vars.antiaim.freestand_disable_air && !g_Vars.globals.m_bGround)
				DoFreestanding = false;

			if (local->m_PlayerAnimState()->m_velocity > 0.1f && g_Vars.globals.m_bGround && !g_Vars.globals.Fakewalking) {
				if (g_Vars.antiaim.freestand_disable_run)
					DoFreestanding = false;
			}

			if (DoFreestanding && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 0) {
				AutoDirection(cmd);
				flRetValue = m_auto + g_Vars.antiaim.add_yaw;
			}
			else if (DoFreestanding && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 1) {
				freestand(cmd.Xor(), cmd->viewangles);
				flRetValue = cmd->viewangles.y;
			}
			//else if (DoFreestanding && DoEdgeAntiAim(local, ang) && !bUsingManualAA && g_Vars.antiaim.freestand_mode == 2) { // run edge aa
			//	flRetValue = Math::AngleNormalize(ang.y);
			//}


			//if (!bUsingManualAA && g_Vars.antiaim.freestand_mode == 0) {
			//	C_AntiAimbot::Directions Direction = HandleDirection(cmd);
			//	switch (Direction) {
			//case Directions::YAW_BACK:
			//	// backwards yaw.
			//	flRetValue = flViewAnlge + 180.f;
			//	break;
			//case Directions::YAW_LEFT:
			//	// left yaw.
			//	flRetValue = flViewAnlge + 90.f;
			//	break;
			//case Directions::YAW_RIGHT:
			//	// right yaw.
			//	flRetValue = flViewAnlge - 90.f;
			//	break;
			//case Directions::YAW_NONE:
			//	// 180.
			//	flRetValue = flViewAnlge + 180.f;
			//	break;
			//	}
			//}
		}

		if (!(g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled)) {
			// lets do our real yaw.'
			switch (settings->yaw) {
			case 2: { // rotate
				flRetValue += std::fmod(Interfaces::m_pGlobalVars->curtime * (settings->rot_speed * 20.f), settings->rot_range);
				break;
			}
			case 3: { // jitter
				auto jitter_range = settings->jitter_range * 0.5f;
				const auto jitter_speed = settings->jitter_speed;

				static auto last_set_tick = 0;
				static auto flip = false;

				static auto add = 0.f;

				if (last_set_tick + jitter_speed < local->m_nTickBase() || last_set_tick > local->m_nTickBase())
				{
					last_set_tick = local->m_nTickBase();

					//if (get_antiaim(type)->jitter_random->get<int>())
					//{
					//	jitter_range = random_float(-jitter_range, jitter_range);
					//	flip = true;
					//}

					add = flip ? jitter_range : -jitter_range;

					flip = !flip;
				}

				flRetValue += add;

				break;
			}
			case 4: { // 180z
				if (!bUsingManualAA) {
					flRetValue = (g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge + 180.f / 2.f);
					flRetValue += std::fmod(Interfaces::m_pGlobalVars->curtime * (3.5 * 20.f), 180.f);
				}

				break;
			}
			case 5: { // lowerbody
				flRetValue = flViewAnlge + local->m_flLowerBodyYawTarget();
				break;
			}
			case 6: { // custom
				if (!bUsingManualAA) {
					flRetValue = g_Vars.antiaim.at_targets ? GetTargetYaw() : flViewAnlge + settings->custom_yaw;
				}
				break;
			}
			default:
				break;
			}
		}

		return flRetValue;
	}

	void C_AntiAimbot::Distort(Encrypted_t<CUserCmd> cmd) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return;

		if (!g_Vars.antiaim.distort)
			return;

		bool bDoDistort = true;
		if (g_Vars.antiaim.distort_disable_fakewalk && g_Vars.globals.Fakewalking)
			bDoDistort = false;

		if (g_Vars.antiaim.distort_disable_air && !g_Vars.globals.m_bGround)
			bDoDistort = false;

		static float flLastMoveTime = FLT_MAX;
		static float flLastMoveYaw = FLT_MAX;
		static bool bGenerate = true;
		static float flGenerated = 0.f;

		if (local->m_PlayerAnimState()->m_velocity > 0.1f && g_Vars.globals.m_bGround && !g_Vars.globals.Fakewalking) {
			flLastMoveTime = Interfaces::m_pGlobalVars->realtime;
			flLastMoveYaw = local->m_flLowerBodyYawTarget();

			if (g_Vars.antiaim.distort_disable_run)
				bDoDistort = false;
		}

		if (g_Vars.globals.manual_aa != -1 && !g_Vars.antiaim.distort_manual_aa)
			bDoDistort = false;

		if (flLastMoveTime == FLT_MAX)
			return;

		if (flLastMoveYaw == FLT_MAX)
			return;

		if (!bDoDistort) {
			bGenerate = true;
		}

		if (bDoDistort) {
			// don't distort for longer than this
			if (fabs(Interfaces::m_pGlobalVars->realtime - flLastMoveTime) > g_Vars.antiaim.distort_max_time && g_Vars.antiaim.distort_max_time > 0.f) {
				return;
			}

			if (g_Vars.antiaim.distort_twist) {
				float flDistortion = std::sin((Interfaces::m_pGlobalVars->realtime * g_Vars.antiaim.distort_speed) * 0.5f + 0.5f);

				cmd->viewangles.y += g_Vars.antiaim.distort_range * flDistortion;
				return;
			}

			if (bGenerate) {
				float flNormalised = std::remainderf(g_Vars.antiaim.distort_range, 360.f);

				flGenerated = RandomFloat(-flNormalised, flNormalised);
				bGenerate = false;
			}

			float flDistortion = std::sin((Interfaces::m_pGlobalVars->realtime * g_Vars.antiaim.distort_speed) * 0.5f + 0.5f);
			float flDelta = fabs(flLastMoveYaw - local->m_flLowerBodyYawTarget());
			cmd->viewangles.y += flDelta + flGenerated * flDistortion;
		}
	}

	// CX
	void C_AntiAimbot::ImposterBreaker(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) {
		auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal)
			return;

		bool bCrouching = pLocal->m_PlayerAnimState()->m_fDuckAmount > 0.f;
		float flVelocity = (bCrouching ? 3.25f : 1.01f);
		static int iUpdates = 0;

		if (!(g_Vars.globals.m_pCmd->buttons & IN_FORWARD) && !(g_Vars.globals.m_pCmd->buttons & IN_BACK) && !(g_Vars.globals.m_pCmd->buttons & IN_MOVELEFT) && !(g_Vars.globals.m_pCmd->buttons & IN_MOVERIGHT) && !(g_Vars.globals.m_pCmd->buttons & IN_JUMP))
		{
			if (Interfaces::m_pClientState->m_nChokedCommands() == 2)
			{
				cmd->forwardmove = iUpdates % 2 ? -450 : 450;
				++iUpdates;
			}
		}
	}


	struct AutoTarget_t { float fov; C_CSPlayer* player; };
	C_AntiAimbot::Directions C_AntiAimbot::HandleDirection(Encrypted_t<CUserCmd> cmd) {
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return Directions::YAW_NONE;

		// best target.
		AutoTarget_t target{ 180.f + 1.f, nullptr };

		// iterate players, for closest distance.
		for (int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);
			if (!player || player->IsDormant() || player == pLocal || player->IsDead() || player->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			auto AngleDistance = [&](QAngle& angles, const Vector& start, const Vector& end) -> float {
				auto direction = end - start;
				auto aimAngles = direction.ToEulerAngles();
				auto delta = aimAngles - angles;
				delta.Normalize();

				return sqrtf(delta.x * delta.x + delta.y * delta.y);
			};

			float fov = AngleDistance(cmd->viewangles, g_Vars.globals.m_vecFixedEyePosition, player->m_vecOrigin());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		// get best player.
		if (const auto player = target.player)
		{
			Vector& bestOrigin = player->m_vecOrigin();

			// calculate direction from bestOrigin to our origin
			const auto yaw = Math::CalcAngle(bestOrigin, pLocal->m_vecOrigin());

			Vector forward, right, up;
			Math::AngleVectors(yaw, forward, right, up);

			Vector vecStart = pLocal->GetEyePosition();
			Vector vecEnd = vecStart + forward * 100.0f;

			Ray_t rightRay(vecStart + right * 35.0f, vecEnd + right * 35.0f), leftRay(vecStart - right * 35.0f, vecEnd - right * 35.0f);

			// setup trace filter
			CTraceFilter filter{ };
			filter.pSkip = pLocal;

			CGameTrace tr{ };

			m_pEngineTrace->TraceRay(rightRay, MASK_SOLID, &filter, &tr);
			float rightLength = (tr.endpos - tr.startpos).Length();

			m_pEngineTrace->TraceRay(leftRay, MASK_SOLID, &filter, &tr);
			float leftLength = (tr.endpos - tr.startpos).Length();

			static auto leftTicks = 0;
			static auto rightTicks = 0;
			static auto backTicks = 0;

			if (rightLength - leftLength > 20.0f)
				leftTicks++;
			else
				leftTicks = 0;

			if (leftLength - rightLength > 20.0f)
				rightTicks++;
			else
				rightTicks = 0;

			if (fabs(rightLength - leftLength) <= 20.0f)
				backTicks++;
			else
				backTicks = 0;

			Directions direction = Directions::YAW_NONE;

			if (rightTicks > 10) {
				direction = Directions::YAW_RIGHT;
			}
			else {
				if (leftTicks > 10) {
					direction = Directions::YAW_LEFT;
				}
				else {
					if (backTicks > 10)
						direction = Directions::YAW_BACK;
				}
			}

			return direction;
		}

		return Directions::YAW_NONE;
	}

	bool C_AntiAimbot::DoEdgeAntiAim(C_CSPlayer* player, QAngle& out) {
		CGameTrace trace;

		if (player->m_MoveType() == MOVETYPE_LADDER)
			return false;

		// skip this player in our traces.
		CTraceFilter filter{ };
		filter.pSkip = player;

		// get player bounds.
		Vector mins = player->OBBMins();
		Vector maxs = player->OBBMaxs();

		// make player bounds bigger.
		mins.x -= 20.f;
		mins.y -= 20.f;
		maxs.x += 20.f;
		maxs.y += 20.f;

		// get player origin.
		Vector start = player->GetAbsOrigin();

		// offset the view.
		start.z += 56.f;

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, start, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		if (!trace.startsolid)
			return false;

		float  smallest = 1.f;
		Vector plane;

		// trace around us in a circle, in 20 steps (anti-degree conversion).
		// find the closest object.
		for (float step{ }; step <= (M_PI * 2.f); step += (M_PI / 10.f)) {
			// extend endpoint x units.
			Vector end = start;

			// set end point based on range and step.
			end.x += std::cos(step) * 32.f;
			end.y += std::sin(step) * 32.f;

			Interfaces::m_pEngineTrace->TraceRay(Ray_t(start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f }), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

			// we found an object closer, then the previouly found object.
			if (trace.fraction < smallest) {
				// save the normal of the object.
				plane = trace.plane.normal;
				smallest = trace.fraction;
			}
		}

		// no valid object was found.
		if (smallest == 1.f || plane.z >= 0.1f)
			return false;

		// invert the normal of this object
		// this will give us the direction/angle to this object.
		Vector inv = -plane;
		Vector dir = inv;
		dir.Normalize();

		// extend point into object by 24 units.
		Vector point = start;
		point.x += (dir.x * 24.f);
		point.y += (dir.y * 24.f);

		// check if we can stick our head into the wall.
		if (Interfaces::m_pEngineTrace->GetPointContents(point, CONTENTS_SOLID) & CONTENTS_SOLID) {
			// trace from 72 units till 56 units to see if we are standing behind something.
			Interfaces::m_pEngineTrace->TraceRay(Ray_t(point + Vector{ 0.f, 0.f, 16.f }, point), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);

			// we didnt start in a solid, so we started in air.
			// and we are not in the ground.
			if (trace.fraction < 1.f && !trace.startsolid && trace.plane.normal.z > 0.7f) {
				// mean we are standing behind a solid object.
				// set our angle to the inversed normal of this object.
				out.y = RAD2DEG(std::atan2(inv.y, inv.x));
				return true;
			}
		}

		// if we arrived here that mean we could not stick our head into the wall.
		// we can still see if we can stick our head behind/asides the wall.

		// adjust bounds for traces.
		mins = { (dir.x * -3.f) - 1.f, (dir.y * -3.f) - 1.f, -1.f };
		maxs = { (dir.x * 3.f) + 1.f, (dir.y * 3.f) + 1.f, 1.f };

		// move this point 48 units to the left 
		// relative to our wall/base point.
		Vector left = start;
		left.x = point.x - (inv.y * 48.f);
		left.y = point.y - (inv.x * -48.f);

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(left, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float l = trace.startsolid ? 0.f : trace.fraction;

		// move this point 48 units to the right 
		// relative to our wall/base point.
		Vector right = start;
		right.x = point.x + (inv.y * 48.f);
		right.y = point.y + (inv.x * -48.f);

		Interfaces::m_pEngineTrace->TraceRay(Ray_t(right, point, mins, maxs), CONTENTS_SOLID, (ITraceFilter*)&filter, &trace);
		float r = trace.startsolid ? 0.f : trace.fraction;

		// both are solid, no edge.
		if (l == 0.f && r == 0.f)
			return false;

		// set out to inversed normal.
		out.y = RAD2DEG(std::atan2(inv.y, inv.x));

		// left started solid.
		// set angle to the left.
		if (l == 0.f) {
			out.y += 90.f;
			return true;
		}

		// right started solid.
		// set angle to the right.
		if (r == 0.f) {
			out.y -= 90.f;
			return true;
		}

		return false;
	}

	void C_AntiAimbot::AutoDirection(Encrypted_t<CUserCmd> cmd) {
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return;

		// constants.
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 32.f };

		// best target.
		struct AutoTarget_t { float fov; C_CSPlayer* player; };
		AutoTarget_t target{ 180.f + 1.f, nullptr };

		// iterate players.
		for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
			auto player = C_CSPlayer::GetPlayerByIndex(i);

			//skip invalid or unwanted
			if (!player || player->IsDormant() || player == pLocal || player->IsDead() || player->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// validate player.
			auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			// get best target based on fov.
			float fov = Math::GetFov(g_Vars.globals.CurrentLocalViewAngles, pLocal->GetEyePosition(), player->WorldSpaceCenter());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = player;
			}
		}

		if (!target.player) {
			// we have a timeout.
			if (m_auto_last > 0.f && m_auto_time > 0.f && Interfaces::m_pGlobalVars->curtime < (m_auto_last + m_auto_time))
				return;

			// set angle to backwards.
			m_auto = Math::NormalizedAngle(m_view - 180.f);
			m_auto_dist = -1.f;
			return;
		}

		/*
		* data struct
		* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
		*/

		// construct vector of angles to test.
		std::vector< AdaptiveAngle > angles{ };
		angles.emplace_back(m_view - 180.f);
		angles.emplace_back(m_view + 90.f);
		angles.emplace_back(m_view - 90.f);

		// start the trace at the enemy shoot pos.
		Vector start = target.player->GetShootPosition();

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for (auto it = angles.begin(); it != angles.end(); ++it) {

			// compute the 'rough' estimation of where our head will be.
			Vector end{ pLocal->GetShootPosition().x + std::cos(Math::deg_to_rad(it->m_yaw)) * RANGE,
				pLocal->GetShootPosition().y + std::sin(Math::deg_to_rad(it->m_yaw)) * RANGE,
				pLocal->GetShootPosition().z };

			// draw a line for debugging purposes.
			//Interfaces::m_pDebugOverlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

			// compute the direction.
			Vector dir = end - start;
			float len = dir.Normalize();

			// should never happen.
			if (len <= 0.f)
				continue;

			// step thru the total distance, 4 units per step.
			for (float i{ 0.f }; i < len; i += STEP) {
				// get the current step position.
				Vector point = start + (dir * i);

				// get the contents at this point.
				int contents = Interfaces::m_pEngineTrace->GetPointContents(point, MASK_SHOT_HULL);

				// contains nothing that can stop a bullet.
				if (!(contents & MASK_SHOT_HULL))
					continue;

				float mult = 1.f;

				// over 50% of the total length, prioritize this shit.
				if (i > (len * 0.5f))
					mult = 1.25f;

				// over 75% of the total length, prioritize this shit.
				if (i > (len * 0.75f))
					mult = 1.25f;

				// over 90% of the total length, prioritize this shit.
				if (i > (len * 0.9f))
					mult = 2.f;

				// append 'penetrated distance'.
				it->m_dist += (STEP * mult);

				// mark that we found anything.
				valid = true;
			}
		}

		if (!valid) {
			// set angle to backwards.
			m_auto = Math::NormalizedAngle(m_view - 180.f);
			m_auto_dist = -1.f;
			EdgeFlick = false;
			return;
		}

		// put the most distance at the front of the container.
		std::sort(angles.begin(), angles.end(),
			[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
				return a.m_dist > b.m_dist;
			});

		// the best angle should be at the front now.
		AdaptiveAngle* best = &angles.front();

		// check if we are not doing a useless change.
		if (best->m_dist != m_auto_dist) {
			// set yaw to the best result.
			m_auto = Math::NormalizedAngle(best->m_yaw);
			m_auto_dist = best->m_dist;
			m_auto_last = Interfaces::m_pGlobalVars->curtime;
			EdgeFlick = true;
		}
	}

	void C_AntiAimbot::freestanding() {
		const auto pLocal = C_CSPlayer::GetLocalPlayer();
		if (!pLocal) return;

		std::vector< angle_data > points;

		auto local_position = pLocal->GetEyePosition();
		std::vector< float > scanned = {};

		AutoTarget_t target{ 180.f + 1.f, nullptr };

		for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
			auto enemy = (C_CSPlayer*)(Interfaces::m_pEntList->GetClientEntity(i));

			if (!enemy || enemy->IsDormant() || enemy == pLocal || !enemy->IsAlive() || enemy->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// validate player.
			auto lag_data = Engine::LagCompensation::Get()->GetLagData(enemy->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			QAngle view;

			// get best target based on fov.
			float fov = Math::GetFov(g_Vars.globals.CurrentLocalViewAngles, local_position, enemy->WorldSpaceCenter());

			if (fov < target.fov) {
				target.fov = fov;
				target.player = enemy;
			}

			view = local_position.AngleTo(target.player->GetEyePosition());

			std::vector< angle_data > angs;

			for (auto y = 1; y < 4; y++) {
				auto ang = quick_normalize((y * 90) + view.y, -180.f, 180.f);
				auto found = false; // check if we already have a similar angle

				for (auto i2 : scanned)
					if (abs(quick_normalize(i2 - ang, -180.f, 180.f)) < 20.f)
						found = true;

				if (found)
					continue;

				points.emplace_back(ang, -1.f);
				scanned.push_back(ang);
			}
			//points.push_back(base_angle_data(view.y, angs)); // base yaws and angle data (base yaw needed for lby breaking etc)
		}

		if (!target.player) {
			// we have a timeout.
			if (m_auto_last > 0.f && m_auto_time > 0.f && Interfaces::m_pGlobalVars->curtime < (m_auto_last + m_auto_time))
				return;

			// set angle to backwards.
			m_auto = Math::NormalizedAngle(m_view - 180.f);
			m_auto_dist = -1.f;
			return;
		}

		for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
			auto enemy = (C_CSPlayer*)(Interfaces::m_pEntList->GetClientEntity(i));

			if (!enemy || enemy->IsDormant() || enemy == pLocal || !enemy->IsAlive() || enemy->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// validate player.
			auto lag_data = Engine::LagCompensation::Get()->GetLagData(enemy->m_entIndex);
			if (!lag_data.IsValid())
				continue;

			auto found = false;
			auto points_copy = points; // copy data so that we compare it to the original later to find the lowest thickness
			auto enemy_shootpos = target.player->GetShootPosition();

			for (auto& z : points_copy) // now we get the thickness for all of the data
			{
				const QAngle tmp(10, z.angle, 0.0f);
				Vector head;
				Math::AngleVectors(tmp, head);
				head *= ((16.0f + 3.0f) + ((16.0f + 3.0f) * sin(DEG2RAD(10.0f)))) + 7.0f;
				head += local_position;
				float distance = -1;
				C_WeaponCSBaseGun* enemy_weapon = (C_WeaponCSBaseGun*)target.player->m_hActiveWeapon().Get();
				if (enemy_weapon) {
					auto weapon_data = enemy_weapon->GetCSWeaponData();
					if (weapon_data.IsValid())
						distance = weapon_data->m_flWeaponRange;
				}
				float local_thickness = get_thickness(head, enemy_shootpos, distance);
				z.thickness = local_thickness;

				if (local_thickness > 0) // if theres a thickness of 0 dont use this data
				{
					found = true;
				}
			}

			if (!found) // dont use, completely visible to this player or data is invalid
				continue;

			for (unsigned int z = 0; points_copy.size() > z; z++)
				if (points_copy[z].thickness < points[z].thickness || points[z].thickness == -1)
					// find the lowest thickness so that we can hide our head best for all entities
					points[z].thickness = points_copy[z].thickness;
		}
		float best = 0;
		for (auto& i : points)
			if ((i.thickness > best || i.thickness == -1) && i.thickness != 0)
				// find the best hiding spot (highest thickness)
			{
				best = i.thickness;
				m_auto = Math::NormalizedAngle(i.angle);
			}
	}


}