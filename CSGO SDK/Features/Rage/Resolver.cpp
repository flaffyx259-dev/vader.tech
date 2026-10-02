#include "Resolver.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Visuals/CChams.hpp"
#include "../Rage/AntiAim.hpp"
#include "../Rage/Ragebot.hpp"
#include "../Rage/Autowall.h"
#include "../Visuals/EventLogger.hpp"
#include "Ragebot.hpp"
#include "TickbaseShift.hpp"
#include "angle.hpp"

static float NextLBYUpdate[65];
static float Add[65];
bool is_flicking;

C_CSPlayer* get_entity(const int index) { return reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(index)); }

namespace Engine {
	CResolver g_Resolver;
	CResolverData g_ResolverData[65];

	void CResolver::collect_wall_detect(const ClientFrameStage_t stage)
	{
		if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || !local->IsAlive())
			return;

		last_eye_positions.insert(last_eye_positions.begin(), local->m_vecOrigin() + local->m_vecViewOffset());
		if (last_eye_positions.size() > 128)
			last_eye_positions.pop_back();

		auto nci = Interfaces::m_pEngine->GetNetChannelInfo();
		if (!nci)
			return;

		const int latency_ticks = TIME_TO_TICKS(nci->GetLatency(FLOW_OUTGOING));
		auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

		for (auto i = 1; i < Interfaces::m_pGlobalVars->maxClients; i++)
		{
			//auto& log = player_log::get().get_log(i);
			auto player = get_entity(i);

			if (!player || player == local)
			{
				continue;
			}

			if (player->IsTeammate(local))
			{
				continue;
			}

			if (!player->IsAlive())
			{
				continue;
			}

			if (player->IsDormant())
			{
				continue;
			}

			if (player->m_vecVelocity().Length2D() > 0.1f)
			{
				continue;
			}

			auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
			if (!anim_data)
				return;

			if (anim_data->m_AnimationRecord.empty())
				return;

			if (!anim_data->m_AnimationRecord.empty() && player->m_flSimulationTime() - anim_data->m_AnimationRecord[0].m_flSimulationTime == 0)
				continue;

			QAngle at_target_angle = Math::CalcAngle(player->m_vecOrigin(), last_eye);



			Vector direction_1, direction_2, direction_3;
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
			Math::AngleVectors(QAngle(0.f, Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

			const float height = 64;

			const auto left_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_1 * 16.f);
			const auto right_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_2 * 16.f);
			const auto back_eye_pos = player->m_vecOrigin() + Vector(0, 0, height) + (direction_3 * 16.f);

			anti_freestanding_record.left_damage = Autowall::ScaleDamage(player, left_damage[i], 1.f, Hitgroup_Head);
			anti_freestanding_record.right_damage = Autowall::ScaleDamage(player, right_damage[i], 1.f, Hitgroup_Head);
			anti_freestanding_record.back_damage = Autowall::ScaleDamage(player, back_damage[i], 1.f, Hitgroup_Head);

			Ray_t ray;
			CGameTrace trace;
			CTraceFilterWorldOnly filter;

			Ray_t first_ray(left_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.left_fraction = trace.fraction;

			Ray_t second_ray(right_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.right_fraction = trace.fraction;

			Ray_t third_ray(back_eye_pos, latency_based_eye_pos);
			Interfaces::m_pEngineTrace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.back_fraction = trace.fraction;
		}
	}

	bool CResolver::IsBreakingLBY120(C_CSPlayer* pEntity)
	{
		for (int w = 0; w < 13; w++)
		{
			C_AnimationLayer prevlayer;
			C_AnimationLayer currentLayer = pEntity->GetAnimLayer(w);
			const int activity = pEntity->GetSequenceActivity(currentLayer.m_nSequence);
			float flcycle = currentLayer.m_flCycle, flprevcycle = currentLayer.m_flPrevCycle, flweight = currentLayer.m_flWeight, flweightdatarate = currentLayer.m_flWeightDeltaRate;
			uint32_t norder = currentLayer.m_nOrder;
			if (activity == 980 || activity == 979 && flweight >= .99 && currentLayer.m_flPrevCycle != currentLayer.m_flCycle)
			{
				float flanimTime = currentLayer.m_flCycle, flsimtime = pEntity->m_flSimulationTime();

				return true;
			}
			prevlayer = currentLayer;
			return false;
		}
		return false;
	}

	bool CResolver::IsResolvableByLBY(C_CSPlayer* pEntity)
	{
		for (int w = 0; w < 13; w++)
		{
			C_AnimationLayer prevlayer;
			C_AnimationLayer currentLayer = pEntity->GetAnimLayer(w);
			const int activity = pEntity->GetSequenceActivity(currentLayer.m_nSequence);
			float flcycle = currentLayer.m_flCycle, flprevcycle = currentLayer.m_flPrevCycle, flweight = currentLayer.m_flWeight, flweightdatarate = currentLayer.m_flWeightDeltaRate;
			uint32_t norder = currentLayer.m_nOrder;
			if (activity == 979 && currentLayer.m_flWeight == 0.f && currentLayer.m_flPrevCycle != currentLayer.m_flCycle)
			{
				return true;
			}
			prevlayer = currentLayer;
		}
		return false;
	}

	bool CResolver::wall_detect(C_CSPlayer* player, C_AnimationRecord* record, float& angle) const
	{

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local->IsAlive())
			return false;

		QAngle at_target_angle = Math::CalcAngle(record->m_vecOrigin, last_eye);

		auto set = false;

		const auto left = left_damage[player->EntIndex()];
		const auto right = right_damage[player->EntIndex()];
		const auto back = back_damage[player->EntIndex()];

		auto max_dmg = std::max(left, std::max(right, back)) - 1.f;
		if (left < max_dmg)
		{
			max_dmg = left;
			angle = Math::normalize_float(at_target_angle.y + 90.f);
			set = true;
		}
		if (right < max_dmg)
		{
			max_dmg = right;
			angle = Math::normalize_float(at_target_angle.y - 90.f);
			set = true;
		}
		if (back < max_dmg || !set)
		{
			max_dmg = back;
			angle = Math::normalize_float(at_target_angle.y + 180.f);
		}

		return true;
	}

