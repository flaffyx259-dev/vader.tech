#include "Autowall.h"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/Classes/weapon.hpp"

//#define DEBUG_AUTOWALL

#ifdef DEBUG_AUTOWALL
#include XorStr( "InputSys.hpp" )
#define DEBUG_DURATION Interfaces::m_pCvar->FindVar(XorStr( "sv_showimpacts_time" ))->GetFloat()
#endif

// IsBreakableEntity
// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/obstacle_pushaway.cpp
bool Autowall::IsBreakable( C_BaseEntity* pEntity ) {
	if( !pEntity || pEntity->m_entIndex == 0 || !pEntity->GetCollideable( ) )
		return false;

	static uintptr_t uTakeDamage = *( uintptr_t* )( ( uintptr_t )Engine::Displacement.Function.m_uIsBreakable + 38 );
	const uintptr_t uTakeDamageBackup = *( uint8_t* )( ( uintptr_t )pEntity + uTakeDamage );

	const ClientClass* pClientClass = pEntity->GetClientClass( );
	if( pClientClass ) {
		const char* name = pClientClass->m_pNetworkName;

		// CBreakableSurface, CBaseDoor, ...
		//if( name[ 1 ] != 'F'
		//	|| name[ 4 ] != 'c'
		//	|| name[ 5 ] != 'B'
		//	|| name[ 9 ] != 'h' ) {
		//	*( uint8_t* )( ( uintptr_t )pEntity + uTakeDamage ) = 2; /*DAMAGE_YES*/
		//}

		if (!strcmp(name, "CBreakableSurface"))
			*(uint8_t*)((uintptr_t)pEntity + uTakeDamage) = 2; /*DAMAGE_YES*/
		else if (!strcmp(name, "CBaseDoor") || !strcmp(name, "CDynamicProp"))
			*(uint8_t*)((uintptr_t)pEntity + uTakeDamage) = 0; /*DAMAGE_NO*/

	}

	using fnIsBreakable = bool( __thiscall* )( C_BaseEntity* );
	const bool bResult = ( ( fnIsBreakable )Engine::Displacement.Function.m_uIsBreakable )( pEntity );
	*( uint8_t* )( ( uintptr_t )pEntity + uTakeDamage ) = uTakeDamageBackup;

	return bResult;
}

bool Autowall::IsArmored( C_CSPlayer* player, int nHitgroup ) {
	const bool bHasHelmet = player->m_bHasHelmet( );
	const bool bHasHeavyArmor = player->m_bHasHeavyArmor( );
	const float flArmorValue = player->m_ArmorValue( );

	if( flArmorValue > 0 ) {
		switch( nHitgroup ) {
		case Hitgroup_Chest:
		case Hitgroup_Stomach:
		case Hitgroup_LeftArm:
		case Hitgroup_RightArm:
			return true;
			break;
		case Hitgroup_Head:
			return bHasHelmet || bHasHeavyArmor;
			break;
		default:
			return bHasHeavyArmor;
			break;
		}
	}

	return false;
}

