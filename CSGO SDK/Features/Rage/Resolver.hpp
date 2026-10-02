#pragma once
#include "LagCompensation.hpp"
#include <vector>
#include <deque>

struct AntiFreestandingRecord
{
	int right_damage = 0, left_damage = 0, back_damage = 0;
	float right_fraction = 0.f, left_fraction = 0.f, back_fraction = 0.f;
};

namespace Engine {
	// taken from supremacy
	enum EResolverModes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_WALK,
		RESOLVE_STAND,
		RESOLVE_STAND1,
		RESOLVE_STAND2,
		RESOLVE_AIR,
		RESOLVE_BODY,
		RESOLVE_STOPPED_MOVING,
		RESOLVE_LBY,
		RESOLVE_OVERRIDE,
		RESOLVE_LASTMOVE,
		RESOLVE_ANTIFREESTAND,
		RESOLVE_UNKNOWM,
		RESOLVE_DELTA,
		RESOLVE_BRUTEFORCE,
	};

	enum RModes {
		LBYDELTA = 0,
		FLICK,
		NOLBY,
		LASTMOVE,
		DISTORTINGLMOVE,
		STAND,
		MOVING,
		ANTIFREESTAND,
		BRUTE,
		OVERRIDE,
		SPIN,
		AGFREESTAND,
		JITTER,
		AIR,
		AIRSTAND
	};

	struct CResolverData {
		struct LastMoveData_t {
			float m_flLowerBodyYawTarget;
			float m_flLastMovingLowerBodyYawTargetTime;
			float m_flSimulationTime;
			float m_flAnimTime;
			Vector m_vecOrigin;
		};

		bool m_bCollectedValidMoveData;
		bool m_bWentDormant;
		bool m_bPredictingUpdates;
		bool is_flicking;
		bool m_player_fire;
		int m_iMode;

		Vector m_vecSavedOrigin;
		LastMoveData_t m_sMoveData;

		float m_flBestYaw;
		float m_flBestDistance;
		bool  m_bCollectedFreestandData;
		bool fakewalking;
		bool hasStoredLby[65];
		bool hitPlayer[65];
		bool m_bInOverride[65];

		float m_flNextBodyUpdate;
		float m_flLastLowerBodyYawTargetUpdateTime;
		float m_flFinalResolverYaw;
		float storedLbyDelta[65];
		float m_flOldLowerBodyYawTarget;
		float m_flLowerBodyYawTarget;

	};

	class CResolver {
	private:
		void ResolveWalk(C_CSPlayer* data, C_AnimationRecord* record);
		void ResolveStand(C_CSPlayer* player, C_AnimationRecord* record);
		void ResolveAir(C_CSPlayer* player, C_AnimationRecord* record);
		void ResolveOverride(C_CSPlayer* player, C_AnimationRecord* record);

		Vector last_eye;

		float left_damage[64];
		float right_damage[64];
		float back_damage[64];

		int m_iMode;

		bool bFacingleft;
		bool bFacingright;

		std::vector<Vector> last_eye_positions;

	public:
		void ResolveManual(C_CSPlayer* player, C_AnimationRecord* record, bool bDisallow = false);
		void override_resolver(C_CSPlayer* player, C_AnimationRecord* record);
		void ResolveYaw(C_CSPlayer* player, C_AnimationRecord* record);
		void OnBodyUpdate(C_CSPlayer* player, float value);
		void MatchShot(C_CSPlayer* data, C_AnimationRecord* record);
		void SetMode(C_CSPlayer* player, C_AnimationRecord* record);
		void LastMoveLby(C_AnimationRecord* record, C_CSPlayer* player);
		float GetDirectionAngle(int index, C_CSPlayer* player);
		float GetLBYRotatedYaw(float lby, float yaw);
		float GetAwayAngle(C_AnimationRecord* record);
		void ResolveAngles(C_CSPlayer* player, C_AnimationRecord* record);

		AntiFreestandingRecord anti_freestanding_record;

		class PlayerResolveRecord
		{
		public:
			struct AntiFreestandingRecord
			{
				int right_damage = 0, left_damage = 0;
				float right_fraction = 0.f, left_fraction = 0.f;
			};

		public:
			AntiFreestandingRecord m_sAntiEdge;
		};

		PlayerResolveRecord player_resolve_records[33];

	public:
		// check if the players yaw is sideways.
		bool IsLastMoveValid(C_AnimationRecord* record, float m_yaw) {
			auto local = C_CSPlayer::GetLocalPlayer();
			if (!local)
				return false;
			Vector angAway;
			Math::VectorAngles(local->m_vecOrigin() - record->m_vecOrigin, angAway);
			const float delta = fabs(Math::AngleNormalize(angAway.y - m_yaw));
			return delta > 20.f && delta < 160.f;
		}

		// freestanding.
		class AdaptiveAngle {
		public:
			float m_yaw;
			float m_dist;

		public:
			// ctor.
			__forceinline AdaptiveAngle() :
				m_yaw{ },
				m_dist{ }
			{ };

			__forceinline AdaptiveAngle(float yaw, float penalty = 0.f) {
				// set yaw.
				m_yaw = Math::AngleNormalize(yaw);

				// init distance.
				m_dist = 0.f;

				// remove penalty.
				m_dist -= penalty;
			}
		};
		void resolve(C_CSPlayer* player, C_AnimationRecord* record);
		void on_lby_proxy(C_CSPlayer* entity, float* LowerBodyYaw);
		bool IsBreakingLBY120(C_CSPlayer* pEntity);
		bool IsResolvableByLBY(C_CSPlayer* pEntity);
		void collect_wall_detect(const ClientFrameStage_t stage);
		bool wall_detect(C_CSPlayer* player, C_AnimationRecord* record, float& angle) const;
		void AntiFreestand(C_AnimationRecord* record, C_CSPlayer* entity);
		void FindBestAngle(C_CSPlayer* player);
		bool AntiFreestanding(C_CSPlayer* entity, float& yaw);
		bool is_spin(C_AnimationRecord* record, C_CSPlayer* player);
		bool ShouldUseFreestand(C_AnimationRecord* record, C_CSPlayer* player);
		//bool wall_detect(Engine::C_LagRecord* record, float& angle) const;
	};

	extern CResolver g_Resolver;
	extern CResolverData g_ResolverData[65];
}