	void CResolver::AntiFreestand(C_AnimationRecord* record, C_CSPlayer* entity) {

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(entity->m_entIndex);
		if (!pLagData.IsValid())
			return;

		if (entity == local)
			return;

		C_AnimationRecord* move = &pLagData->m_walk_record;

		// constants
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 32.f };

		// best target.
		Vector enemypos = entity->GetShootPosition();
		float away = GetAwayAngle(record);

		// construct vector of angles to test.
		std::vector< AdaptiveAngle > angles{ };
		angles.emplace_back(away - 180.f);
		angles.emplace_back(away + 90.f);
		angles.emplace_back(away - 90.f);

		// start the trace at the your shoot pos.
		Vector start = local->GetShootPosition();

		// see if we got any valid result.
		// if this is false the path was not obstructed with anything.
		bool valid{ false };

		// iterate vector of angles.
		for (auto it = angles.begin(); it != angles.end(); ++it) {

			// compute the 'rough' estimation of where our head will be.
			Vector end{ enemypos.x + std::cos(Math::deg_to_rad(it->m_yaw)) * RANGE,
				enemypos.y + std::sin(Math::deg_to_rad(it->m_yaw)) * RANGE,
				enemypos.z };

			// draw a line for debugging purposes.
			// g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

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

				// over 90% of the total length, prioritize this shit.
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
			record->m_angEyeAngles.y = away + 180.f;
			record->m_iResolverText = XorStr("BACKWARDS");
			return;
		}

		// put the most distance at the front of the container.
		std::sort(angles.begin(), angles.end(),
			[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
				return a.m_dist > b.m_dist;
			});

		// the best angle should be at the front now.
		AdaptiveAngle* best = &angles.front();

		//wall_detect(entity, record, record->m_angEyeAngles.y)

		// fix ppl breaking last move/freestand.
		if (!record->m_bUnsafeVelocityTransition && !record->m_bIsFakeFlicking && best->m_yaw - pLagData->m_body > 90.f || best->m_yaw - pLagData->m_body < -90.f) {
			record->m_angEyeAngles.y = pLagData->m_body;
			record->m_iResolverText = XorStr("WHATTTTT");
		}