// references CCSPlayer::TraceAttack and CCSPlayer::OnTakeDamage
float Autowall::ScaleDamage( C_CSPlayer* player, float flDamage, float flArmorRatio, int nHitgroup ) {
	if( !player )
		return -1.f;

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

	if( !pLocal )
		return -1.f;

	C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )pLocal->m_hActiveWeapon( ).Get( );

	if( !pWeapon )
		return -1.f;

	const int nTeamNum = player->m_iTeamNum( );
	//float flHeadDamageScale = nTeamNum == TEAM_CT ? g_Vars.mp_damage_scale_ct_head->GetFloat( ) : g_Vars.mp_damage_scale_t_head->GetFloat( );
	//const float flBodyDamageScale = nTeamNum == TEAM_CT ? g_Vars.mp_damage_scale_ct_body->GetFloat( ) : g_Vars.mp_damage_scale_t_body->GetFloat( );

	const bool bIsArmored = IsArmored( player, nHitgroup );
	const bool bHasHeavyArmor = player->m_bHasHeavyArmor( );
	const bool bIsZeus = pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS;

	const float flArmorValue = static_cast< float >( player->m_ArmorValue( ) );

	//if( bHasHeavyArmor )
	//	flHeadDamageScale /= 2.f;

	if( !bIsZeus ) {
		switch( nHitgroup ) {
		case Hitgroup_Head:
			if (bHasHeavyArmor)
				flDamage = (flDamage * 4.f) * 0.5f;
			else
				flDamage = (flDamage * 4.f);
			break;
		case Hitgroup_Stomach:
			flDamage = ( flDamage * 1.25f );
			break;
		case Hitgroup_LeftLeg:
		case Hitgroup_RightLeg:
			flDamage = ( flDamage * 0.75f );
			break;
		default:
			break;
		}
	}

	// enemy have armor
	if( bIsArmored ) {
		float flArmorScale = 1.f;
		float flArmorBonusRatio = 0.5f;
		float flArmorRatioCalculated = flArmorRatio * 0.5f;
		float fDamageToHealth = 0.f;

		if( bHasHeavyArmor ) {
			flArmorRatioCalculated = flArmorRatio * 0.25f;
			flArmorBonusRatio = 0.33f;

			flArmorScale = 0.33f;

			fDamageToHealth = ( flDamage * flArmorRatioCalculated ) * 0.85f;
		}
		else {
			fDamageToHealth = flDamage * flArmorRatioCalculated;
		}

		float fDamageToArmor = ( flDamage - fDamageToHealth ) * ( flArmorScale * flArmorBonusRatio );

		//// Does this use more armor than we have?
		//if( fDamageToArmor > flArmorValue )
		//	fDamageToHealth = flDamage - ( flArmorValue / flArmorBonusRatio );

		if (((flDamage - fDamageToHealth) * (flArmorScale * flArmorBonusRatio)) > flArmorValue)
			fDamageToHealth = flDamage - (flArmorValue / flArmorBonusRatio);

		flDamage = fDamageToHealth;
	}

	return std::floor( flDamage );
}

void Autowall::TraceLine( const Vector& start, const Vector& end, uint32_t mask, C_BasePlayer* ignore, CGameTrace* ptr ) {
	Ray_t ray;
	ray.Init( start, end );
	CTraceFilter filter;
	filter.pSkip = ignore;
	Interfaces::m_pEngineTrace->TraceRay( ray, mask, &filter, ptr );
}

void Autowall::ClipTraceToPlayer( const Vector vecAbsStart, const Vector& vecAbsEnd, uint32_t iMask, ITraceFilter* pFilter, CGameTrace* pGameTrace, Encrypted_t<Autowall::C_FireBulletData> pData ) {
	ICollideable* pCollideble = pData->m_TargetPlayer->GetCollideable();

	if (!pCollideble)
		return;
	
	const auto mins = pCollideble->OBBMins();
	const auto maxs = pCollideble->OBBMaxs();

	auto dir(vecAbsEnd - vecAbsStart);
	auto len = dir.Normalize();

	const auto center = (mins + maxs) / 2;
	const auto pos(pData->m_TargetPlayer->m_vecOrigin() + center);

	auto to = pos - vecAbsStart;
	const float range_along = dir.Dot(to);

	float range;
	if (range_along < 0.f) {
		range = -(to).Length();
	}
	else if (range_along > len) {
		range = -(pos - vecAbsEnd).Length();
	}
	else {
		auto ray(pos - (vecAbsStart + (dir * range_along)));
		range = ray.Length();
	}

	if (range <= 60.f) {

		Ray_t ray;
		ray.Init(vecAbsStart, vecAbsEnd);

		CGameTrace trace;
		Interfaces::m_pEngineTrace->ClipRayToEntity(ray, 0x4600400B, pData->m_TargetPlayer, &trace);

		if (pGameTrace->fraction > trace.fraction)
			*pGameTrace = trace;
	}
}

