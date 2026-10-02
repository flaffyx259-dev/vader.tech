#pragma once

#include "entity.hpp"

#include "PlayerAnimState.hpp"
#include "../Valve/CBaseHandle.hpp"

class C_BaseViewModel;

class VarMapEntry_t
{
public:
	unsigned short type;
	unsigned short m_bNeedsToInterpolate;
	void* data;
	void* watcher;
};

struct VarMapping_t
{
	CUtlVector<VarMapEntry_t> m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};

class CPlayerState {
public:
   virtual ~CPlayerState( ) { }
   bool deadflag;
   QAngle		v_angle; // Viewing angle (player only)

   // The client .dll only cares about deadflag
   // the game and engine .dlls need to worry about the rest of this data
   // Player's network name
   string_t	netname;

   // 0:nothing, 1:force view angles, 2:add avelocity
   int			fixangle;

   // delta angle for fixangle == FIXANGLE_RELATIVE
   QAngle		anglechange;

   // flag to single the HLTV/Replay fake client, not transmitted
   bool		hltv;
   bool		replay;
   int			frags;
   int			deaths;
};

class C_BasePlayer : public C_BaseCombatCharacter {
public:
   QAngle& m_aimPunchAngle( );
   QAngle& m_aimPunchAngleVel( );
   QAngle& m_viewPunchAngle( );
   Vector& m_vecViewOffset( );
   Vector& m_vecVelocity( );
   Vector& m_vecBaseVelocity( );
   float& m_flFallVelocity( );
   float& m_flDuckAmount( );
   float& m_flDuckSpeed( );
   char& m_lifeState( );
   int& m_nTickBase( );
   int& m_iHealth( );
   bool IsTeammate(C_BasePlayer* player);
   int& m_fFlags( );
   int& m_iDefaultFOV( );
   int& m_iObserverMode( );

   CPlayerState& pl( );
   CBaseHandle& m_hObserverTarget( );
   CHandle<C_BaseViewModel> m_hViewModel( );
   int& m_vphysicsCollisionState( );
   float GetMaxSpeed( );
   float SequenceDuration( CStudioHdr* pStudioHdr, int iSequence );
   float GetFOV( );
   const Vector& WorldSpaceCenter( );
   float GetSequenceMoveDist( CStudioHdr* pStudioHdr, int iSequence );

public:
   void* Renderable();
   void DrawModel(int flags = STUDIO_RENDER, const RenderableInstance_t& instance = {});
   bool IsAlive();
   bool& m_bSpotted();
   VarMapping_t* VarMapping();
   bool IsDead( );
   void SetCurrentCommand( CUserCmd* cmd );

   Vector GetEyePosition();
   Vector GetViewHeight( );
   C_AnimationLayer& GetAnimLayer( int index );

   float GetLayerSequenceCycleRate( C_AnimationLayer* pLayer, int iSequence );
   void TryInitiateAnimation( C_AnimationLayer* pLayer, int nSequence );
   bool& m_bIsWalking();
};

class C_CSPlayer : public C_BasePlayer {
public:
	template< typename t >
	__forceinline t& get(size_t offset) {
		return *(t*)((uintptr_t)this + offset);
	}

	bool valid(bool check_team, bool check_dormant);
	static C_CSPlayer* GetLocalPlayer( );
   static C_CSPlayer* GetPlayerByIndex( int index );

   Vector& GetHitboxPosition(int hitbox);

   bool IsTeammate( C_CSPlayer* player );
   Vector GetShootPosition();
   bool CanShoot( bool skip_revolver = false);
   bool IsReloading( );
   float& get_creation_time();

   Vector GetEyePosition( );
   void ForceAngleTo(QAngle angle);
public:
   CCSGOPlayerAnimState*& m_PlayerAnimState( );
   float& m_surfaceFriction( );
   QAngle& m_angEyeAngles( );
   int& m_nSurvivalTeam( );
   int& m_ArmorValue( );
   int& m_iAccount( );
   int& m_iFOV( );
   int& m_iShotsFired( );
   float& m_flFlashDuration( );
   float& m_flSecondFlashAlpha( );
   float& m_flVelocityModifier( );
   float& m_flLowerBodyYawTarget( );
   float& m_flSpawnTime( );
   float& m_flHealthShotBoostExpirationTime( );
   bool& m_bHasHelmet( );
   bool& m_bHasHeavyArmor( );
   bool& m_bIsScoped( );
   bool& m_bWaitForNoAttack( );
   bool& m_bIsPlayerGhost( );
   std::array<int, 5>& m_vecPlayerPatchEconIndices( );
   int& m_iMatchStats_Kills( );
   int& m_iMatchStats_Deaths( );
   int& m_iMatchStats_HeadShotKills( );
   bool& m_bGunGameImmunity( );
   bool& m_bIsDefusing( );
   bool& m_bHasDefuser( );
   bool PhysicsRunThink( int unk01 );
   int SetNextThink( int tick );
   void Think( );
   void PreThink( );
   void PostThink( );

   bool	is( std::string networkname );

};

FORCEINLINE C_CSPlayer* ToCSPlayer( C_BaseEntity* pEnt ) {
   if ( !pEnt || !pEnt->m_entIndex || !pEnt->IsPlayer( ) )
	  return nullptr;

   return ( C_CSPlayer* ) ( pEnt );
}