		record->m_iResolverText = XorStr("FREESTAND");
		record->m_angEyeAngles.y = best->m_yaw;
	}

	void CResolver::MatchShot(C_CSPlayer* data, C_AnimationRecord* record) {
		// do not attempt to do this in nospread mode.
		//if (g_menu.main.config.mode.get() == 1)
		//	return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(data->m_entIndex);
		if (!anim_data)
			return;

		if (anim_data->m_AnimationRecord.empty())
			return;

		auto weapon = (C_WeaponCSBaseGun*)(data->m_hActiveWeapon().Get());


		const auto simulation_ticks = TIME_TO_TICKS(data->m_flSimulationTime());
		auto old_simulation_ticks = TIME_TO_TICKS(data->m_flOldSimulationTime());
		int m_shot;
		static float m_last_nonshot_pitch;

		if (weapon && !weapon->IsKnife()) {
			if (record->m_iChokeTicks > 0) {
				const auto& shot_tick = TIME_TO_TICKS(weapon->m_fLastShotTime());

				if (record->m_bIsShoting) {
					m_shot = 1;
					//ILoggerEvent::Get()->PushEvent("shot = 1", FloatColor(1.f, 1.f, 1.f), true, "");
				}
				else {
					if (abs(simulation_ticks - shot_tick) > record->m_iChokeTicks + 2) {
						if (m_shot != 1) { // lets not save angle while hes shooting
							m_last_nonshot_pitch = data->m_angEyeAngles().x;
						}
					}
				}
			}

			if (m_shot == 1) {
				if (record->m_iChokeTicks > 2) {
					record->m_angEyeAngles.x = m_last_nonshot_pitch;
				}
			}
		}
	}

	static float NextLBYUpdate[65];
	static float Add[65];

	void CResolver::SetMode(C_CSPlayer* player, C_AnimationRecord* record) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data)
			return;

		C_AnimationRecord* move = &pLagData->m_walk_record;

		const auto simtime = record->m_flSimulationTime;

		float speed = record->m_vecVelocity.Length2D();

		// predict LBY flicks.
		if (!player->IsDormant() && !record->dormant() && (player->m_fFlags() & FL_ONGROUND)) {
			auto nextPredictedSimtimeAccurate = player->m_flSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
			auto nextPredictedSimtime = player->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;

			// lby wont update on this tick but after.
			if (nextPredictedSimtimeAccurate >= NextLBYUpdate[player->EntIndex()])
			{
				is_flicking = true;
				Add[player->EntIndex()] = 1.1f;
				NextLBYUpdate[player->EntIndex()] = nextPredictedSimtimeAccurate + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
				Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
			}
			else
				is_flicking = false;

			if (anim_data->m_AnimationRecord.size() >= 2) {
				C_AnimationRecord* previous = &anim_data->m_AnimationRecord.at(1);

				if (previous && !previous->m_bIsInvalid && !previous->dormant()) {

					// LBY updated via PROXY.
					if (record->m_flLowerBodyYawTarget != previous->m_flLowerBodyYawTarget && fabs(Math::normalize_float(pLagData->m_body - pLagData->m_old_body)) > 5.f) {
						is_flicking = true;
						Add[player->EntIndex()] = 1.1f;
						NextLBYUpdate[player->EntIndex()] = nextPredictedSimtimeAccurate + Add[player->EntIndex()];
						record->m_body_update = NextLBYUpdate[player->EntIndex()];
						Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
					}
				}
			}

			if (player->m_vecVelocity().Length2D() > 0.1f && !record->m_bFakeWalking) {
				Add[player->EntIndex()] = 0.22f;
				NextLBYUpdate[player->EntIndex()] = nextPredictedSimtimeAccurate + Add[player->EntIndex()];
				record->m_body_update = NextLBYUpdate[player->EntIndex()];
				Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = NextLBYUpdate[player->EntIndex()];
			}

		}
		else {
			is_flicking = false;
			record->m_body_update = 0.f;
			NextLBYUpdate[player->EntIndex()] = 0.f;
			Engine::g_ResolverData[player->EntIndex()].m_flNextBodyUpdate = 0.f;
		}

		C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
		const int activity = player->GetSequenceActivity(curr->m_nSequence);
		float delta = record->m_anim_time - move->m_anim_time;

		if (record->m_fFlags & FL_ONGROUND && (record->m_vecVelocity.Length2D() < 0.1f || record->m_bFakeWalking)) {

			if (is_flicking && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition && !record->m_bFakeWalking && pLagData->m_iMissedShotsLBY < 1) {
				record->m_iResolverMode = FLICK;
			}
			else if (record->m_moved && !record->m_iDistorting[player->EntIndex()] && pLagData->m_last_move < 1 && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition) {
				record->m_iResolverMode = LASTMOVE;
			}

			else if (simtime - pLagData->m_flLastLowerBodyYawTargetUpdateTime > 1.35f && record->m_vecLastNonDormantOrig == record->m_vecOrigin && pLagData->m_iMissedShotsFreestand < 1 && pLagData->m_delta_index < 1)
			{
				if (simtime - pLagData->m_flLastLowerBodyYawTargetUpdateTime > 1.65f && pLagData->m_iMissedShotsFreestand < 1)
				{
					record->m_iResolverMode = ANTIFREESTAND;
				}
				else
				{
					record->m_iResolverMode = LBYDELTA;
					pLagData->m_flSavedLbyDelta = pLagData->m_flLowerBodyYawTarget - pLagData->m_flOldLowerBodyYawTarget;
				}
			}

			else if (record->m_moved && record->m_iDistorting[player->EntIndex()] && pLagData->m_iMissedShotsDistort < 1 && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition && (activity == 979 && curr->m_flWeight == 0 && delta > .22f)) {
				record->m_iResolverMode = DISTORTINGLMOVE;
			}
			else if (ShouldUseFreestand(record, player) && pLagData->m_iMissedShotsFreestand < 1) {
				record->m_iResolverMode = ANTIFREESTAND;
			}

			else if (record->m_bIsFakeFlicking || record->m_bUnsafeVelocityTransition && pLagData->m_iMissedShotsFreestand < 2) {
				record->m_iResolverMode = ANTIFREESTAND;
			}
			else {
				record->m_iResolverMode = STAND;
			}
		}

		// if not on ground.
		else if (!(player->m_fFlags() & FL_ONGROUND) && record->m_vecVelocity.Length2D() > 60.f)
			record->m_iResolverMode = AIR;

		else if (!(player->m_fFlags() & FL_ONGROUND) && record->m_vecVelocity.Length2D() <= 60.f)
			record->m_iResolverMode = AIRSTAND;

		// if on ground, moving, and not fakewalking.
		else 
			record->m_iResolverMode = MOVING;
	}

	bool HasStaticRealAngle(const std::deque<C_AnimationRecord>& l, float tolerance) {
		auto minmax = std::minmax_element(std::begin(l), std::end(l), [](const C_AnimationRecord& t1, const C_AnimationRecord& t2) { return t1.m_flLowerBodyYawTarget < t2.m_flLowerBodyYawTarget; });
		return (fabs(minmax.first->m_flLowerBodyYawTarget - minmax.second->m_flLowerBodyYawTarget) <= tolerance);
	}

	const inline float GetDelta(float a, float b) {
		return abs(Math::normalize_float(a - b));
	}

	const inline bool IsDifferent(float a, float b, float tolerance = 10.f) {
		return (GetDelta(a, b) > tolerance);
	}

	const inline float LBYDelta(const C_AnimationRecord& v) {
		return v.m_angEyeAngles.y - v.m_flLowerBodyYawTarget;
	}

	int GetDifferentDeltas(const std::deque<C_AnimationRecord>& l, float tolerance) {
		std::vector<float> vec;
		for (auto var : l) {
			float curdelta = LBYDelta(var);
			bool add = true;
			for (auto fl : vec) {
				if (!IsDifferent(curdelta, fl, tolerance))
					add = false;
			}
			if (add)
				vec.push_back(curdelta);
		}
		return vec.size();
	}

	int GetDifferentLBYs(const std::deque<C_AnimationRecord>& l, float tolerance) {
		std::vector<float> vec;
		for (auto var : l)
		{
			float curyaw = var.m_flLowerBodyYawTarget;
			bool add = true;
			for (auto fl : vec)
			{
				if (!IsDifferent(curyaw, fl, tolerance))
					add = false;
			}
			if (add)
				vec.push_back(curyaw);
		}
		return vec.size();
	}

	bool DeltaKeepsChanging(const std::deque<C_AnimationRecord>& cur, float tolerance) {
		return (GetDifferentDeltas(cur, tolerance) > (int)cur.size() / 2);
	}

	bool LBYKeepsChanging(const std::deque<C_AnimationRecord>& cur, float tolerance) {
		return (GetDifferentLBYs(cur, tolerance) > (int)cur.size() / 2);
	}

	bool HasStaticYawDifference(const std::deque<C_AnimationRecord>& l, float tolerance) {
		for (auto i = l.begin(); i < l.end() - 1;)
		{
			if (GetDelta(LBYDelta(*i), LBYDelta(*++i)) > tolerance)
				return false;
		}
		return true;
	}

	void CResolver::ResolveAngles(C_CSPlayer* player, C_AnimationRecord* record) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data) {
			record->m_moved = false;
			return;
		}

		C_AnimationRecord* move = &pLagData->m_walk_record;

		static int m_iFakeFlickCheck = 0;
		static int m_iFakeFlickResetCheck = 0;
		static float m_flLastResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
		static float m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime;

		Encrypted_t<C_AnimationRecord> previous_record = nullptr;

		if (anim_data->m_AnimationRecord.size() > 0) {
			previous_record = &anim_data->m_AnimationRecord.front();
		}

		if (record->m_vecVelocity.Length() < 20.f
			&& record->m_serverAnimOverlays[6].m_flWeight != 1.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != 0.0f
			&& record->m_serverAnimOverlays[6].m_flWeight != previous_record->m_serverAnimOverlays[6].m_flWeight
			&& (record->m_fFlags & FL_ONGROUND)) {
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
			record->m_bUnsafeVelocityTransition = true;
			pLagData->m_bUnsafeVelocityTransition = true;
		}

		m_flLastResetTime1 = Interfaces::m_pGlobalVars->curtime;

		if (m_flLastResetTime1 >= m_flMaxResetTime1 && record->m_bUnsafeVelocityTransition && record->m_vecVelocity.Length2D() < 30.f) {
			m_iFakeFlickCheck++;
			m_iFakeFlickResetCheck = 0;
		}
		else if (m_flLastResetTime1 >= m_flMaxResetTime1 && m_iFakeFlickCheck >= 0) {
			m_iFakeFlickResetCheck++;
		}

		if (m_iFakeFlickCheck >= 10) {
			record->m_bIsFakeFlicking = true;
		}

		if (record->m_vecVelocity.Length2D() >= 30.f) {
			record->m_bIsFakeFlicking = false;
			m_iFakeFlickCheck = 0;
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
		}
		else if (m_iFakeFlickResetCheck >= 50) {
			record->m_bIsFakeFlicking = false;
			m_iFakeFlickResetCheck = 0;
			m_iFakeFlickCheck = 0;
			m_flMaxResetTime1 = Interfaces::m_pGlobalVars->curtime + 1.f;
		}

		if (move->m_flSimulationTime > 0.f) {
			if (!record->m_moved) {
				Vector delta = move->m_vecOrigin - record->m_vecOrigin;
				if (delta.Length() <= 128.f && record->m_fFlags & FL_ONGROUND) {
					record->m_moved = true;
				}
			}
		}

		C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
		const int activity = player->GetSequenceActivity(curr->m_nSequence);

		if (pLagData->m_bRoundStart) {
			record->m_moved = false;
		}

		float delta = record->m_anim_time - move->m_anim_time;

		static bool m_iFirstCheck = true;
		static bool m_iRestartDistortCheck = true;
		static int m_iDistortCheck = 0;
		static int m_iResetCheck = 0;
		static float m_flLastDistortTime = 0.f;
		static float m_flMaxDistortTime = Interfaces::m_pGlobalVars->curtime + 0.10f;
		static float m_flLastResetTime = 0.f;
		static float m_flMaxResetTime = Interfaces::m_pGlobalVars->curtime + 0.85f;
		static bool sugma = false;
		static bool balls = false;

		if ((m_iRestartDistortCheck || m_iFirstCheck) && local->IsAlive()) {
			record->m_iDistortTiming = Interfaces::m_pGlobalVars->curtime + 0.15f;
			m_iRestartDistortCheck = false;
			m_iFirstCheck = false;
		}

		if ((player->m_AnimOverlay()[3].m_flWeight == 0.f && player->m_AnimOverlay()[3].m_flCycle == 0.f) &&
			player->m_vecVelocity().Length2D() < 0.1f && !m_iRestartDistortCheck) {
			if (!sugma) {
				m_flMaxDistortTime = Interfaces::m_pGlobalVars->curtime + 0.10f;
				sugma = true;
			}

			m_flLastDistortTime = Interfaces::m_pGlobalVars->curtime;

			if (m_flLastDistortTime >= m_flMaxDistortTime) {
				m_iDistortCheck++;
				m_iResetCheck = 0;
				sugma = false;
			}
		}
		else {
			sugma = false;
		}

		if (player->m_AnimOverlay()[3].m_flCycle >= 0.1f && player->m_AnimOverlay()[3].m_flCycle <= 0.99999f && player->m_vecVelocity().Length2D() < 0.01f && record->m_vecVelocity.Length2D() < 0.1f) {
			if (!balls) {
				m_flMaxResetTime = Interfaces::m_pGlobalVars->curtime + 0.85f;
				balls = true;
			}

			m_flLastResetTime = Interfaces::m_pGlobalVars->curtime;

			if (m_flLastResetTime >= m_flMaxResetTime) {
				m_iResetCheck++;
				balls = false;
			}
		}
		else {
			balls = false;
		}


		if (m_iResetCheck >= 3) {
			m_iRestartDistortCheck = true;
			m_iDistortCheck = 0;
			record->m_iDistorting[player->EntIndex()] = false;
		}

		if (m_iDistortCheck >= 2) {
			record->m_iDistorting[player->EntIndex()] = true;
		}

		const auto simtime = record->m_flSimulationTime;

		if (!local->IsAlive() || !player->IsAlive() || player->IsDormant())
			record->m_moved = false;

		// mark this record if it contains a shot.
		MatchShot(player, record);

		// next up mark this record with a resolver mode that will be used.
		SetMode(player, record);

		// we arrived here we can do the acutal resolve.
		if (record->m_iResolverMode == MOVING && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition)
			ResolveWalk(player, record);

		//else if (record->m_iResolverMode == RESOLVE_OVERRIDE || g_Vars.rage.override_reoslver.enabled)
		//	ResolveOverride(player, record);

		else if (record->m_iResolverMode == AIR || record->m_iResolverMode == AIRSTAND)
			ResolveAir(player, record);

		else {
			resolve(player, record);
		}

		auto animstate = player->m_PlayerAnimState();
		static float absrotation_before_flick;

		if (animstate && !is_flicking) {
			absrotation_before_flick = animstate->m_flAbsRotation;
		}

		// normalize the eye angles, doesn't really matter but its clean.
		Math::NormalizeAngle(record->m_angEyeAngles.y);

		player->m_angEyeAngles() = record->m_angEyeAngles;
	}

	void CResolver::resolve(C_CSPlayer* player, C_AnimationRecord* record) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data) {
			record->m_moved = false;
			return;
		}

		C_AnimationRecord* move = &pLagData->m_walk_record;

		auto at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

		if (is_flicking && pLagData->m_iMissedShotsLBY < 1 && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition && (record->m_vecVelocity.Length2D() < 0.01f || record->m_bFakeWalking))
		{
			record->m_angEyeAngles.y = pLagData->m_flLowerBodyYawTarget;
			record->m_iResolverMode = FLICK;
			record->m_iResolverText = XorStr("UPDATE");
		}
		else {
			auto animstate = player->m_PlayerAnimState();

			if (record->m_bIsFakeFlicking || record->m_bUnsafeVelocityTransition) {
				if (animstate) {
					animstate->m_flAbsRotation = Math::normalize_angle(pLagData->m_body);
				}
				AntiFreestand(record, player);
			}
			else if (record->m_iResolverMode == LASTMOVE) {
				static float absrotation_before_flick;

				if (animstate && !is_flicking) {
					absrotation_before_flick = animstate->m_flAbsRotation;
				}

				static bool repeat[64];
				if (g_ResolverData->hitPlayer[player->EntIndex()] && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition && (player->m_vecVelocity().Length2D() < 0.1f || (player->m_vecVelocity().Length2D() > 0.1f && record->m_bFakeWalking))) {
					if (!repeat[player->EntIndex()]) {
						if (pLagData->m_iMissedLBYLog == 0) {
							g_ResolverData->storedLbyDelta[player->EntIndex()] = Math::normalize_float(record->m_angEyeAngles.y - pLagData->m_body);
						}
						else
							g_ResolverData->storedLbyDelta[player->EntIndex()] = Math::normalize_float(record->m_angEyeAngles.y + pLagData->m_body);

						g_ResolverData->hasStoredLby[player->EntIndex()] = true;
						repeat[player->EntIndex()] = true;
					}
					if (repeat[player->EntIndex()]) {
						g_ResolverData->hasStoredLby[player->EntIndex()] = true;
					}
				}
				else if (!(g_ResolverData->hitPlayer[player->EntIndex()])) {
					g_ResolverData->hasStoredLby[player->EntIndex()] = false;
				}

				if (pLagData->m_iMissedLBYLog >= 2) {
					g_ResolverData->hitPlayer[player->EntIndex()] = false;
					repeat[player->EntIndex()] = false; // log that nigga again
					pLagData->m_iMissedLBYLog = 0; // reset that shit.
				}

				if (g_ResolverData->hasStoredLby[player->EntIndex()] && g_Vars.misc.expermimental_resolver) {
					if (pLagData->m_iMissedLBYLog == 0) {
						record->m_angEyeAngles.y = (record->m_flLowerBodyYawTarget - g_ResolverData->storedLbyDelta[player->EntIndex()]);
						record->m_iResolverText = XorStr("-LBY LOGGED");
					}
					else {
						record->m_angEyeAngles.y = (record->m_flLowerBodyYawTarget + g_ResolverData->storedLbyDelta[player->EntIndex()]);
						record->m_iResolverText = XorStr("+LBY LOGGED");
					}
				}
				else {
					record->m_angEyeAngles.y = move->m_flLowerBodyYawTarget;
					record->m_iResolverText = XorStr("LASTMOVE");
				}
			}
			else if (record->m_iResolverMode == LBYDELTA && pLagData->m_delta_index < 1) {
				record->m_angEyeAngles.y = Math::normalize_float(pLagData->m_flLowerBodyYawTarget - pLagData->m_flSavedLbyDelta);
				record->m_iResolverText = XorStr("DELTA");
			}
			else if (record->m_iResolverMode == DISTORTINGLMOVE) {
				record->m_angEyeAngles.y = at_target_yaw + 180.f;
				record->m_iResolverText = XorStr("DISTORTION");
			}
			else if (record->m_iResolverMode == ANTIFREESTAND && pLagData->m_iMissedShotsFreestand < 1) {
				record->m_iResolverText = XorStr("FREESTAND");
				AntiFreestand(record, player);
			}
			else if (record->m_iResolverMode == STAND) {
				auto animstate = player->m_PlayerAnimState();
				static float absrotation_before_flick;

				if (animstate && !is_flicking) {
					absrotation_before_flick = animstate->m_flAbsRotation;
				}

				static bool repeat[64];
				if (pLagData->m_iMissedLBYLog >= 2) {
					g_ResolverData->hitPlayer[player->EntIndex()] = false;
					repeat[player->EntIndex()] = false; // log that nigga again
					pLagData->m_iMissedLBYLog = 0; // reset that shit.
				}

				if (g_ResolverData->hitPlayer[player->EntIndex()] && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition && (player->m_vecVelocity().Length2D() < 0.1f || (player->m_vecVelocity().Length2D() > 0.1f && record->m_bFakeWalking))) {
					if (!repeat[player->EntIndex()]) {
						if (pLagData->m_iMissedLBYLog == 0) {
							g_ResolverData->storedLbyDelta[player->EntIndex()] = Math::normalize_float(record->m_angEyeAngles.y + pLagData->m_body);
						}
						else
							g_ResolverData->storedLbyDelta[player->EntIndex()] = Math::normalize_float(record->m_angEyeAngles.y - pLagData->m_body);

						g_ResolverData->hasStoredLby[player->EntIndex()] = true;
						repeat[player->EntIndex()] = true;
					}
					if (repeat[player->EntIndex()]) {
						g_ResolverData->hasStoredLby[player->EntIndex()] = true;
					}
				}
				else if (!(g_ResolverData->hitPlayer[player->EntIndex()])) {
					g_ResolverData->hasStoredLby[player->EntIndex()] = false;
				}

				if (g_ResolverData->hasStoredLby[player->EntIndex()] && g_Vars.misc.expermimental_resolver) {
					if (pLagData->m_iMissedLBYLog == 0) {
						record->m_angEyeAngles.y = (record->m_flLowerBodyYawTarget - g_ResolverData->storedLbyDelta[player->EntIndex()]);
						record->m_iResolverText = XorStr("-LBY LOGGED");
					}
					else {
						record->m_angEyeAngles.y = (record->m_flLowerBodyYawTarget + g_ResolverData->storedLbyDelta[player->EntIndex()]);
						record->m_iResolverText = XorStr("+LBY LOGGED");
					}
				}
				else {
					switch (pLagData->m_iMissedBruteShots % 8) {
					case 0:
						if (pLagData->m_iMissedShotsDistort < 1) {
							record->m_angEyeAngles.y = at_target_yaw + 180.f;
							record->m_iResolverText = XorStr("BACKWARDS");
						}
						else {
							AntiFreestand(record, player);
							record->m_iResolverText = XorStr("DISTORTFSTAND");
						}
						break;
					case 1:
						record->m_angEyeAngles.y = at_target_yaw - 70.f;
						record->m_iResolverText = XorStr("LEFT");
						break;
					case 2:
						record->m_angEyeAngles.y = at_target_yaw + 70.f;
						record->m_iResolverText = XorStr("RIGHT");
						break;
					case 3:
						record->m_angEyeAngles.y = at_target_yaw;
						record->m_iResolverText = XorStr("FORWARDS");
						break;
					case 4:
						record->m_angEyeAngles.y = at_target_yaw - 120.f;
						record->m_iResolverText = XorStr("-120");
						break;
					case 5:
						record->m_angEyeAngles.y = at_target_yaw + 120.f;
						record->m_iResolverText = XorStr("+120");
						break;
					case 6:
						static float randomfloat = RandomFloat(35.0, 120.0);

						record->m_angEyeAngles.y = randomfloat + pLagData->m_body;
						record->m_iResolverText = XorStr("LBY+RND");
						break;
					case 7:
						static float randomfloat2 = RandomFloat(35.0, 60.0);

						record->m_angEyeAngles.y = pLagData->m_body - randomfloat2;
						record->m_iResolverText = XorStr("LBY-RND");
						break;
					}
				}
			}
		}
	}

	void CResolver::on_lby_proxy(C_CSPlayer* entity, float* LowerBodyYaw)
	{
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(entity->m_entIndex);
		if (!pLagData.IsValid())
			return;


		float oldBodyYaw; // xmm4_4
		float nextPredictedSimtime; // xmm3_4
		float nextPredictedSimtimeAccurate; // xmm3_4
		float nextBodyUpdate = 0.f; // xmm3_4

		oldBodyYaw = pLagData->m_body;

		if (oldBodyYaw != *LowerBodyYaw)
		{
			if (entity != local && entity->m_fFlags() & FL_ONGROUND)
			{
				//---------------
				nextPredictedSimtime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
				float vel = entity->m_vecVelocity().Length2D();
				if (vel > 0.1f)
					nextBodyUpdate = nextPredictedSimtime + 0.22f;
				else if (pLagData->nextBodyUpdate <= nextPredictedSimtime)
					nextBodyUpdate = nextPredictedSimtime + 1.1f;

				if (nextBodyUpdate != 0.f)
					pLagData->nextBodyUpdate = nextBodyUpdate;
			}

			pLagData->m_flLastLowerBodyYawTargetUpdateTime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
			pLagData->m_flOldLowerBodyYawTarget = oldBodyYaw;
			pLagData->m_flLowerBodyYawTarget = Math::normalize_float(*LowerBodyYaw);

			//util::print_dev_console( true, Color::Green(), "update time = %f\n", PlayerRecord->m_flLastLowerBodyYawTargetUpdateTime );
		}

		if (entity->m_vecVelocity().Length2D() > 0.1f && !Engine::g_ResolverData[entity->EntIndex()].fakewalking && entity->m_fFlags() & FL_ONGROUND)
		{
			pLagData->m_flLastMovingLowerBodyYawTarget = Math::normalize_float(*LowerBodyYaw);
			pLagData->m_flLastMovingLowerBodyYawTargetTime = entity->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
		}
	}


	void CResolver::OnBodyUpdate(C_CSPlayer* player, float value) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		float oldBodyYaw; // xmm4_4
		float nextPredictedSimtime; // xmm3_4
		float nextPredictedSimtimeAccurate; // xmm3_4
		float nextBodyUpdate = 0.f; // xmm3_4

		oldBodyYaw = pLagData->m_flLowerBodyYawTarget;
		
		if (oldBodyYaw != value) {
			pLagData->m_flLastLowerBodyYawTargetUpdateTime = player->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
			pLagData->m_flOldLowerBodyYawTarget = oldBodyYaw;
			pLagData->m_flLowerBodyYawTarget = Math::normalize_float(value);
		}

		if (player->m_vecVelocity().Length2D() > 0.1f && player->m_fFlags() & FL_ONGROUND && !Engine::g_ResolverData[player->EntIndex()].fakewalking /*&& !pLagData->m_bUnsafeVelocityTransition*/)
		{
			pLagData->m_flLastMovingLowerBodyYawTarget = Math::normalize_float(value);
			pLagData->m_flLastMovingLowerBodyYawTargetTime = player->m_flOldSimulationTime() + Interfaces::m_pGlobalVars->interval_per_tick;
		}

		// set data.
		pLagData->m_old_body = pLagData->m_body;
		pLagData->m_body = value;
	}

	void CResolver::ResolveWalk(C_CSPlayer* data, C_AnimationRecord* record) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(data->m_entIndex);
		if (!pLagData.IsValid())
			return;

		record->m_iResolverText = XorStr("MOVING");

		// apply lby to eyeangles.
		record->m_angEyeAngles.y = pLagData->m_body;

		float speed = record->m_vecVelocity.Length2D();

		if (record->m_fFlags & FL_ONGROUND && speed > 1.f && data->m_fFlags() & FL_ONGROUND && !record->m_bFakeWalking && !record->m_bIsFakeFlicking && !record->m_bUnsafeVelocityTransition) {
			record->m_moved = true;
		}

		pLagData->m_iMissedShots = 0;
		pLagData->m_iMissedShotsInAir = 0;
		pLagData->m_iMissedBruteShots = 0;
		pLagData->m_iMissedShotsDistort = 0;
		pLagData->m_iMissedShotsFreestand = 0;
		pLagData->m_iMissedShotsLBYTEST = 0;
		pLagData->m_stand_index2 = 0;
		pLagData->m_iMissedShotsLBY = 0;
		pLagData->m_last_move = 0;
		pLagData->m_unknown_move = 0;
		pLagData->m_delta_index = 0;

		// copy the last record that this player was walking
		// we need it later on because it gives us crucial data.
		std::memcpy(&pLagData->m_walk_record, record, sizeof(C_AnimationRecord));
	}

	float CResolver::GetLBYRotatedYaw(float lby, float yaw)
	{
		float delta = Math::NormalizedAngle(yaw - lby);
		if (fabs(delta) < 25.f)
			return lby;

		if (delta > 0.f)
			return yaw + 25.f;

		return yaw;
	}

	float CResolver::GetAwayAngle(C_AnimationRecord* record) {
		float  delta{ std::numeric_limits< float >::max() };
		Vector pos;
		QAngle  away;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return 0.f;

		// other cheats predict you by their own latency.
		// they do this because, then they can put their away angle to exactly
		// where you are on the server at that moment in time.

		// the idea is that you would need to know where they 'saw' you when they created their user-command.
		// lets say you move on your client right now, this would take half of our latency to arrive at the server.
		// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

		// we have no historical origins.
		// no choice but to use the most recent one.
		//if( g_cl.m_net_pos.empty( ) ) {
		Math::VectorAngles(local->m_vecOrigin() - record->m_vecOrigin, away);
		return away.y;
	}

	bool CResolver::AntiFreestanding(C_CSPlayer* entity, float& yaw)
	{

		const auto freestanding_record = anti_freestanding_record;

		//g_pEntitiyList->GetClientEntity(g_pEngine->GetLocalPlayer())

		auto local_player = C_CSPlayer::GetLocalPlayer();
		if (!local_player)
			return false;

		float at_target_yaw = Math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;

		if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
			yaw = at_target_yaw;

		auto set = false;

		if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
		{
			if (freestanding_record.right_fraction < freestanding_record.left_fraction) {
				set = true;
				yaw = at_target_yaw + 125.f;
			}
			else if (freestanding_record.right_fraction > freestanding_record.left_fraction) {
				set = true;
				yaw = at_target_yaw - 73.f;
			}
			else {
				yaw = at_target_yaw;
			}
		}
		else
		{
			if (freestanding_record.left_damage > freestanding_record.right_damage) {
				yaw = at_target_yaw + 130.f;
				set = true;
			}
			else
				yaw = at_target_yaw;
		}

		return true;
	}

	bool CResolver::ShouldUseFreestand(C_AnimationRecord* record, C_CSPlayer* player) // allows freestanding if not in open
	{
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return false;

		if (!player || player == local)
			return false;

		/* externs */
		Vector src3D, dst3D, forward, right, up, src, dst;
		float back_two, right_two, left_two;
		CGameTrace tr;
		CTraceFilterSimple filter;

		/* angle vectors */
		Math::angle_vectors(QAngle(0, GetAwayAngle(record), 0), &forward, &right, &up);

		/* filtering */
		filter.SetPassEntity(player);
		src3D = player->GetShootPosition();
		dst3D = src3D + (forward * 384);

		/* back engine tracers */
		Interfaces::m_pEngineTrace->TraceRay(Ray_t(src3D, dst3D), MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &tr);
		back_two = (tr.endpos - tr.startpos).Length();

		/* right engine tracers */
		Interfaces::m_pEngineTrace->TraceRay(Ray_t(src3D + right * 35, dst3D + right * 35), MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &tr);
		right_two = (tr.endpos - tr.startpos).Length();

		/* left engine tracers */
		Interfaces::m_pEngineTrace->TraceRay(Ray_t(src3D - right * 35, dst3D - right * 35), MASK_SHOT_BRUSHONLY | MASK_OPAQUE_AND_NPCS, &filter, &tr);
		left_two = (tr.endpos - tr.startpos).Length();

		/* fix side */
		if (left_two > right_two) {
			bFacingleft = true;
			bFacingright = false;
			return true;
		}
		else if (right_two > left_two) {
			bFacingright = true;
			bFacingleft = false;
			return true;
		}
		else
			return false;
	}

	// Use this resolver for players who are in air moving < 60 units per second
	void CResolver::LastMoveLby(C_AnimationRecord* record, C_CSPlayer* player)
	{
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data)
			return;

		const auto simtime = record->m_flSimulationTime;

		// pointer for easy access.
		C_AnimationRecord* move = &pLagData->m_walk_record;

		// get predicted away angle for the player.
		float away = GetAwayAngle(record);

		C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
		int act = player->GetSequenceActivity(curr->m_nSequence);

		float diff = Math::NormalizedAngle(record->m_flLowerBodyYawTarget - move->m_flLowerBodyYawTarget);
		float delta = record->m_anim_time - move->m_anim_time;
		QAngle vAngle = QAngle(0, 0, 0);
		Math::CalcAngle3(player->m_vecOrigin(), local->m_vecOrigin(), vAngle);

		float flToMe = vAngle.y;

		auto index = player->EntIndex();

		float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

		const auto freestanding_record = player_resolve_records[player->EntIndex()].m_sAntiEdge;

		if (move->m_flSimulationTime > 0.f) {
			if (!record->m_moved) {
				Vector delta = move->m_vecOrigin - record->m_vecOrigin;
				if (delta.Length() <= 128.f && record->m_fFlags & FL_ONGROUND) {
					record->m_moved = true;
					//printf("move true 2\n");
				}
			}
		}

			if (!record->m_moved) {

				record->m_iResolverMode = STAND;


				float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

				if (is_flicking && pLagData->m_iMissedShotsLBY < 1 /* && !record->m_bFakeWalking*/)
				{
					record->m_angEyeAngles.y = pLagData->m_flLowerBodyYawTarget;
					record->m_iResolverMode = FLICK;
					record->m_iResolverText = XorStr("UPDATE");
				}
				else {
					switch (pLagData->m_unknown_move % 4) {
					case 0:
						AntiFreestand(record, player);
						m_iMode = 1;
						break;
					case 1:
						record->m_angEyeAngles.y = at_target_yaw + 180.f;
						record->m_iResolverText = XorStr("NMOVE +180");
						m_iMode = 0;
						break;
					case 2:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) + 70.f;
						record->m_iResolverText = XorStr("NMOVE +70");
						m_iMode = 0;
						break;
					case 3:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) - 70.f;
						record->m_iResolverText = XorStr("NMOVE -70");
						m_iMode = 0;
						break;
					}
				}
			}
			else if (record->m_moved) {
				float diff = Math::NormalizedAngle(record->m_flLowerBodyYawTarget - move->m_flLowerBodyYawTarget);
				float delta = record->m_anim_time - move->m_anim_time;
				C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
				const int activity = player->GetSequenceActivity(curr->m_nSequence);


				record->m_iResolverMode = LASTMOVE;

				float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

				if (is_flicking && pLagData->m_iMissedShotsLBY < 1/* && !record->m_bFakeWalking*/)
				{
					record->m_angEyeAngles.y = pLagData->m_flLowerBodyYawTarget;
					record->m_iResolverMode = FLICK;
					record->m_iResolverText = XorStr("UPDATE");
				}
				else {
					switch (pLagData->m_last_move % 5) {
					case 0:
						AntiFreestand(record, player);
						m_iMode = 1;
						break;
					case 1:
						if (AntiFreestanding(player, record->m_angEyeAngles.y) && pLagData->m_iMissedShotsFreestand < 1) { // using same freestand twice might not be a good idea so i switched to fatal here
							record->m_iResolverText = XorStr("FREESTAND_F");
							m_iMode = 1;
						}
						else {
							record->m_angEyeAngles.y = at_target_yaw + 180.f;
							record->m_iResolverText = XorStr("BACKWARDS_F");
							m_iMode = 0;
						}
						break;
					case 2:
						record->m_angEyeAngles.y = at_target_yaw + 180.f;
						record->m_iResolverText = XorStr("BACKWARDS");
						m_iMode = 0;
						break;
					case 3:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) + 70.f;
						record->m_iResolverText = XorStr("+70");
						m_iMode = 0;
						break;
					case 4:
						record->m_angEyeAngles.y = (at_target_yaw + 180.f) - 70.f;
						record->m_iResolverText = XorStr("-70");
						m_iMode = 0;
						break;
					}
				}
			}
	}

	void CResolver::ResolveAir(C_CSPlayer* player, C_AnimationRecord* record) {
		// get lag data.
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		record->m_iResolverText = XorStr("AIR");

		// we have barely any speed. 
		// either we jumped in place or we just left the ground.
		// or someone is trying to fool our resolver.
		if (record->m_vecAnimationVelocity.Length2D() < 60.f) {
			// set this for completion.
			// so the shot parsing wont pick the hits / misses up.
			// and process them wrongly.
			record->m_iResolverMode = STAND;

			// invoke our stand resolver.
			LastMoveLby(record, player);
		}
		else {
			float away = GetAwayAngle(record);

			// try to predict the direction of the player based on his velocity direction.
			// this should be a rough estimation of where he is looking.
			float velyaw = RAD2DEG(std::atan2(record->m_vecAnimationVelocity.y, record->m_vecAnimationVelocity.x));

			switch (pLagData->m_iMissedShotsInAir % 4) {
			case 0:
				if (g_Vars.misc.expermimental_resolver) {
					record->m_angEyeAngles.y = pLagData->m_body;
				}
				else
					record->m_angEyeAngles.y = away - 180.f;
				break;

			case 1:
				record->m_angEyeAngles.y = velyaw - 110.f;
				break;

			case 2:
				record->m_angEyeAngles.y = velyaw + 110.f;
				break;

			case 3:
				record->m_angEyeAngles.y = velyaw;
				break;
			}
		}
	}

	void CResolver::ResolveOverride(C_CSPlayer* player, C_AnimationRecord* record) {
		Encrypted_t<Engine::C_EntityLagData> pLagData = Engine::LagCompensation::Get()->GetLagData(player->m_entIndex);
		if (!pLagData.IsValid())
			return;

		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local)
			return;

		auto anim_data = AnimationSystem::Get()->GetAnimationData(player->m_entIndex);
		if (!anim_data)
			return;

		// get predicted away angle for the player.
		float away = GetAwayAngle(record);

		// pointer for easy access.
		C_AnimationRecord* move = &pLagData->m_walk_record;

		C_AnimationLayer* curr = &record->m_serverAnimOverlays[3];
		int act = player->GetSequenceActivity(curr->m_nSequence);

		Engine::g_ResolverData->m_bInOverride[player->EntIndex()] = false;

		if (g_Vars.rage.override_reoslver.enabled) {
			QAngle viewangles = g_Vars.globals.CurrentLocalViewAngles;

			const float at_target_yaw = Math::CalcAngle(local->m_vecOrigin(), player->m_vecOrigin()).y;

			if (fabs(Math::NormalizedAngle(viewangles.y - at_target_yaw)) > 30.f) {
				return resolve(player, record);
			}

			Engine::g_ResolverData->m_bInOverride[player->EntIndex()] = true;
			record->m_angEyeAngles.y = (Math::NormalizedAngle(viewangles.y - at_target_yaw) > 0) ? at_target_yaw + 90.f : at_target_yaw - 90.f;
			record->m_iResolverMode = RESOLVE_OVERRIDE;
			record->m_iResolverText = XorStr("OVERRIDE");
		}
	}
}