void Autowall::ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t iMask, ITraceFilter* pFilter, CGameTrace* pGameTrace, float flMaxRange, float flMinRange ) {
	float flSmallestFraction = pGameTrace->fraction;

	Vector vecDelta( vecAbsEnd - vecAbsStart );
	const float flDelta = vecDelta.Normalize( );

	Ray_t Ray;
	Ray.Init( vecAbsStart, vecAbsEnd );

	for( int i = 0; i <= Interfaces::m_pGlobalVars->maxClients; ++i ) {
		C_CSPlayer* pPlayer = C_CSPlayer::GetPlayerByIndex( i );
		if( !pPlayer || pPlayer->IsDormant( ) || pPlayer->IsDead( ) )
			continue;

		if( pFilter && !pFilter->ShouldHitEntity( pPlayer, iMask ) )
			continue;

		ICollideable* pCollideble = pPlayer->GetCollideable( );
		if( !pCollideble )
			continue;

		// get bounding box
		const Vector vecObbMins = pCollideble->OBBMins( );
		const Vector vecObbMaxs = pCollideble->OBBMaxs( );
		const Vector vecObbCenter = ( vecObbMaxs + vecObbMins ) / 2.f;

		// calculate world space center
		const Vector vecPosition = vecObbCenter + pPlayer->m_vecOrigin( );

		const Vector vecTo = vecPosition - vecAbsStart;
		Vector vecDirection = vecAbsEnd - vecAbsStart;
		const float flLength = vecDirection.Normalize( );

		const float flRangeAlong = vecDirection.Dot( vecTo );
		float flRange = 0.0f;

		Vector on_ray;

		// calculate distance to ray
		// off start point.
		if (flRangeAlong < 0.f)
			flRange = -(vecTo).Length();

		// off end point.
		else if (flRangeAlong > flLength)
			flRange = -(vecPosition - vecAbsEnd).Length();

		// within ray bounds.
		else {
			on_ray = vecAbsStart + (vecDirection * flRangeAlong);
			flRange = (vecPosition - on_ray).Length();
		}

		if (/*range > 0.0f && */flRange <= 60.0f) {
			CGameTrace playerTrace;

			//ray.Init(start, end);

			Interfaces::m_pEngineTrace->ClipRayToEntity(Ray, 0x4600400B, pPlayer, &playerTrace);

			if (playerTrace.fraction < flSmallestFraction) {
				// we shortened the ray - save off the trace
				*pGameTrace = playerTrace;
				flSmallestFraction = playerTrace.fraction;
			}
		}
	}
}

bool Autowall::TraceToExit(CGameTrace* pEnterTrace, Vector vecStartPos, Vector vecDirection, CGameTrace* pExitTrace, Vector out) {
	Vector          new_end;
	float           dist = 0.0f;
	int				iterations = 23;
	int				first_contents = 0;
	int             contents;
	Ray_t r{};

	while (1)
	{
		iterations--;

		if (iterations <= 0 || dist > 90.f)
			break;

		dist += 4.0f;
		out = vecStartPos + (vecDirection * dist);

		contents = Interfaces::m_pEngineTrace->GetPointContents(out, 0x4600400B, nullptr);

		if (first_contents == -1)
			first_contents = contents;

		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
			continue;

		new_end = out - (vecDirection * 4.f);

		//r.Init(out, new_end);
		//CTraceFilter filter;
		//filter.pSkip = nullptr;
		//csgo.m_engine_trace()->TraceRay(r, MASK_SHOT, &filter, exit_trace);
		TraceLine(out, new_end, 0x4600400B, nullptr, pExitTrace);

		if (pExitTrace->startsolid && (pExitTrace->surface.flags & SURF_HITBOX) != 0)
		{
			//r.Init(out, start);
			//filter.pSkip = exit_trace->m_pEnt;
			//filter.pSkip = (exit_trace->m_pEnt);
			//csgo.m_engine_trace()->TraceRay(r, MASK_SHOT_HULL, &filter, exit_trace);
			TraceLine(out, vecStartPos, MASK_SHOT_HULL, (C_BasePlayer*)pExitTrace->hit_entity, pExitTrace);

			if (pExitTrace->DidHit() && !pExitTrace->startsolid)
			{
				out = pExitTrace->endpos;
				return true;
			}
			continue;
		}

		if (!pExitTrace->DidHit() || pExitTrace->startsolid)
		{
			if (pEnterTrace->hit_entity != Interfaces::m_pEntList->GetClientEntity(0)) {
				if (pExitTrace->hit_entity && pExitTrace->hit_entity->is_breakable())
				{
					pExitTrace->surface.surfaceProps = pEnterTrace->surface.surfaceProps;
					pExitTrace->endpos = vecStartPos + vecDirection;
					return true;
				}
			}

			continue;
		}

		if ((pExitTrace->surface.flags & 0x80u) != 0)
		{
			if (pEnterTrace->hit_entity && pEnterTrace->hit_entity->is_breakable()
				&& pExitTrace->hit_entity && pExitTrace->hit_entity->is_breakable())
			{
				out = pExitTrace->endpos;
				return true;
			}

			if (!(pEnterTrace->surface.flags & 0x80u))
				continue;
		}

		if (pExitTrace->plane.normal.Dot(vecDirection) <= 1.f) // exit nodraw is only valid if our entrace is also nodraw
		{
			out -= vecDirection * (pExitTrace->fraction * 4.0f);
			return true;
		}
	}

	return false;
}

bool Autowall::HandleBulletPenetration( Encrypted_t<C_FireBulletData> data ) {
	int iEnterMaterial = data->m_EnterSurfaceData->game.material;
	const int nPenetrationSystem = g_Vars.sv_penetration_type->GetInt( );

	bool bSolidSurf = ( ( data->m_EnterTrace.contents >> 3 ) & CONTENTS_SOLID );
	bool bLightSurf = ( ( data->m_EnterTrace.surface.flags >> 7 ) & SURF_LIGHT );
	bool bContentsGrate = data->m_EnterTrace.contents & CONTENTS_GRATE;
	bool bNoDrawSurf = !!( data->m_EnterTrace.surface.flags & ( SURF_NODRAW ) ); // this is valve code :D!

	// check if bullet can penetrarte another entity
	if( data->m_iPenetrationCount == 0 &&
		!bContentsGrate &&
		!bNoDrawSurf &&
		iEnterMaterial != CHAR_TEX_GRATE &&
		iEnterMaterial != CHAR_TEX_GLASS )
		return true; // no, stop

	 // if we hit a grate with iPenetration == 0, stop on the next thing we hit
	if( data->m_WeaponData->m_flPenetration <= 0.f || data->m_iPenetrationCount == 0 )
		return true;

	Vector end;

	// find exact penetration exit
	CGameTrace ExitTrace = { };
	if( !TraceToExit( &data->m_EnterTrace, data->m_EnterTrace.endpos, data->m_vecDirection, &ExitTrace, end ) ) {
		// ended in solid
		if( ( Interfaces::m_pEngineTrace->GetPointContents( end, 0x600400B ) & 0x600400B ) == 0 )
			return true;
	}

	const surfacedata_t* pExitSurfaceData = Interfaces::m_pPhysSurface->GetSurfaceData( ExitTrace.surface.surfaceProps );

	if( !pExitSurfaceData )
		return true;

	const float flEnterPenetrationModifier = data->m_EnterSurfaceData->game.flPenetrationModifier;
	const float flExitPenetrationModifier = pExitSurfaceData->game.flPenetrationModifier;
	const float flExitDamageModifier = pExitSurfaceData->game.flDamageModifier;

	const int iExitMaterial = pExitSurfaceData->game.material;

	float flDamageModifier = 0.f;
	float flPenetrationModifier = 0.f;

	// percent of total damage lost automatically on impacting a surface
	flDamageModifier = 0.16f;
	flPenetrationModifier = ( flEnterPenetrationModifier + flExitPenetrationModifier ) * 0.5f;

	// new penetration method
	if( nPenetrationSystem == 1 ) {
		// percent of total damage lost automatically on impacting a surface
		//flDamageModifier = 0.16f;
		//flPenetrationModifier = ( flEnterPenetrationModifier + flExitPenetrationModifier ) * 0.5f;

		//Are we using the newer penetration system?
		if (iEnterMaterial == CHAR_TEX_GLASS || iEnterMaterial == CHAR_TEX_GRATE) {
			flDamageModifier = 0.05f;
			flPenetrationModifier = 3;
		}
		else if (bSolidSurf || bLightSurf) {
			flDamageModifier = 0.16f;
			flPenetrationModifier = 1;
		}
		else if (iEnterMaterial == CHAR_TEX_FLESH && (data->m_Player->IsTeammate((C_CSPlayer*)(data->m_EnterTrace.hit_entity))) &&
			g_Vars.ff_damage_reduction_bullets->GetFloat() >= 0.f) {
			//Look's like you aren't shooting through your teammate today
			if (g_Vars.ff_damage_bullet_penetration->GetFloat() == 0.f)
				return true;

			//Let's shoot through teammates and get kicked for teamdmg! Whatever, atleast we did damage to the enemy. I call that a win.
			flPenetrationModifier = g_Vars.ff_damage_bullet_penetration->GetFloat();
		}
		else {
			flPenetrationModifier = (flEnterPenetrationModifier + flExitPenetrationModifier) / 2;
		}

		// if enter & exit point is wood we assume this is 
		// a hollow crate and give a penetration bonus
		if( iEnterMaterial == iExitMaterial ) {
			if( iExitMaterial == CHAR_TEX_WOOD || iExitMaterial == CHAR_TEX_CARDBOARD )
				flPenetrationModifier = 3.f;
			else if( iExitMaterial == CHAR_TEX_PLASTIC )
				flPenetrationModifier = 2.f;
		}

		// calculate damage  
		float flPenetrationLength = (ExitTrace.endpos - data->m_EnterTrace.endpos).Length();
		float flModifier = fmaxf(0, 1.0f / flPenetrationModifier);
		float flTakenFirst = (fmaxf(((3.0f / data->m_WeaponData->m_flPenetration) * 1.25f), 0.0f) * (flModifier * 3.0f) + (data->m_flCurrentDamage * flDamageModifier));
		float flTakenDamage = (((flPenetrationLength * flPenetrationLength) * flModifier) / 24.0f) + flTakenFirst;

		//Did we loose too much damage?
		if (fmaxf(0.0f, flTakenDamage) > data->m_flCurrentDamage)
			return true;

		data->m_flCurrentDamage -= fmaxf(0.0f, flTakenDamage);

		if (data->m_flCurrentDamage < 1.0f)
			return true;

		// penetration was successful
		// setup new start end parameters for successive trace
		data->m_vecStart = ExitTrace.endpos;
		--data->m_iPenetrationCount;
		return false;
	}
	else {
		// since some railings in de_inferno are CONTENTS_GRATE but CHAR_TEX_CONCRETE, we'll trust the
		// CONTENTS_GRATE and use a high damage modifier.
		if( bContentsGrate || bNoDrawSurf ) {
			// If we're a concrete grate (TOOLS/TOOLSINVISIBLE texture) allow more penetrating power.
			flPenetrationModifier = 1.0f;
			flDamageModifier = 0.99f;
		}
		else {
			if( flExitPenetrationModifier < flPenetrationModifier ) {
				flPenetrationModifier = flExitPenetrationModifier;
			}
			if( flExitDamageModifier < flDamageModifier ) {
				flDamageModifier = flExitDamageModifier;
			}
		}

		// if enter & exit point is wood or metal we assume this is 
		// a hollow crate or barrel and give a penetration bonus
		if( iEnterMaterial == iExitMaterial ) {
			if( iExitMaterial == CHAR_TEX_WOOD ||
				iExitMaterial == CHAR_TEX_METAL ) {
				flPenetrationModifier *= 2;
			}
		}

		float flTraceDistance = ( ExitTrace.endpos - data->m_EnterTrace.endpos ).Length( );

		// check if bullet has enough power to penetrate this distance for this material
		if( flTraceDistance > ( data->m_WeaponData->m_flPenetration * flPenetrationModifier ) )
			return true; // bullet hasn't enough power to penetrate this distance

		 // reduce damage power each time we hit something other than a grate
		data->m_flCurrentDamage *= flDamageModifier;

		// penetration was successful
		// setup new start end parameters for successive trace
		data->m_vecStart = ExitTrace.endpos;
		--data->m_iPenetrationCount;
		return false;
	}
}

float Autowall::FireBullets( Encrypted_t<C_FireBulletData> data ) {
	constexpr float rayExtension = 40.f;

	//This gets set in FX_Firebullets to 4 as a pass-through value.
	//CS:GO has a maximum of 4 surfaces a bullet can pass-through before it 100% stops.
	//Excerpt from Valve: https://steamcommunity.com/sharedfiles/filedetails/?id=275573090
	//"The total number of surfaces any bullet can penetrate in a single flight is capped at 4." -CS:GO Official

	if( !data->m_Weapon ) {
		data->m_Weapon = ( C_WeaponCSBaseGun* )( data->m_Player->m_hActiveWeapon( ).Get( ) );
		if( data->m_Weapon ) {
			data->m_WeaponData = data->m_Weapon->GetCSWeaponData( ).Xor( );
		}
	}

	data->m_flTraceLength = 0.f;
	data->m_flCurrentDamage = static_cast< float >( data->m_WeaponData->m_iWeaponDamage );

	CTraceFilter TraceFilter;
	TraceFilter.pSkip = data->m_Player;

	if( !data->m_Filter )
		data->m_Filter = &TraceFilter;

	data->m_flMaxLength = data->m_WeaponData->m_flWeaponRange;

	g_Vars.globals.m_InHBP = true;

	while( data->m_flCurrentDamage > 0.f ) {
		// calculate max bullet range
		data->m_flMaxLength -= data->m_flTraceLength;

		// create end point of bullet
		Vector vecEnd = data->m_vecStart + data->m_vecDirection * data->m_flMaxLength;

		TraceLine( data->m_vecStart, vecEnd, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_Player, &data->m_EnterTrace );

		// create extended end point
		Vector vecEndExtended = vecEnd + data->m_vecDirection * rayExtension;

		// NOTICE: can remove valve`s hack aka bounding box fix
		// Check for player hitboxes extending outside their collision bounds
		if( data->m_TargetPlayer ) {
			// clip trace to one player
			ClipTraceToPlayer( data->m_vecStart, vecEndExtended, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_Filter, &data->m_EnterTrace, data );
		}
		else {
			ClipTraceToPlayers( data->m_vecStart, vecEndExtended, MASK_SHOT_HULL | CONTENTS_HITBOX, data->m_Filter, &data->m_EnterTrace );
		}

		//We have to do this *after* tracing to the player.
		surfacedata_t* enterSurfaceData = Interfaces::m_pPhysSurface->GetSurfaceData(data->m_EnterTrace.surface.surfaceProps);
		int enterMaterial = enterSurfaceData->game.material;

		if( data->m_EnterTrace.fraction == 1.0f )
			break;  // we didn't hit anything, stop tracing shoot

		 //calculate the damage based on the distance the bullet traveled.
		data->m_flTraceLength += data->m_EnterTrace.fraction * data->m_flMaxLength;

		//Let's make our damage drops off the further away the bullet is.
		if( !data->m_bShouldIgnoreDistance )
			data->m_flCurrentDamage *= powf( data->m_WeaponData->m_flRangeModifier, (data->m_flTraceLength / 500) );

		if (!(data->m_EnterTrace.contents & CONTENTS_HITBOX))
			data->m_EnterTrace.hitgroup = 1;

		C_CSPlayer* pHittedPlayer = ToCSPlayer( ( C_BasePlayer* )data->m_EnterTrace.hit_entity );

#ifdef DEBUG_AUTOWALL
		if( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
			Interfaces::m_pDebugOverlay->AddLineOverlay( data->m_vecStart, data->m_EnterTrace.endpos, 255, 255, 255, true, DEBUG_DURATION );
			// Interfaces::m_pDebugOverlay->AddBoxOverlay( data->m_EnterTrace.endpos, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 255, 0, 0, 200, DEBUG_DURATION );
		}
#endif

		const int nHitGroup = data->m_EnterTrace.hitgroup;
		const bool bHitgroupIsValid = data->m_Weapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS ? ( nHitGroup >= Hitgroup_Generic && nHitGroup < Hitgroup_Neck ) : ( nHitGroup >= Hitgroup_Generic && nHitGroup < Hitgroup_Neck );
		const bool bTargetIsValid = !data->m_TargetPlayer || ( pHittedPlayer != nullptr && pHittedPlayer->m_entIndex == data->m_TargetPlayer->m_entIndex );
		if( pHittedPlayer != nullptr ) {
			if( bTargetIsValid && bHitgroupIsValid && pHittedPlayer->IsPlayer( ) && pHittedPlayer->m_entIndex <= Interfaces::m_pGlobalVars->maxClients && pHittedPlayer->m_entIndex > 0 ) {
				data->m_flCurrentDamage = ScaleDamage( pHittedPlayer, data->m_flCurrentDamage, data->m_WeaponData->m_flArmorRatio, data->m_Weapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS ? Hitgroup_Generic : nHitGroup );
				data->m_iHitgroup = nHitGroup;

				// very big brain
				if (data->m_flCurrentDamage < 1.f)
					return 0;


				g_Vars.globals.m_InHBP = false;
				return data->m_flCurrentDamage;
			} 
		}

		bool bCanPenetrate = data->m_bPenetration;
		//if( !data->m_bPenetration )
		//	bCanPenetrate = data->m_EnterTrace.contents & CONTENTS_WINDOW;

		if( !bCanPenetrate )
			break;

		data->m_EnterSurfaceData = Interfaces::m_pPhysSurface->GetSurfaceData( data->m_EnterTrace.surface.surfaceProps );

		if( !data->m_EnterSurfaceData )
			break;

		// check if we reach penetration distance, no more penetrations after that
		// or if our modifier is super low, just stop the bullet
		if( ( data->m_flTraceLength > 3000.f && data->m_WeaponData->m_flPenetration > 0.f ) || // was 3150 (antipaste)
			data->m_EnterSurfaceData->game.flPenetrationModifier < 0.1f ) {
			data->m_iPenetrationCount = 0;
			break;
		}

		bool bIsBulletStopped = HandleBulletPenetration( data );

		if( bIsBulletStopped )
			break;

#ifdef DEBUG_AUTOWALL
		if( InputSys::Get( )->IsKeyDown( VirtualKeys::H ) ) {
			Interfaces::m_pDebugOverlay->AddLineOverlay( data->m_vecStart, data->m_EnterTrace.endpos, 0, 0, 255, true, DEBUG_DURATION );
			Interfaces::m_pDebugOverlay->AddBoxOverlay( data->m_vecStart, Vector( -1.0f, -1.0f, -1.0f ), Vector( 1.0f, 1.0f, 1.0f ), QAngle( 0.0f, 0.0f, 0.0f ), 0, 255, 0, 200, DEBUG_DURATION );
			Interfaces::m_pDebugOverlay->AddTextOverlay( data->m_vecStart - Vector( 0.0f, 0.0f, 5.0f ), DEBUG_DURATION, XorStr( "^ %.2f damage\t %d pen count\t %.2f thickness" ), data->m_flCurrentDamage, data->m_iPenetrationCount, data->m_flPenetrationDistance );
	}
#endif
}

	g_Vars.globals.m_InHBP = false;
	return -1.f;
}
