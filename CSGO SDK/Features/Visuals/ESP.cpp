#include "Esp.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/CVariables.hpp"
#include "../../source.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../Renderer/Render.hpp"
#include "../Rage/Autowall.h"
#include "Hitmarker.hpp"
#include "../../SDK/Classes/PropManager.hpp"
#include "../Rage/LagCompensation.hpp"
#include "EventLogger.hpp"
#include "../../Utils/InputSys.hpp"
#include "../Rage/Ragebot.hpp"
#include <minwindef.h>
#include "../../Utils/Math.h"
#include "../../SDK/RayTracer.h"
#include "../../SDK/displacement.hpp"
#include "CChams.hpp"
#include "../Rage/TickbaseShift.hpp"
#include "ExtendedEsp.hpp"
#include <sstream>
#include <ctime>
#include <Psapi.h>
#include "../../Utils/tinyformat.h"
//#include "../Rage/AnimationSystem.hpp"

#include "../Miscellaneous/Movement.hpp"
#include "IVEffects.h"
#include "Trace.h"
#include "../Miscellaneous/walkbot.h"
#include "gh.h"
#include "../Miscellaneous/MusicPlayer.hpp"
#include "../../ShittierMenu/ImGuiRenderer.h"

#include <iomanip>

extern Vector AutoPeekPos;

struct BBox_t {
	int x, y, w, h;
};

class CEsp : public IEsp {
public:
	void PenetrateCrosshair( Vector2D center );
	void DrawAntiAimIndicator( ) override;
	void DrawZeusDistance( );
	void IndicateAngles();
	void SpreadCrosshair();
	void RenderImGuiNades() override;
	void Main( ) override;
	void SetAlpha( int idx ) override;
	float GetAlpha( int idx ) override;
	void AddSkeletonMatrix( C_CSPlayer* player, matrix3x4_t* bones ) override;
private:
	struct IndicatorsInfo_t {
		IndicatorsInfo_t( ) {

		}

		IndicatorsInfo_t( const char* m_szName,
			int m_iPrioirity,
			bool m_bLoading,
			float m_flLoading,
			FloatColor m_Color ) {
			this->m_szName = m_szName;
			this->m_iPrioirity = m_iPrioirity;
			this->m_bLoading = m_bLoading;
			this->m_flLoading = m_flLoading;
			this->m_Color = m_Color;
		}

		const char* m_szName = "";
		int m_iPrioirity = -1;
		bool m_bLoading = false;
		float m_flLoading = 0.f;
		FloatColor m_Color = FloatColor( 0, 0, 0, 255 );
	};

	std::vector< IndicatorsInfo_t > m_vecTextIndicators;

	struct EspData_t {
		C_CSPlayer* player;
		bool bEnemy;
		Vector2D head_pos;
		Vector2D feet_pos;
		Vector origin;
		BBox_t bbox;
		player_info_t info;
	};

	EspData_t m_Data;
	C_CSPlayer* m_LocalPlayer = nullptr;
	C_CSPlayer* m_LocalObserved = nullptr;

	int storedTick = 0;
	int crouchedTicks[ 65 ];
	float m_flAlpha[ 65 ];

	float lastTime = 0.0f;
	int oldframecount = 0;
	int curfps = 0;

	bool m_bAlphaFix[ 65 ];
	bool Begin( C_CSPlayer* player );
	bool ValidPlayer( C_CSPlayer* player );
	void AmmoBar( C_CSPlayer* player, BBox_t bbox );
	void RenderNades( C_WeaponCSBaseGun* nade );
	void DrawBox( BBox_t bbox, const FloatColor& clr, C_CSPlayer* player );
	void DrawHealthBar( C_CSPlayer* player, BBox_t bbox );
	void DrawInfo( C_CSPlayer* player, BBox_t bbox, player_info_t player_info );
	void DrawBottomInfo( C_CSPlayer* player, BBox_t bbox, player_info_t player_info );
	void DrawName( C_CSPlayer* player, BBox_t bbox, player_info_t player_info );
	void DrawTeamName( C_CSPlayer* player, BBox_t bbox, player_info_t player_info );
	void DrawSkeleton( C_CSPlayer* player );
	void DrawHitSkeleton( );
	void DrawLocalSkeleton( );
	bool GetBBox( C_BaseEntity* player, Vector2D screen_points[ ], BBox_t& outRect );
	void Offscreen( );
	void OverlayInfo( );
	void Indicators( );
	void BloomEffect( );
	bool IsFakeDucking( C_CSPlayer* player ) {
		if( !player )
			return false;

		float duckamount = player->m_flDuckAmount( );
		if( !duckamount ) {
			crouchedTicks[ player->EntIndex( ) ] = 0;
			return false;
		}

		float duckspeed = player->m_flDuckSpeed( );
		if( !duckspeed ) {
			crouchedTicks[ player->EntIndex( ) ] = 0;
			return false;
		}

		if( storedTick != Interfaces::m_pGlobalVars->tickcount ) {
			crouchedTicks[ player->EntIndex( ) ]++;
			storedTick = Interfaces::m_pGlobalVars->tickcount;
		}

		if( int( duckspeed ) == 8 && duckamount <= 0.9f && duckamount > 0.01
			&& ( player->m_fFlags( ) & FL_ONGROUND ) && ( crouchedTicks[ player->EntIndex( ) ] >= 5 ) )
			return true;
		else
			return false;
	};

	C_Window m_KeyBinds = { Vector2D( g_Vars.esp.keybind_window_x, g_Vars.esp.keybind_window_y ), Vector2D( 180, 10 ), 0 };
	//C_Window m_SpecList = { Vector2D( g_Vars.esp.spec_window_x, g_Vars.esp.spec_window_y ), Vector2D( 180, 10 ), 1 };

	//void SpectatorList( bool window = false );
	void Keybinds( );

	struct C_HitMatrixEntry {
		float m_flTime = 0.0f;
		float m_flAlpha = 0.0f;

		C_CSPlayer* m_pEntity = nullptr;
		matrix3x4_t pBoneToWorld[ 128 ] = { };
	};

	std::vector<C_HitMatrixEntry> m_Hitmatrix;
};

struct dlight_p {
	dlight_t* light;
};


void CEsp::AddSkeletonMatrix( C_CSPlayer* player, matrix3x4_t* bones ) {
	if( !player || !bones )
		return;

	C_HitMatrixEntry info;

	info.m_flTime = Interfaces::m_pGlobalVars->realtime + g_Vars.esp.hitskeleton_time;
	info.m_flAlpha = 1.f;
	info.m_pEntity = player;
	std::memcpy( info.pBoneToWorld, bones, player->m_CachedBoneData( ).Count( ) * sizeof( matrix3x4_t ) );

	m_Hitmatrix.push_back( info );
}

void CEsp::BloomEffect( ) {
	static bool props = false;

	static ConVar* r_modelAmbientMin = Interfaces::m_pCvar->FindVar( XorStr( "r_modelAmbientMin" ) );

	for( int i = 0; i < Interfaces::m_pEntList->GetHighestEntityIndex( ); i++ ) {
		C_BaseEntity* pEntity = ( C_BaseEntity* )Interfaces::m_pEntList->GetClientEntity( i );

		if( !pEntity )
			continue;

		if( pEntity->GetClientClass( )->m_ClassID == 69 ) {
			auto pToneMap = ( CEnvTonemapContorller* )pEntity;
			if( pToneMap ) {
				*pToneMap->m_bUseCustomAutoExposureMin( ) = true;
				*pToneMap->m_bUseCustomAutoExposureMax( ) = true;

				*pToneMap->m_flCustomAutoExposureMin( ) = 0.2f;
				*pToneMap->m_flCustomAutoExposureMax( ) = 0.2f;
				*pToneMap->m_flCustomBloomScale( ) = 10.1f;

				r_modelAmbientMin->SetValue( g_Vars.esp.model_brightness );
			}
		}

		if( pEntity->GetClientClass( )->m_ClassID == CPrecipitation ) {
			auto pToneMap = pEntity;
			if( pToneMap ) {

			}
		}
	}
}

void DrawWatermark() {
	if (!g_Vars.misc.watermark /*|| !g_IMGUIMenu.Loaded*/) {
		return;
	}

	// constants for colors.
	// Constants for padding, etc...
	const auto margin = 10; // Padding between screen edges and watermark
	const auto padding = 4; // Padding between watermark elements

	// Constants for colors
	const auto col_background = Color(41, 32, 59, 175); // Watermark background color
	FloatColor col_accent; // Watermark line accent color

	if (!g_Vars.misc.custom_menu) {
		col_accent = FloatColor(255, 0, 0, 255);
	}
	else {
		col_accent = g_Vars.misc.accent_colorz;
	}

	const auto col_text = Color(255, 255, 255); // Watermark text color

	static auto framerate = 0.0f;
	framerate = framerate * 0.9f + Interfaces::m_pGlobalVars->frametime * 0.1f;
	auto fps = (int)(1.0f / framerate, 0.0f, 999.0f);


	// Cheat variables
	std::string logo = XorStr("vader.tech");
#ifdef DEV
	logo.append(XorStr(" [debug]")); // :)
#endif
#ifdef BETA_MODE
	logo.append(XorStr(" [beta]")); // :)
#endif
	const std::string user = g_Vars.globals.c_username; /*g_cl.m_user*/

	std::string text = logo + XorStr(" | ") + user;

	if (Interfaces::m_pEngine->IsInGame())
	{
		auto netchannel = Encrypted_t<INetChannel>(Interfaces::m_pEngine->GetNetChannelInfo());
		if (!netchannel.IsValid())
			return;

		// Game variables
		auto delay = std::max(0, (int)std::round(netchannel->GetLatency(FLOW_OUTGOING) * 1000.f));
		static auto framerate = 0.0f;
		framerate = framerate * 0.9f + Interfaces::m_pGlobalVars->frametime * 0.1f;
		auto fps = (int)(1.0f / framerate);

		text = logo + XorStr(" | ") + user + XorStr(" | latency: ") + std::to_string(delay) + XorStr(" ms | ") + std::to_string(fps) + XorStr(" fps");

    }

	Vector2D screen = Render::GetScreenSize();

	// Calculating text size and position
	auto text_size = Render::Engine::tahoma_sexy.size(text);
	auto text_pos = Vector2D(screen.x - margin - padding - text_size.m_width, // Right align + margin + padding + text_size
		margin + padding); // Top align

	// Calculating watermark background size and position
	auto bg_size = Vector2D(padding + text_size.m_width + padding, // Width
		padding + text_size.m_height + padding); // Height
	auto bg_pos = Vector2D(screen.x - margin - padding - text_size.m_width - padding, // Right align + margin
		margin); // Top align

	Render::Engine::RectFilled(bg_pos.x, bg_pos.y, bg_size.x, bg_size.y, col_background); // Background
	Render::Engine::Rect(bg_pos.x, bg_pos.y, bg_size.x, 2, col_accent.ToRegularColor()); // Accent line
	Render::Engine::tahoma_sexy.string(text_pos.x, text_pos.y, col_text, text); // Text
}

void circle_above_head(C_CSPlayer* e)
{
	if (!g_Vars.esp.halo_above_head || !e || !e->IsAlive() || !g_Vars.misc.third_person_bind.enabled || !g_Vars.misc.third_person)
		return;

	auto bone_pos = e->GetHitboxPosition(HITBOX_HEAD);
	Vector2D angle;
	if (Render::Engine::WorldToScreen(bone_pos, angle))
	{
		bone_pos.z = bone_pos.z + 10;
		Render::Engine::WorldCircle(bone_pos, 5, g_Vars.esp.halo_above_head_color.ToRegularColor(), Color(0, 0, 0, 0));
	}


}

void dlight_players()
{
	C_CSPlayer* local = C_CSPlayer::GetLocalPlayer();

	if (!local)
		return;

	if (local->IsAlive() && g_Vars.esp.dlight_local_enable) {
		static auto DLight_local = Interfaces::g_IVEffects->CL_AllocDlight(local->EntIndex());
		if (DLight_local) {

			DLight_local->origin = local->m_vecOrigin();
			DLight_local->flags = 0x2;
			DLight_local->style = 5;
			DLight_local->key = local->EntIndex();
			DLight_local->die = Interfaces::m_pGlobalVars->curtime + 60.f;
			ColorRGBExp32 color;
			color.r = g_Vars.esp.dlight_local_color.ToRegularColor().r();
			color.g = g_Vars.esp.dlight_local_color.ToRegularColor().g();
			color.b = g_Vars.esp.dlight_local_color.ToRegularColor().b();
			color.exponent = (g_Vars.esp.dlight_local_color.ToRegularColor().a() / 255) * 5;
			DLight_local->radius = g_Vars.esp.dlight_local_radius;
			DLight_local->decay = 300;
			DLight_local->m_Direction = local->m_vecOrigin();
			DLight_local->color = color;

		}

	}

	if (g_Vars.esp.dlight_enemy_enable) {
		static dlight_p DLights[65];
		for (int i = 0; i < Interfaces::m_pEngine->GetMaxClients(); i++) {
			C_CSPlayer* cplayer = (C_CSPlayer *)Interfaces::m_pEntList->GetClientEntity(i);
			if (!cplayer || !cplayer->IsAlive() || cplayer->IsDormant() || cplayer == local || cplayer->m_iTeamNum() == local->m_iTeamNum())
				continue;

			if (!DLights[i].light)
				DLights[i].light = Interfaces::g_IVEffects->CL_AllocDlight(i);

			DLights[i].light->origin = cplayer->m_vecOrigin();
			DLights[i].light->flags = 0x2;
			DLights[i].light->style = 5;
			DLights[i].light->key = cplayer->EntIndex();
			DLights[i].light->die = Interfaces::m_pGlobalVars->curtime + 60.f;
			ColorRGBExp32 color;
			color.r = g_Vars.esp.dlight_enemy_color.ToRegularColor().r();
			color.g = g_Vars.esp.dlight_enemy_color.ToRegularColor().g();
			color.b = g_Vars.esp.dlight_enemy_color.ToRegularColor().b();
			color.exponent = (g_Vars.esp.dlight_enemy_color.ToRegularColor().a() / 255) * 5;
			DLights[i].light->radius = g_Vars.esp.dlight_enemy_radius;
			DLights[i].light->decay = 300;
			DLights[i].light->m_Direction = cplayer->m_vecOrigin();
			DLights[i].light->color = color;
		}
	}
}


bool CEsp::Begin( C_CSPlayer* player ) {
	m_Data.player = player;
	m_Data.bEnemy = player->m_iTeamNum( ) != m_LocalPlayer->m_iTeamNum( );
	m_LocalObserved = ( C_CSPlayer* )m_LocalPlayer->m_hObserverTarget( ).Get( );

	player_info_t player_info;
	if( !Interfaces::m_pEngine->GetPlayerInfo( player->EntIndex( ), &player_info ) )
		return false;

	m_Data.info = player_info;

	//if( !m_Data.bEnemy )
	//	return false;

	Vector2D points[ 8 ];
	return GetBBox( player, points, m_Data.bbox );
}

bool CEsp::ValidPlayer( C_CSPlayer* player ) {
	if( !player )
		return false;

	int idx = player->EntIndex( );

	if( player->IsDead( ) ) {
		m_flAlpha[ idx ] = 0.f;
		return false;
	}

	constexpr int EXPIRE_DURATION = 450; // miliseconds-ish?
	auto& sound_player = IExtendedEsp::Get()->m_cSoundPlayers[player->EntIndex()];
	bool sound_expired = GetTickCount() - sound_player.m_iReceiveTime > EXPIRE_DURATION;

	static auto g_GameRules = *( uintptr_t** )( Engine::Displacement.Data.m_GameRules );
	if( *( bool* )( *( uintptr_t* )g_GameRules + 0x20 ) ) {
		if( player->IsDormant( ) && sound_expired ) {
			m_flAlpha[ idx ] = 0.f;
		}
		return false;
	}
	if( player->IsDormant( ) && sound_expired ) {
		if( m_flAlpha[ idx ] < 0.6f ) {
			m_flAlpha[ idx ] -= ( 1.0f / 1.0f ) * Interfaces::m_pGlobalVars->frametime;
			m_flAlpha[ idx ] = std::clamp( m_flAlpha[ idx ], 0.f, 0.6f );
		}
		else {
			m_flAlpha[ idx ] -= ( 1.0f / 20.f ) * Interfaces::m_pGlobalVars->frametime;
		}
	}
	else {
		m_flAlpha[ idx ] += ( 1.0f / 0.2f ) * Interfaces::m_pGlobalVars->frametime;
		m_flAlpha[ idx ] = std::clamp( m_flAlpha[ idx ], 0.f, 1.f );
	}

	return ( m_flAlpha[ idx ] > 0.f );
}

int fps( ) {
	static float m_Framerate = 0.f;

	// Move rolling average
	m_Framerate = 0.9 * m_Framerate + ( 1.0 - 0.9 ) * Interfaces::m_pGlobalVars->absoluteframetime;

	if( m_Framerate <= 0.0f )
		m_Framerate = 1.0f;

	return ( int )( 1.0f / m_Framerate );
}

CGrenade GrenadeClass;

void Grenade_Tracer() {
	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();

	if (!pLocal || !g_Vars.esp.NadeTracer)
		return;

	for (int i = 1; i < Interfaces::m_pEntList->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity* pBaseEntity = static_cast<C_BaseEntity*>(Interfaces::m_pEntList->GetClientEntity(i));
		if (!pBaseEntity)
			continue;

		C_CSPlayer* player = (C_CSPlayer*)pBaseEntity->m_hOwnerEntity().Get();
		if(!player)
			continue;

		if (player->EntIndex() != pLocal->EntIndex())
			continue;

		const auto client_class = pBaseEntity->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CMolotovProjectile) {

			if (client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CMolotovProjectile)
			{

				if (GrenadeClass.checkGrenades(pBaseEntity)) {
					Grenade_t grenade;
					grenade.entity = pBaseEntity;
					grenade.addTime = Interfaces::m_pGlobalVars->realtime;

					GrenadeClass.addGrenade(grenade);
				}
				else
				{
					GrenadeClass.updatePosition(pBaseEntity, pBaseEntity->m_vecOrigin());
				}


			}

		}

	}

}

void CEsp::Indicators() {
	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	if (auto pLocal = C_CSPlayer::GetLocalPlayer(); pLocal) {
		if (g_Vars.esp.indicator_ping) {
			if (g_Vars.misc.extended_backtrack_key.enabled && g_Vars.misc.extended_backtrack) {
				Indicator_t ind{ };
				ind.color = Color(123, 194, 21);
				ind.text = XorStr("PING");

				indicators.push_back(ind);
			}
		}

		if (g_Vars.esp.indicator_lby) {
			if (g_Vars.antiaim.enabled && (pLocal->m_vecVelocity().Length2D() <= 0.1f || g_Vars.globals.Fakewalking) && !((g_Vars.antiaim.desync_on_dt && g_TickbaseController.s_nExtraProcessingTicks > 0) || (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled))) {
				Indicator_t ind{ };
				// get the absolute change between current lby and animated angle.
				float change = std::abs(Math::NormalizedAngle(g_Vars.globals.m_flBody - g_Vars.globals.RegularAngles.y));
				ind.color = change > 35.f ? Color(123, 194, 21) : Color(255, 0, 0);
				ind.text = XorStr("LBY");

				indicators.push_back(ind);
			}
		}

		if (g_Vars.esp.indicator_lagcomp) {
			if (pLocal->m_vecVelocity().Length2D() > 270.f || g_Vars.globals.bBrokeLC) {
				Indicator_t ind{ };
				if (g_Vars.rage.dt_exploits && g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit_lagcomp) {
					ind.color = Color(123, 194, 21);
				}
				else
					ind.color = g_Vars.globals.bBrokeLC ? Color(123, 194, 21) : Color(255, 0, 0);

				ind.text = XorStr("LC");

				indicators.push_back(ind);
			}
		}

		if (g_Vars.esp.indicator_exploits) {
			if (g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled) {
				Indicator_t ind{ };
				ind.color = g_TickbaseController.s_nExtraProcessingTicks > 0 ? Color(220, 220, 220) : Color(255, 0, 0);
				ind.text = XorStr("DT");

				indicators.push_back(ind);
			}
		}

		if (g_Vars.esp.indicator_mindmg) {
			if (g_Vars.rage.key_dmg_override.enabled && g_Vars.globals.OverridingMinDmg) {
				Indicator_t ind{ };
				ind.color = Color(220, 220, 220);
				ind.text = XorStr("DMG: ") + std::to_string(g_Vars.globals.OverrideDmgAmount);

				indicators.push_back(ind);
			}
		}
	}

	if (indicators.empty())
		return;

	// iterate and draw indicators.
	for (size_t i{ }; i < indicators.size(); ++i) {
		auto& indicator = indicators[i];
		auto TextSize = Render::Engine::indi.size(indicator.text);

		Render::Engine::Gradient(19.f, Render::GetScreenSize().y - 376.f - (30 * i), TextSize.m_width / 2, 23, Color(0, 0, 0, 0), Color(0, 0, 0, 127), true);
		Render::Engine::Gradient(19.f + TextSize.m_width / 2, Render::GetScreenSize().y - 376.f - (30 * i), TextSize.m_width / 2, 23, Color(0, 0, 0, 127), Color(0, 0, 0, 0), true);
		Render::Engine::indi.string(21.f, Render::GetScreenSize().y - 380.f - (30 * i) + 1, Color(0, 0, 0, 200), indicator.text); // text shadow
		Render::Engine::indi.string(20.f, Render::GetScreenSize().y - 380.f - (30 * i), indicator.color, indicator.text);
	}


	//auto pLocal = C_CSPlayer::GetLocalPlayer();

	//static float next_lby_update[65];

	//const float curtime = Interfaces::m_pGlobalVars->curtime;

	//if (pLocal->m_vecVelocity().Length2D() > 0.1f && !g_Vars.globals.Fakewalking)
	//	return;

	//CCSGOPlayerAnimState* state = pLocal->m_PlayerAnimState();
	//if (!state)
	//	return;

	//static float last_lby[65];
	//if (last_lby[pLocal->EntIndex()] != pLocal->m_flLowerBodyYawTarget())
	//{
	//	last_lby[pLocal->EntIndex()] = pLocal->m_flLowerBodyYawTarget();
	//	next_lby_update[pLocal->EntIndex()] = curtime + 1.125f + Interfaces::m_pGlobalVars->interval_per_tick;
	//}

	//if (next_lby_update[pLocal->EntIndex()] < curtime)
	//{
	//	next_lby_update[pLocal->EntIndex()] = curtime + 1.125f;
	//}

	//float time_remain_to_update = next_lby_update[pLocal->EntIndex()] - pLocal->m_flSimulationTime();
	//float time_update = next_lby_update[pLocal->EntIndex()];

	//float fill = 0;
	//fill = (((time_remain_to_update)));
	//static float add = 0.000f;
	//add = 1.125f - fill;

	//float change1337 = std::abs(Math::AngleNormalize(g_Vars.globals.m_flBody - g_Vars.globals.RegularAngles.y));

	//Color color1337 = {  };

	//if (change1337 > 35.f) {
	//	color1337 = { 124,195,13,255 }; // green color
	//}

	//Render::Engine::RectFilled(13, Render::GetScreenSize().y - 74 + 26, 48, 4, { 10, 10, 10, 125 });
	//Render::Engine::RectFilled(13, Render::GetScreenSize().y - 74 + 26, add * 40, 2, color1337);
}

//void CEsp::SpectatorList( bool window ) {
//	std::vector< std::string > spectators{ };
//	int h = Render::Engine::hud.m_size.m_height;
//	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
//
//	if( window )
//		this->m_SpecList.size.y = 20.0f;
//
//	if( Interfaces::m_pEngine->IsInGame( ) && pLocal ) {
//		const auto local_observer = pLocal->m_hObserverTarget( );
//		for( int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i ) {
//			C_CSPlayer* player = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntity( i );
//			if( !player )
//				continue;
//
//			if( !player->IsDead( ) )
//				continue;
//
//			if( player->IsDormant( ) )
//				continue;
//
//			if( player->EntIndex( ) == pLocal->EntIndex( ) )
//				continue;
//
//			player_info_t info;
//			if( !Interfaces::m_pEngine->GetPlayerInfo( i, &info ) )
//				continue;
//
//			if( pLocal->IsDead( ) ) {
//				auto observer = player->m_hObserverTarget( );
//				if( local_observer.IsValid( ) && observer.IsValid( ) ) {
//					const auto spec = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntityFromHandle( local_observer );
//					auto target = reinterpret_cast< C_CSPlayer* >( Interfaces::m_pEntList->GetClientEntityFromHandle( observer ) );
//
//					if( target == spec && spec ) {
//						spectators.push_back( std::string( info.szName ).substr( 0, 24 ) );
//					}
//				}
//			}
//			else {
//				if( player->m_hObserverTarget( ) != pLocal )
//					continue;
//
//				spectators.push_back( std::string( info.szName ).substr( 0, 24 ) );
//			}
//		}
//	}
//
//	if( !window ) {
//		if( spectators.empty( ) )
//			return;
//
//		size_t total_size = spectators.size( ) * ( h - 1 );
//
//		for( size_t i{ }; i < spectators.size( ); ++i ) {
//			const std::string& name = spectators[ i ];
//
//			Render::Engine::hud.string( Render::GetScreenSize( ).x - 10, ( Render::GetScreenSize( ).y / 2 ) - ( total_size / 2 ) + ( i * ( h - 1 ) ),
//				{ 255, 255, 255, 179 }, name, Render::Engine::ALIGN_RIGHT );
//		}
//	}
//	else {
//		this->m_SpecList.Drag( );
//		Vector2D pos = { g_Vars.esp.spec_window_x, g_Vars.esp.spec_window_y };
//
//		static float alpha = 0.f;
//		bool condition = !( spectators.empty( ) && !g_Vars.globals.menuOpen );
//		float multiplier = static_cast< float >( ( 1.0f / 0.05f ) * Interfaces::m_pGlobalVars->frametime );
//		if( condition && ( !pLocal ? true : pLocal->m_iObserverMode( ) != 6 ) ) {
//			alpha += multiplier * ( 1.0f - alpha );
//		}
//		else {
//			if( alpha > 0.01f )
//				alpha += multiplier * ( 0.0f - alpha );
//			else
//				alpha = 0.0f;
//		}
//
//		alpha = std::clamp( alpha, 0.f, 1.0f );
//
//		if( alpha <= 0.f )
//			return;
//
//		Color main = Color( 39, 41, 54, 150 * alpha );
//
//		// header
//		Render::Engine::RectFilled( pos, this->m_SpecList.size, Color( 39, 41, 54, 220 * alpha ) );
//
//		Color accent_color = g_Vars.menu.ascent.ToRegularColor( );
//		accent_color.RGBA[ 3 ] *= alpha;
//
//		// line splitting
//		Render::Engine::Line( pos + Vector2D( 0, this->m_SpecList.size.y ), pos + this->m_SpecList.size, accent_color );
//		Render::Engine::Line( pos + Vector2D( 0, this->m_SpecList.size.y + 1 ), pos + Vector2D( this->m_SpecList.size.x, this->m_SpecList.size.y + 1 ), accent_color );
//
//		for( size_t i{ }; i < spectators.size( ); ++i ) {
//			if( i > 0 )
//				this->m_SpecList.size.y += 13.0f;
//		}
//
//		// the actual window
//		Render::Engine::RectFilled( pos + Vector2D( 0, 20 + 2 ), Vector2D( this->m_SpecList.size.x, this->m_SpecList.size.y - 1 ), main );
//
//		// title
//		auto size = Render::Engine::segoe.size( XorStr( "Spectators" ) );
//		Render::Engine::segoe.string( pos.x + ( this->m_SpecList.size.x * 0.5 ) - 2, pos.y + ( size.m_height * 0.5 ) - 4, Color::White( ).OverrideAlpha( 255 * alpha ), XorStr( "Spectators" ), Render::Engine::ALIGN_CENTER );
//
//		if( spectators.empty( ) )
//			return;
//
//		float offset = 14.0f;
//		for( size_t i{ }; i < spectators.size( ); ++i ) {
//			const std::string& name = spectators[ i ];
//
//			// name
//			Render::Engine::segoe.string( pos.x + 2, pos.y + 10 + offset, Color::White( ).OverrideAlpha( 255 * alpha ), name );
//
//			offset += 13.0f;
//		}
//	}
//}

void CEsp::Keybinds() {
	std::vector<
		std::pair<std::string, int>
	> vecNames;

	this->m_KeyBinds.Drag();

	Vector2D pos = { g_Vars.esp.keybind_window_x, g_Vars.esp.keybind_window_y };

	this->m_KeyBinds.size.y = 20.0f;

	auto AddBind = [this, &vecNames](const char* name, KeyBind_t& bind) {
		if (!bind.enabled)
			return;

		if (!vecNames.empty())
			this->m_KeyBinds.size.y += 13.0f;

		vecNames.push_back(std::pair<std::string, int>(std::string(name), bind.cond));
	};

	if (g_Vars.rage.enabled) {
		if (g_Vars.rage.exploit) {
			AddBind(XorStr("Double tap"), g_Vars.rage.key_dt);
		}

		if (g_Vars.misc.fakeduck) {
			AddBind(XorStr("Fake-Duck"), g_Vars.misc.fakeduck_bind);
		}

		if (g_Vars.misc.move_exploit) {
			AddBind(XorStr("Move Exploit"), g_Vars.misc.move_exploit_key);
		}

		//AddBind( XorStr( "Force safe point" ), g_Vars.rage.force_safe_point );
		AddBind(XorStr("Min dmg override"), g_Vars.rage.key_dmg_override);
		AddBind(XorStr("Force body-aim"), g_Vars.rage.prefer_body);
	}

	//if( g_Vars.misc.instant_stop ) {
	//	AddBind( XorStr( "Instant stop in air" ), g_Vars.misc.instant_stop_key );
	//}

	//AddBind( XorStr( "Anti-aim invert" ), g_Vars.antiaim.desync_flip_bind );
	//AddBind( XorStr( "Thirdperson" ), g_Vars.misc.third_person_bind );
	//AddBind( XorStr( "Desync jitter" ), g_Vars.antiaim.desync_jitter_key );
	AddBind(XorStr("Hitscan override"), g_Vars.rage.override_key);

	if (g_Vars.misc.edgejump) {
		AddBind(XorStr("Edge jump"), g_Vars.misc.edgejump_bind);
	}

	if (g_Vars.misc.autopeek) {
		AddBind(XorStr("Auto-Peek"), g_Vars.misc.autopeek_bind);
	}

	if (g_Vars.misc.slow_walk) {
		AddBind(XorStr("Fake-walk"), g_Vars.misc.slow_walk_bind);
	}

	if (g_Vars.misc.extended_backtrack) {
		AddBind(XorStr("Ping-spike"), g_Vars.misc.extended_backtrack_key);
	}

	if (g_Vars.misc.mind_trick) {
		AddBind(XorStr("Jedi Mind-Trick"), g_Vars.misc.mind_trick_bind);
	}

	float gaySize = this->m_KeyBinds.size.y;

	static float alpha = 0.f;
	bool condition = ((vecNames.empty() && (Menu::opened)) || !vecNames.empty());
	float multiplier = static_cast<float>((1.0f / 0.05f) * Interfaces::m_pGlobalVars->frametime);
	if (condition) {
		alpha += multiplier * (1.0f - alpha);
	}
	else {
		if (alpha > 0.01f)
			alpha += multiplier * (0.0f - alpha);
		else
			alpha = 0.0f;
	}

	alpha = std::clamp(alpha, 0.f, 1.0f);

	if (alpha <= 0.f)
		return;

	Color main = Color(39, 41, 54, 150 * alpha);
	Color accent = g_Vars.menu.ascent.ToRegularColor();
	accent.RGBA[3] *= alpha;

	this->m_KeyBinds.size.y = 20.0f;

	// header
	Render::Engine::RectFilled(pos, this->m_KeyBinds.size, Color(41, 32, 59, 230 * alpha));

	//line
	Render::Engine::Rect(pos.x, pos.y, this->m_KeyBinds.size.x, 1.5f, Color(255, 215, 0, 240 * alpha));

	// line splitting
	//Render::Engine::Line( pos + Vector2D( 0, this->m_KeyBinds.size.y ), pos + this->m_KeyBinds.size, accent );
	//Render::Engine::Line( pos + Vector2D( 0, this->m_KeyBinds.size.y + 1 ), pos + Vector2D( this->m_KeyBinds.size.x, this->m_KeyBinds.size.y + 1 ), accent );

	this->m_KeyBinds.size.y = gaySize;

	// the actual window
	//Render::Engine::RectFilled( pos + Vector2D( 0, 20 + 2 ), Vector2D( this->m_KeyBinds.size.x, this->m_KeyBinds.size.y - 1 ), main );

	auto hold_size = Render::Engine::esp.size(XorStr("[hold]"));
	auto toggle_size = Render::Engine::esp.size(XorStr("[toggled]"));
	auto always_size = Render::Engine::esp.size(XorStr("[always]"));

	if (!vecNames.empty()) {
		float offset = 15.0f;
		for (auto name : vecNames) {
			// hotkey name
			Render::Engine::esp.string(pos.x + 2, pos.y + 9 + offset, Color::White().OverrideAlpha(255 * alpha), name.first.c_str());

			// hotkey type
			Render::Engine::esp.string(pos.x + (this->m_KeyBinds.size.x - (name.second == KeyBindType::HOLD ? hold_size.m_width : name.second == KeyBindType::TOGGLE ? toggle_size.m_width : always_size.m_width)), pos.y + 9 + offset, Color::White().OverrideAlpha(255 * alpha),
				name.second == KeyBindType::HOLD ? XorStr("[hold]") : name.second == KeyBindType::TOGGLE ? XorStr("[toggled]") : XorStr("[always]"));

			// add offset
			offset += 14.0f;
		}
	}

	// title
	auto size = Render::Engine::esp.size(XorStr("keybinds"));
	Render::Engine::esp.string(pos.x + (this->m_KeyBinds.size.x * 0.5) - 2, pos.y + (size.m_height * 0.5) - 4, Color::White().OverrideAlpha(255 * alpha), XorStr("keybinds"), Render::Engine::ALIGN_CENTER);
}

// lol
bool IsAimingAtPlayerThroughPenetrableWall( C_CSPlayer* local, C_WeaponCSBaseGun* pWeapon ) {
	auto weaponInfo = pWeapon->GetCSWeaponData( );
	if( !weaponInfo.IsValid( ) )
		return -1.0f;

	QAngle view_angles;
	Interfaces::m_pEngine->GetViewAngles( view_angles );

	Autowall::C_FireBulletData data;

	data.m_Player = local;
	data.m_TargetPlayer = nullptr;
	data.m_bPenetration = true;
	data.m_vecStart = local->GetEyePosition( );
	data.m_vecDirection = view_angles.ToVectors( );
	data.m_flMaxLength = data.m_vecDirection.Normalize( );
	data.m_WeaponData = weaponInfo.Xor( );
	data.m_flCurrentDamage = static_cast< float >( weaponInfo->m_iWeaponDamage );

	return Autowall::FireBullets( &data ) >= 1.f;
}

float GetPenetrationDamage( C_CSPlayer* local, C_WeaponCSBaseGun* pWeapon ) {
	auto weaponInfo = pWeapon->GetCSWeaponData( );
	if( !weaponInfo.IsValid( ) )
		return -1.0f;

	Autowall::C_FireBulletData data;

	data.m_iPenetrationCount = 4;
	data.m_Player = local;
	data.m_TargetPlayer = nullptr;

	QAngle view_angles;
	Interfaces::m_pEngine->GetViewAngles( view_angles );
	data.m_vecStart = local->GetEyePosition( );
	data.m_vecDirection = view_angles.ToVectors( );
	data.m_flMaxLength = data.m_vecDirection.Normalize( );
	data.m_WeaponData = weaponInfo.Xor( );
	data.m_flTraceLength = 0.0f;

	data.m_flCurrentDamage = static_cast< float >( weaponInfo->m_iWeaponDamage );

	CTraceFilter filter;
	filter.pSkip = local;

	Vector end = data.m_vecStart + data.m_vecDirection * weaponInfo->m_flWeaponRange;

	Autowall::TraceLine( data.m_vecStart, end, MASK_SHOT_HULL | CONTENTS_HITBOX, local, &data.m_EnterTrace );
	Autowall::ClipTraceToPlayers( data.m_vecStart, end + data.m_vecDirection * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &data.m_EnterTrace );
	if( data.m_EnterTrace.fraction == 1.f )
		return -1.0f;

	data.m_flTraceLength += data.m_flMaxLength * data.m_EnterTrace.fraction;
	if( data.m_flMaxLength != 0.0f && data.m_flTraceLength >= data.m_flMaxLength )
		return data.m_flCurrentDamage;

	data.m_flCurrentDamage *= powf( weaponInfo->m_flRangeModifier, data.m_flTraceLength * 0.002f );
	data.m_EnterSurfaceData = Interfaces::m_pPhysSurface->GetSurfaceData( data.m_EnterTrace.surface.surfaceProps );

	C_BasePlayer* hit_player = static_cast< C_BasePlayer* >( data.m_EnterTrace.hit_entity );
	bool can_do_damage = ( data.m_EnterTrace.hitgroup >= Hitgroup_Head && data.m_EnterTrace.hitgroup <= Hitgroup_Gear );
	bool hit_target = !data.m_TargetPlayer || hit_player == data.m_TargetPlayer;
	if( can_do_damage && hit_player && hit_player->EntIndex( ) <= Interfaces::m_pGlobalVars->maxClients && hit_player->EntIndex( ) > 0 && hit_target ) {
		if( pWeapon && pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS )
			return ( data.m_flCurrentDamage * 0.9f );

		if( pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS ) {
			data.m_EnterTrace.hitgroup = Hitgroup_Generic;
		}

		data.m_flCurrentDamage = Autowall::ScaleDamage( ( C_CSPlayer* )hit_player, data.m_flCurrentDamage, weaponInfo->m_flArmorRatio, data.m_EnterTrace.hitgroup );
		return data.m_flCurrentDamage;
	};

	if( data.m_flTraceLength > 3000.0f && weaponInfo->m_flPenetration > 0.f || 0.1f > data.m_EnterSurfaceData->game.flPenetrationModifier )
		return -1.0f;

	if( Autowall::HandleBulletPenetration( &data ) )
		return -1.0f;

	return data.m_flCurrentDamage;
};

void CEsp::PenetrateCrosshair( Vector2D center ) {
	C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	if( !local || local->IsDead( ) )
		return;

	C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );
	if( !pWeapon )
		return;

	if( !pWeapon->GetCSWeaponData( ).IsValid( ) )
		return;

	auto type = pWeapon->GetCSWeaponData( ).Xor( )->m_iWeaponType;

	if( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	// fps enhancer
	g_Vars.globals.m_nPenetrationDmg = ( int )GetPenetrationDamage( local, pWeapon );
	g_Vars.globals.m_bAimAtEnemyThruWallOrVisibleLoool = IsAimingAtPlayerThroughPenetrableWall( local, pWeapon );
	Color color = g_Vars.globals.m_bAimAtEnemyThruWallOrVisibleLoool ? ( Color( 0, 30, 225, 210 ) ) : ( g_Vars.globals.m_nPenetrationDmg >= 1 ? Color( 0, 255, 0, 210 ) : Color( 255, 0, 0, 210 ) );

	if( g_Vars.esp.autowall_crosshair ) {
		Render::Engine::RectFilled( center - 1, { 3, 3 }, Color( 0, 0, 0, 125 ) );
		Render::Engine::RectFilled( Vector2D( center.x, center.y - 1 ), Vector2D( 1, 3 ), color );
		Render::Engine::RectFilled( Vector2D( center.x - 1, center.y ), Vector2D( 3, 1 ), color );
		//Render::Engine::tahoma_sexy.string(center.x - 10, center.y + 2, Color(255,255,255,255), std::to_string(g_Vars.globals.m_nPenetrationDmg));
	}
}

void CEsp::DrawAntiAimIndicator( ) {
	if( !g_Vars.antiaim.manual || !g_Vars.antiaim.manual_arrows || !Interfaces::m_pEngine->IsInGame( ) )
		return;

	C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
	if( !local || local->IsDead( ) )
		return;

	//if( g_Vars.globals.manual_aa == -1 )
	//	return;

	bool bLeft = false, bRight = false, bBack = false;
	switch( g_Vars.globals.manual_aa ) {
	case 0:
		bLeft = true;
		bRight = false;
		bBack = false;
		break;
	case 1:
		bLeft = false;
		bRight = false;
		bBack = true;
		break;
	case 2:
		bLeft = false;
		bRight = true;
		bBack = false;
		break;
	}

	Color color = g_Vars.antiaim.manual_color.ToRegularColor( );

	float alpha = 255/*floor( sin( Interfaces::m_pGlobalVars->realtime * 4 ) * ( color.RGBA[ 3 ] / 2 - 1 ) + color.RGBA[ 3 ] / 2 )*/;

	color.RGBA[ 3 ] = alpha;

	// Polygon points aka arrows
	auto ScreenSize = Render::GetScreenSize( );
	auto center = ScreenSize * 0.5f;

	Vector2D LPx = { ( center.x ) - 50, ( center.y ) + 10 };
	Vector2D LPy = { ( center.x ) - 50, ( center.y ) - 10 };
	Vector2D LPz = { ( center.x ) - 70, ( center.y ) };
	Vector2D RPx = { ( center.x ) + 50, ( center.y ) + 10 };
	Vector2D RPy = { ( center.x ) + 50, ( center.y ) - 10 };
	Vector2D RPz = { ( center.x ) + 70, ( center.y ) };
	Vector2D LPxx = { ( center.x ) - 49, ( center.y ) + 12 };
	Vector2D LPyy = { ( center.x ) - 49, ( center.y ) - 12 };
	Vector2D LPzz = { ( center.x ) - 73, ( center.y ) };
	Vector2D RPxx = { ( center.x ) + 49, ( center.y ) + 12 };
	Vector2D RPyy = { ( center.x ) + 49, ( center.y ) - 12 };
	Vector2D RPzz = { ( center.x ) + 73, ( center.y ) };
	Vector2D BPx = { ( center.x ) + 10, ( center.y ) + 50 };
	Vector2D BPy = { ( center.x ) - 10, ( center.y ) + 50 };
	Vector2D BPz = { ( center.x ), ( center.y ) + 70 };
	Vector2D BPxx = { ( center.x ) + 12, ( center.y ) + 49 };
	Vector2D BPyy = { ( center.x ) - 12, ( center.y ) + 49 };
	Vector2D BPzz = { ( center.x ), ( center.y ) + 73 };

	// Shadows
	//Render::Engine::FilledTriangle( LPxx, LPzz, LPyy, { 0, 0, 0, 125 } );
	//Render::Engine::FilledTriangle( RPyy, RPzz, RPxx, { 0, 0, 0, 125 } );
	//Render::Engine::FilledTriangle( BPyy, BPxx, BPzz, { 0, 0, 0, 125 } );

	if( bLeft )
		Render::Engine::FilledTriangle( LPx, LPz, LPy, color );
	else if( bRight )
		Render::Engine::FilledTriangle( RPy, RPz, RPx, color );
	else if( bBack )
		Render::Engine::FilledTriangle( BPy, BPx, BPz, color );
}

void CEsp::DrawZeusDistance( ) {
	if( !g_Vars.esp.zeus_distance )
		return;

	C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer( );

	if( !pLocalPlayer || pLocalPlayer->IsDead( ) )
		return;

	C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )pLocalPlayer->m_hActiveWeapon( ).Get( );

	if( !pWeapon )
		return;

	auto pWeaponInfo = pWeapon->GetCSWeaponData( );
	if( !pWeaponInfo.IsValid( ) )
		return;

	if( !( pWeapon->m_iItemDefinitionIndex( ) == WEAPON_ZEUS ) )
		return;

	auto collision = pLocalPlayer->m_Collision( );
	Vector eyePos = pLocalPlayer->GetAbsOrigin( ) + ( collision->m_vecMins + collision->m_vecMaxs ) * 0.5f;

	float flBestDistance = FLT_MAX;
	C_CSPlayer* pBestPlayer = nullptr;

	for( int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++ ) {

		C_CSPlayer* player = ( C_CSPlayer* )( Interfaces::m_pEntList->GetClientEntity( i ) );

		if( !player || player->IsDead( ) || player->IsDormant( ) )
			continue;

		float Dist = pLocalPlayer->m_vecOrigin( ).Distance( player->m_vecOrigin( ) );

		if( Dist < flBestDistance ) {
			flBestDistance = Dist;
			pBestPlayer = player;
		}
	}

	auto GetZeusRange = [ & ] ( C_CSPlayer* player ) -> float {
		const float RangeModifier = 0.00490000006f, MaxDamage = 500.f;
		return ( log( player->m_iHealth( ) / MaxDamage ) / log( RangeModifier ) ) / 0.002f;
	};

	float flRange = 0.f;
	if( pBestPlayer ) {
		flRange = GetZeusRange( pBestPlayer );
	}

	const int accuracy = 360;
	const float step = DirectX::XM_2PI / accuracy;
	for( float a = 0.0f; a < DirectX::XM_2PI; a += step ) {
		float a_c, a_s, as_c, as_s;
		DirectX::XMScalarSinCos( &a_s, &a_c, a );
		DirectX::XMScalarSinCos( &as_s, &as_c, a + step );

		Vector startPos = Vector( a_c * flRange + eyePos.x, a_s * flRange + eyePos.y, eyePos.z );
		Vector endPos = Vector( as_c * flRange + eyePos.x, as_s * flRange + eyePos.y, eyePos.z );

		Ray_t ray;
		CGameTrace tr;
		CTraceFilter filter = CTraceFilter( );
		filter.pSkip = pLocalPlayer;

		ray.Init( eyePos, startPos );
		Interfaces::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );

		auto frac_1 = tr.fraction;
		Vector2D start2d;
		if( !WorldToScreen( tr.endpos, start2d ) )
			continue;

		ray.Init( eyePos, endPos );
		Interfaces::m_pEngineTrace->TraceRay( ray, MASK_SOLID, &filter, &tr );

		Vector2D end2d;

		if( !WorldToScreen( tr.endpos, end2d ) )
			continue;

		Render::Engine::Line( start2d, end2d, g_Vars.esp.zeus_distance_color.ToRegularColor( ) );
	}
}

void CEsp::IndicateAngles() // moneybot :))))))
{

	if (!g_Vars.esp.draw_antiaim_angles)
		return;

	C_CSPlayer* pLocalPlayer = C_CSPlayer::GetLocalPlayer();

	if (!pLocalPlayer || pLocalPlayer->IsDead())
		return;

	int screen_w, screen_h;

	screen_w = Render::GetScreenSize().x;
	screen_h = Render::GetScreenSize().y;

	if (Interfaces::m_pEngine->IsInGame() && Interfaces::m_pEngine->IsConnected()) {

		auto get_rotated_point = [](Vector2D point, float rotation, float distance) {
			float rad = Math::deg_to_rad(rotation);

			point.x += sin(rad) * distance;
			point.y += cos(rad) * distance;

			return point;
		};

		auto draw_rotated_triangle = [&get_rotated_point, this](Vector2D point, float rotation, Color col) {
			Vector2D rotated_pos_1 = get_rotated_point(point, rotation + 205.f, 30.f);
			Vector2D rotated_pos_2 = get_rotated_point(point, rotation + 155.f, 30.f);

			Vertex_t v[] = {
				{ point },
			{ rotated_pos_1 },
			{ rotated_pos_2 }
			};


			Render::Engine::Line(point, rotated_pos_1, col);
			Render::Engine::Line(point, rotated_pos_2, col);
			Render::Engine::Line(rotated_pos_1, rotated_pos_2, col);

		};

		Vector2D rotated_lby{ screen_w * 0.5f, screen_h * 0.5f };
		Vector2D rotated_real{ screen_w * 0.5f, screen_h * 0.5f };

		QAngle angles{ };
		Interfaces::m_pEngine->GetViewAngles(angles);

		float rotation_lby = std::remainderf(pLocalPlayer->m_flLowerBodyYawTarget() - angles.y, 360.f) - 180.f;
		float rotation_real = std::remainderf(g_Vars.globals.flRealYaw -angles.y, 360.f) - 180.f;

		rotated_lby = get_rotated_point(rotated_lby, rotation_lby, 120.f);
		rotated_real = get_rotated_point(rotated_real, rotation_real, 120.f);

		draw_rotated_triangle(rotated_lby, rotation_lby, g_Vars.esp.draw_antiaim_angles_lby.ToRegularColor());
		draw_rotated_triangle(rotated_real, rotation_real, g_Vars.esp.draw_antiaim_angles_real.ToRegularColor());

	}
}

void CEsp::SpreadCrosshair() {
	auto local = C_CSPlayer::GetLocalPlayer();

	// dont do if dead.
	if (!local || local->IsDead())
		return;

	if (!g_Vars.esp.spread_crosshair)
		return;

	// get active weapon.
	C_WeaponCSBaseGun* weapon = (C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto data = weapon->GetCSWeaponData();
	if (!data.IsValid())
		return;


	// do not do this on: bomb, knife and nades.
	int type = data->m_iWeaponType;
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;

	// calc radius.
	float radius = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / (std::tan(Math::deg_to_rad(local->GetFOV()) * 0.5f) + FLT_EPSILON);

	Vector2D center = Render::GetScreenSize();

	// scale by screen size.
	radius *= center.y * (1.f / 480.f);

	// get color.
	Color col = g_Vars.esp.spread_crosshair_color.ToRegularColor();

	int segements = std::max(16, (int)std::round(radius * 0.75f));

	Render::Engine::CircleFilled(center.x / 2, center.y / 2, radius, segements, col);
}

std::map<int, char> weapon_icons = {
	{ WEAPON_DEAGLE, 'A' },
	{ WEAPON_ELITE, 'B' },
	{ WEAPON_FIVESEVEN, 'C' },
	{ WEAPON_GLOCK, 'D' },
	{ WEAPON_P2000, 'E' },
	{ WEAPON_P250, 'F' },
	{ WEAPON_USPS, 'G' },
	{ WEAPON_TEC9, 'H' },
	{ WEAPON_CZ75A, 'I' },
	{ WEAPON_REVOLVER, 'J' },
	{ WEAPON_MAC10, 'K' },
	{ WEAPON_UMP45, 'L' },
	{ WEAPON_BIZON, 'M' },
	{ WEAPON_MP7, 'N' },
	{ WEAPON_MP9, 'O' },
	{ WEAPON_P90, 'P' },
	{ WEAPON_GALIL, 'Q' },
	{ WEAPON_FAMAS, 'R' },
	{ WEAPON_M4A4, 'S' },
	{ WEAPON_M4A1S, 'T' },
	{ WEAPON_AUG, 'U' },
	{ WEAPON_SG553, 'V' },
	{ WEAPON_AK47, 'W' },
	{ WEAPON_G3SG1, 'X' },
	{ WEAPON_SCAR20, 'Y' },
	{ WEAPON_AWP, 'Z' },
	{ WEAPON_SSG08, 'a' },
	{ WEAPON_XM1014, 'b' },
	{ WEAPON_SAWEDOFF, 'c' },
	{ WEAPON_MAG7, 'd' },
	{ WEAPON_NOVA, 'e' },
	{ WEAPON_NEGEV, 'f' },
	{ WEAPON_M249, 'g' },
	{ WEAPON_ZEUS, 'h' },
	{ WEAPON_KNIFE_T, 'i' },
	{ WEAPON_KNIFE_CT, 'j' },
	{ WEAPON_KNIFE_FALCHION, '0' },
	{ WEAPON_KNIFE_BAYONET, '1' },
	{ WEAPON_KNIFE_FLIP, '2' },
	{ WEAPON_KNIFE_GUT, '3' },
	{ WEAPON_KNIFE_KARAMBIT, '4' },
	{ WEAPON_KNIFE_M9_BAYONET, '5' },
	{ WEAPON_KNIFE_HUNTSMAN, '6' },
	{ WEAPON_KNIFE_BOWIE, '7' },
	{ WEAPON_KNIFE_BUTTERFLY, '8' },
	{ WEAPON_FLASHBANG, 'k' },
	{ WEAPON_HEGRENADE, 'l' },
	{ WEAPON_SMOKE, 'm' },
	{ WEAPON_MOLOTOV, 'n' },
	{ WEAPON_DECOY, 'o' },
	{ WEAPON_FIREBOMB, 'p' },
	{ WEAPON_C4, 'q' },
};

std::string GetWeaponIcon(const int id) {
	auto search = weapon_icons.find(id);
	if (search != weapon_icons.end())
		return std::string(&search->second, 1);

	return "";
}

void CEsp::Main( ) {
	//DrawWatermark( );
	//spotify( ); - there might be fps drops / fps issues caused by this.

	//if( g_Vars.esp.keybind_window_enabled )
	//	Keybinds( );

	//if( g_Vars.esp.spec_window_enabled )
	//	SpectatorList( true );

	m_LocalPlayer = C_CSPlayer::GetLocalPlayer( );
	if( !g_Vars.globals.HackIsReady || !m_LocalPlayer || !Interfaces::m_pEngine->IsInGame( ) ) {
		g_Vars.globals.vader_user.clear( );
		g_Vars.globals.vader_beta.clear( );
		g_Vars.globals.vader_dev.clear( );
		g_Vars.globals.vader_crack.clear( );
		return;
	}

	if( g_Vars.esp.remove_scope && g_Vars.esp.remove_scope_type == 0 && ( m_LocalPlayer && m_LocalPlayer->m_hActiveWeapon( ).Get( ) && ( ( C_WeaponCSBaseGun* )m_LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( ).IsValid( ) && ( ( C_WeaponCSBaseGun* )m_LocalPlayer->m_hActiveWeapon( ).Get( ) )->GetCSWeaponData( )->m_iWeaponType == WEAPONTYPE_SNIPER_RIFLE && m_LocalPlayer->m_bIsScoped( ) ) ) {
		Interfaces::m_pSurface->DrawSetColor( Color( 0, 0, 0, 255 ) );
		Vector2D center = Render::GetScreenSize( );
		Interfaces::m_pSurface->DrawLine( center.x / 2, 0, center.x / 2, center.y );
		Interfaces::m_pSurface->DrawLine( 0, center.y / 2, center.x, center.y / 2 );

	}

	if (Interfaces::m_pGlobalVars->tickcount % 2 == 1)
	{
		if (Interfaces::MusicPlayer::Instance()->time_to_reset_sound > 0.f && Interfaces::MusicPlayer::Instance()->time_to_reset_sound <= Interfaces::m_pGlobalVars->realtime) {
			Interfaces::MusicPlayer::Instance()->stop();
			Interfaces::MusicPlayer::Instance()->time_to_reset_sound = 0.f;
		}
	}

	IndicateAngles( );

	//if( !m_LocalPlayer->IsDead( ) )
	//	Indicators( );

	auto& predicted_nades = g_grenades_pred.get_list();

	static auto last_server_tick = Interfaces::m_pEngine->GetServerTick();
	if (Interfaces::m_pEngine->GetServerTick() != last_server_tick) {
		predicted_nades.clear();

		last_server_tick = Interfaces::m_pEngine->GetServerTick();
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= Interfaces::m_pEntList->GetHighestEntityIndex(); ++i) {
		auto* ent = (C_BaseEntity*)Interfaces::m_pEntList->GetClientEntity(i);
		if (!ent)
			continue;

		C_CSPlayer* player = (C_CSPlayer*)ent->m_hOwnerEntity().Get();

		if (!player)
			continue;

		if (player->m_iTeamNum() == m_LocalPlayer->m_iTeamNum() && player->EntIndex() != m_LocalPlayer->EntIndex())
			continue;

		if (ent->IsDormant())
			continue;

		if (!ent->GetClientClass() /*|| !entity->GetClientClass( )->m_ClassID*/)
			continue;

		if (ent->GetClientClass()->m_ClassID != ClassId_t::CMolotovProjectile
			&& ent->GetClientClass()->m_ClassID != ClassId_t::CBaseCSGrenadeProjectile)
			continue;

		if (ent->GetClientClass()->m_ClassID == ClassId_t::CBaseCSGrenadeProjectile) {
			const auto studio_model = ent->GetModel();
			if (!studio_model
				|| std::string_view(studio_model->szName).find(XorStr("fraggrenade")) == std::string::npos)
				continue;
		}

		const auto handle = reinterpret_cast<C_CSPlayer*>(ent)->GetRefEHandle().ToLong();

		if (ent->m_fEffects() & EF_NODRAW) {
			predicted_nades.erase(handle);

			continue;
		}

		if (predicted_nades.find(handle) == predicted_nades.end()) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(handle),
				std::forward_as_tuple(
					reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntityFromHandle(ent->m_hThrower())),
					ent->GetClientClass()->m_ClassID == ClassId_t::CMolotovProjectile ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
					ent->m_vecOrigin(), reinterpret_cast<C_CSPlayer*>(ent)->m_vecVelocity(), ent->m_flSpawnTime_Grenade(),
					TIME_TO_TICKS(reinterpret_cast<C_CSPlayer*>(ent)->m_flSimulationTime() - ent->m_flSpawnTime_Grenade())
				)
			);
		}

		if (predicted_nades.at(handle).draw())
			continue;

		predicted_nades.erase(handle);
	}

	g_grenades_pred.get_local_data().draw();


	for (auto k = 0; k < Interfaces::m_pEntList->GetHighestEntityIndex(); k++)
	{
		C_CSPlayer* entity = Interfaces::m_pEntList->GetClientEntity(k)->as<C_CSPlayer*>();

		if (entity == nullptr ||
			!entity->GetClientClass() ||
			entity == m_LocalPlayer)
			continue;

		g_grenades_pred.grenade_warning(entity);
		g_grenades_pred.get_local_data().draw();
	}

	OverlayInfo();

	SpreadCrosshair();

	DrawLocalSkeleton();

	circle_above_head(m_LocalPlayer);

	dlight_players();

	//walkbot::Instance().update(false);

	if( !g_Vars.esp.esp_enable )
		return;

	//DrawZeusDistance( );

	Vector2D points[ 8 ];
	Vector2D center;

	//if( g_Vars.esp.shot_visualization == 3 )
		//DrawHitSkeleton( );

	for( int i = 0; i <= Interfaces::m_pEntList->GetHighestEntityIndex( ); ++i ) {
		auto entity = ( C_BaseEntity* )Interfaces::m_pEntList->GetClientEntity( i );

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		if( !entity )
			continue;

		if( !entity->GetClientClass( ) /*|| !entity->GetClientClass( )->m_ClassID*/ )
			continue;

		if (g_Vars.esp.nades) {

			if (entity->GetClientClass()->m_ClassID == CInferno) {
				C_Inferno* pInferno = reinterpret_cast<C_Inferno*>(entity);
				C_CSPlayer* player = (C_CSPlayer*)entity->m_hOwnerEntity().Get();

				int dist = m_LocalPlayer->GetAbsOrigin().Distance(pInferno->GetAbsOrigin());

				if (dist > 2000)
					return;

				if (player) {
					FloatColor color;

					if (player->m_iTeamNum() == m_LocalPlayer->m_iTeamNum() && player->EntIndex() != m_LocalPlayer->EntIndex()) {
						color = FloatColor(66.f / 255.f, 123.f / 255.f, 245.f / 255.f, 0.8f);
					}
					else {
						color = FloatColor(1.f, 0.f, 0.f, 0.8f);
					}

					const Vector origin = pInferno->GetAbsOrigin();
					Vector2D screen_origin = Vector2D();

					if (WorldToScreen(origin, screen_origin)) {
						struct s {
							Vector2D a, b, c;
						};
						std::vector<int> excluded_ents;
						std::vector<s> valid_molotovs;

						const auto spawn_time = pInferno->m_flSpawnTime();
						const auto time = ((spawn_time + C_Inferno::GetExpiryTime()) - Interfaces::m_pGlobalVars->curtime);

						if (time > 0.05f) {
							static const auto size = Vector2D(70.f, 4.f);

							auto new_pos = Vector2D(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5);

							Vector min, max;
							entity->GetClientRenderable()->GetRenderBounds(min, max);

							auto radius = (max - min).Length2D() * 0.5f;
							Vector boundOrigin = Vector((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, min.z + 5) + origin;
							const int accuracy = 25;
							const float step = DirectX::XM_2PI / accuracy;
							for (float a = 0.0f; a < DirectX::XM_2PI; a += step) {
								float a_c, a_s, as_c, as_s;
								DirectX::XMScalarSinCos(&a_s, &a_c, a);
								DirectX::XMScalarSinCos(&as_s, &as_c, a + step);

								Vector startPos = Vector(a_c * radius + boundOrigin.x, a_s * radius + boundOrigin.y, boundOrigin.z);
								Vector endPos = Vector(as_c * radius + boundOrigin.x, as_s * radius + boundOrigin.y, boundOrigin.z);

								Vector2D start2d, end2d, boundorigin2d;
								if (!WorldToScreen(startPos, start2d) || !WorldToScreen(endPos, end2d) || !WorldToScreen(boundOrigin, boundorigin2d)) {
									excluded_ents.push_back(i);
									continue;
								}

								s n;
								n.a = start2d;
								n.b = end2d;
								n.c = boundorigin2d;
								valid_molotovs.push_back(n);
							}

							if (!excluded_ents.empty()) {
								for (int v = 0; v < excluded_ents.size(); ++v) {
									auto bbrr = excluded_ents[v];
									if (bbrr == i)
										continue;

									if (!valid_molotovs.empty())
										for (int m = 0; m < valid_molotovs.size(); ++m) {
											auto ba = valid_molotovs[m];
											//Render::Engine::FilledTriangle( ba.c, ba.a, ba.b, color.ToRegularColor( ).OverrideAlpha( 45 ) );
											if (g_Vars.esp.molotov_radius) {
												Render::Engine::Line(ba.a, ba.b, color.ToRegularColor().OverrideAlpha(220));
											}
										}
								}
							}
							else {
								if (!valid_molotovs.empty())
									for (int m = 0; m < valid_molotovs.size(); ++m) {
										auto ba = valid_molotovs[m];
										//Render::Engine::FilledTriangle( ba.c, ba.a, ba.b, color.ToRegularColor( ).OverrideAlpha( 45 ) );
										if (g_Vars.esp.molotov_radius) {
											Render::Engine::Line(ba.a, ba.b, color.ToRegularColor().OverrideAlpha(220));
										}
									}
							}

							char buf[128] = { };
							sprintf(buf, XorStr(" - %.2fs"), time);
							//Render::Engine::RectFilled( Vector2D( new_pos.x - 2, new_pos.y - 15 ),
							//	Vector2D( Render::Engine::segoe.size( buf ).m_width + 4, Render::Engine::segoe.size( buf ).m_height ), Color( 0, 0, 0, 200 ) );

							if (g_Vars.esp.molotov_timer) {
								Render::Engine::cs_huge.string(new_pos.x + 35 - (Render::Engine::grenades.size(buf).m_width * 0.6f), new_pos.y - 23, Color(255, 255, 255, 255), "n", Render::Engine::ALIGN_CENTER);
								Render::Engine::grenades.string(new_pos.x + 35, new_pos.y - 15, Color(255, 255, 255, 255), buf, Render::Engine::ALIGN_CENTER);
							}

						}
						else {
							if (!valid_molotovs.empty())
								valid_molotovs.erase(valid_molotovs.begin() + i);

							if (!excluded_ents.empty())
								excluded_ents.erase(excluded_ents.begin() + i);
						}
					}
				}
			}

			C_SmokeGrenadeProjectile* pSmokeEffect = reinterpret_cast<C_SmokeGrenadeProjectile*>(entity);
			if (pSmokeEffect->GetClientClass()->m_ClassID == CSmokeGrenadeProjectile) {
				const Vector origin = pSmokeEffect->GetAbsOrigin();
				Vector2D screen_origin = Vector2D();

				int dist = m_LocalPlayer->GetAbsOrigin().Distance(origin);

				if (dist > 2000)
					return;

				if (WorldToScreen(origin, screen_origin)) {
					struct s {
						Vector2D a, b;
					};
					std::vector<int> excluded_ents;
					std::vector<s> valid_smokes;
					const auto spawn_time = TICKS_TO_TIME(pSmokeEffect->m_nSmokeEffectTickBegin());
					const auto time = (spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime()) - Interfaces::m_pGlobalVars->curtime;

					static const auto size = Vector2D(70.f, 4.f);

					auto new_pos = Vector2D(screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5);
					if (time > 0.f) {
						auto radius = 120.f;

						const int accuracy = 25;
						const float step = DirectX::XM_2PI / accuracy;
						for (float a = 0.0f; a < DirectX::XM_2PI; a += step) {
							float a_c, a_s, as_c, as_s;
							DirectX::XMScalarSinCos(&a_s, &a_c, a);
							DirectX::XMScalarSinCos(&as_s, &as_c, a + step);

							Vector startPos = Vector(a_c * radius + origin.x, a_s * radius + origin.y, origin.z + 5);
							Vector endPos = Vector(as_c * radius + origin.x, as_s * radius + origin.y, origin.z + 5);

							Vector2D start2d, end2d;
							if (!WorldToScreen(startPos, start2d) || !WorldToScreen(endPos, end2d)) {
								excluded_ents.push_back(i);
								continue;
							}

							s n;
							n.a = start2d;
							n.b = end2d;
							valid_smokes.push_back(n);
						}

						if (!excluded_ents.empty()) {
							for (int v = 0; v < excluded_ents.size(); ++v) {
								auto bbrr = excluded_ents[v];
								if (bbrr == i)
									continue;

								if (!valid_smokes.empty())
									for (int m = 0; m < valid_smokes.size(); ++m) {
										auto ba = valid_smokes[m];
										//Render::Engine::FilledTriangle( screen_origin, ba.a, ba.b, Color( 220, 220, 220, 25 ) );
										if (g_Vars.esp.smoke_radius) {
											Render::Engine::Line(ba.a, ba.b, Color(220, 220, 220, 220));
										}
									}
							}
						}
						else {
							if (!valid_smokes.empty())
								for (int m = 0; m < valid_smokes.size(); ++m) {
									auto ba = valid_smokes[m];
									//Render::Engine::FilledTriangle( screen_origin, ba.a, ba.b, Color( 220, 220, 220, 25 ) );
									if (g_Vars.esp.smoke_radius) {
										Render::Engine::Line(ba.a, ba.b, Color(220, 220, 220, 220));
									}
								}
						}

						char buf[128] = { };
						sprintf(buf, XorStr(" - %.2fs"), time);
						//Render::Engine::RectFilled( Vector2D( new_pos.x - 2, new_pos.y - 15 ),
						//	Vector2D( Render::Engine::segoe.size( buf ).m_width + 4, Render::Engine::segoe.size( buf ).m_height ), Color( 0, 0, 0, 200 ) );

						if (g_Vars.esp.smoke_timer) {
							Render::Engine::cs_huge.string(new_pos.x + 35 - (Render::Engine::grenades.size(buf).m_width * 0.6f), new_pos.y - 23, Color(255, 255, 255, 255), "m", Render::Engine::ALIGN_CENTER);
							Render::Engine::grenades.string(new_pos.x + 35, new_pos.y - 15, Color(255, 255, 255, 255), buf, Render::Engine::ALIGN_CENTER);
						}

					}
					else {
						if (!valid_smokes.empty())
							valid_smokes.erase(valid_smokes.begin() + i);

						if (!excluded_ents.empty())
							excluded_ents.erase(excluded_ents.begin() + i);
					}
				}
			}
		}

		auto player = ToCSPlayer( entity );

		if( ValidPlayer( player ) && i <= 64 ) {
			g_Vars.globals.m_vecTextInfo[ i ].clear( );

			if( Begin( player ) ) {
				if (!(player->IsTeammate(local))) {
					if (g_Vars.esp.name)
						DrawName(player, m_Data.bbox, m_Data.info);

					if (g_Vars.esp.skeleton)
						DrawSkeleton(player);

					if (g_Vars.esp.box)
						DrawBox(m_Data.bbox, g_Vars.esp.box_color, player);

					if (g_Vars.esp.health)
						DrawHealthBar(player, m_Data.bbox);

					DrawInfo(player, m_Data.bbox, m_Data.info);

					if (g_Vars.esp.draw_ammo_bar || g_Vars.esp.draw_lby_bar)
						AmmoBar(player, m_Data.bbox);

					DrawBottomInfo(player, m_Data.bbox, m_Data.info);
				}
				else if(g_Vars.esp.teamname) {
					DrawTeamName(player, m_Data.bbox, m_Data.info);
				}
			}
		}

		auto rgb_to_int = [ ] ( int red, int green, int blue ) -> int {
			int r;
			int g;
			int b;

			r = red & 0xFF;
			g = green & 0xFF;
			b = blue & 0xFF;
			return ( r << 16 | g << 8 | b );
		};

		if( entity->GetClientClass( )->m_ClassID == CFogController ) {
			static DWORD dwFogEnable = Engine::Displacement.DT_FogController.m_fog_enable;
			*( byte* )( ( uintptr_t )entity + dwFogEnable ) = g_Vars.esp.fog_effect;

			g_Vars.r_3dsky->SetValue( int( !g_Vars.esp.fog_effect ) );

			*( bool* )( uintptr_t( entity ) + 0xA1D ) = g_Vars.esp.fog_effect;
			*( float* )( uintptr_t( entity ) + 0x9F8 ) = 0;
			*( float* )( uintptr_t( entity ) + 0x9FC ) = g_Vars.esp.fog_distance;
			*( float* )( uintptr_t( entity ) + 0xA04 ) = g_Vars.esp.fog_density / 100.f;
			*( float* )( uintptr_t( entity ) + 0xA24 ) = g_Vars.esp.fog_hdr_scale / 100.f;
			*( int* )( uintptr_t( entity ) + 0x9E8 ) = rgb_to_int( ( int )( g_Vars.esp.fog_color.b * 255.f ), ( int )( g_Vars.esp.fog_color.g * 255.f ), ( int )( g_Vars.esp.fog_color.r * 255.f ) );
			*( int* )( uintptr_t( entity ) + 0x9EC ) = rgb_to_int( ( int )( g_Vars.esp.fog_color.b * 255.f ), ( int )( g_Vars.esp.fog_color.g * 255.f ), ( int )( g_Vars.esp.fog_color.r * 255.f ) );


			continue;
		}

		RenderNades( ( C_WeaponCSBaseGun* )entity );

		float distance{ };
		distance = !m_LocalPlayer->IsDead( ) ? m_LocalPlayer->GetAbsOrigin( ).Distance( entity->m_vecOrigin( ) ) : 0.f;

		float initial_alpha = 255.f;
		const auto clamped_distance = std::clamp<float>( distance - 300.f, 0.f, 510.f );
		initial_alpha = 255.f - ( clamped_distance * 0.5f );
		initial_alpha *= 0.70588235294;

		if( !_strcmpi( entity->GetClientClass( )->m_pNetworkName, ( XorStr( "CPlantedC4" ) ) ) ) {
			auto bomb_entity = ( C_PlantedC4* )entity;

			static ConVar* mp_c4timer = Interfaces::m_pCvar->FindVar( XorStr( "mp_c4timer" ) );
			static bool bomb_planted_tick_begin = false;
			static float bomb_time_tick_begin = 0.f;

			if( g_Vars.esp.draw_c4_bar && g_Vars.globals.bBombActive ) {
				Vector origin = bomb_entity->GetAbsOrigin( );
				Vector2D screen_origin = Vector2D( );

				if( !bomb_planted_tick_begin ) {
					bomb_time_tick_begin = Interfaces::m_pGlobalVars->curtime;
					bomb_planted_tick_begin = true;
				}

				float timer_bomb = 0.f;
				if( bomb_entity->m_flC4Blow( ) - Interfaces::m_pGlobalVars->curtime > 0.f )
					timer_bomb = bomb_entity->m_flC4Blow( ) - Interfaces::m_pGlobalVars->curtime;
				else
					timer_bomb = 0.f;

				if( timer_bomb > 0.f ) {
					const auto spawn_time = TIME_TO_TICKS( bomb_time_tick_begin );
					const auto factor = timer_bomb / mp_c4timer->GetFloat( );

					char subs[ 64 ] = { };
					sprintf( subs, XorStr( "%.1fs" ), timer_bomb );

					char buf[ 64 ] = { };
					sprintf( buf, XorStr( "C4 - %s" ), subs );

					if( factor > 0.f ) {
						Math::Clamp( factor, 0.f, 1.0f );

						if( g_Vars.globals.bBombTicked ) {
							g_Vars.globals.bBombTicked = false;
						}

						Render::Engine::indi.string(6, 11, { 0,0, 0, 125 }, buf);
						Render::Engine::indi.string(5, 10, Color::Green(), buf);

					}


					if( WorldToScreen( origin, screen_origin ) ) {
						static const auto size = Vector2D( Render::Engine::tahoma_sexy.size( buf ).m_width + 2, 4.f );

						auto new_pos = Vector2D( screen_origin.x - size.x * 0.5, screen_origin.y - size.y * 0.5 );

						if( factor > 0.f ) {
							Math::Clamp( factor, 0.f, 1.0f );
							Render::Engine::RectFilled( Vector2D(new_pos.x + 1, new_pos.y + 3), size, FloatColor( 0.f, 0.f, 0.f, 0.58f ).ToRegularColor( ) );
							Render::Engine::RectFilled( Vector2D( new_pos.x + 1, new_pos.y + 3 ), Vector2D( ( size.x - 1 ) * factor, size.y - 2 ), g_Vars.esp.c4_color.ToRegularColor( ).OverrideAlpha( 255 * 0.87f ) );

							Render::Engine::tahoma_sexy.string( new_pos.x + ( size.x * 0.5f ), new_pos.y - 9, g_Vars.esp.c4_color.ToRegularColor( ).OverrideAlpha( 180 * 0.87f ), buf, Render::Engine::ALIGN_CENTER );
						}
					}
				}
			}
		}

		if( g_Vars.esp.draw_c4_bar ) {
			if( entity->m_hOwnerEntity( ) == -1 &&
				entity->GetClientClass( )->m_ClassID == CC4 ) {

				Vector2D out;
				if( WorldToScreen( entity->GetAbsOrigin( ), out ) ) {
					Render::Engine::tahoma_sexy.string( out.x + 2, out.y, g_Vars.esp.c4_color.ToRegularColor( ).OverrideAlpha( 180 * 0.87f ), XorStr( "C4" ) );
				}
			}
		}

		if( ( g_Vars.esp.dropped_weapons || g_Vars.esp.dropped_weapons_ammo ) && initial_alpha && !m_LocalPlayer->IsDead( ) ) {
			if( !entity->IsWeapon( ) || entity->m_hOwnerEntity( ) != -1 ||
				entity->GetClientClass( )->m_ClassID == CC4 ||
				entity->GetClientClass( )->m_ClassID == CPlantedC4 )
				continue;

			auto weapon = reinterpret_cast< C_WeaponCSBaseGun* >( entity );
			if( !weapon )
				continue;

			Vector2D out;
			if( !WorldToScreen( weapon->GetAbsOrigin( ), out ) )
				continue;

			auto weapondata = weapon->GetCSWeaponData( );
			if( !weapondata.IsValid( ) )
				continue;

			if( !weapon->m_iItemDefinitionIndex( ) )
				continue;

			std::wstring localized = Interfaces::m_pLocalize->Find( weapondata->m_szHudName );
			std::string name{ localized.begin( ), localized.end( ) };
			std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

			if( name.empty( ) )
				continue;

			if( g_Vars.esp.dropped_weapons ) {
				if (g_Vars.esp.dropped_weapons_font == 0) {
					Render::Engine::cs.string(out.x + 2, out.y, g_Vars.esp.dropped_weapons_color.ToRegularColor().OverrideAlpha(static_cast<int>(initial_alpha)), GetWeaponIcon(weapon->m_iItemDefinitionIndex()));
				}
				else
					Render::Engine::tahoma_sexy.string(out.x + 2, out.y, g_Vars.esp.dropped_weapons_color.ToRegularColor().OverrideAlpha(static_cast<int>(initial_alpha)), name);
			}

			if( g_Vars.esp.dropped_weapons_ammo ) {
				auto clip = weapon->m_iClip1( );
				if( clip > 0 ) {

					Render::Engine::FontSize_t TextSize;
						
					if (g_Vars.esp.dropped_weapons_font == 0) {
						TextSize = Render::Engine::cs.size(GetWeaponIcon(weapon->m_iItemDefinitionIndex()));
					}
					else
						TextSize = Render::Engine::tahoma_sexy.size(name);

					const auto MaxClip = weapondata->m_iMaxClip;
					auto Width = TextSize.m_width;

					Width *= clip;
					Width /= MaxClip;

					Render::Engine::RectFilled( Vector2D( out.x, out.y ) + Vector2D( 1, 11 ), Vector2D( TextSize.m_width + 1, 4 ),
						FloatColor( 0.f, 0.f, 0.f, ( initial_alpha / 255.f ) * 0.58f ).ToRegularColor( ) );


					Render::Engine::RectFilled( Vector2D( out.x, out.y ) + Vector2D( 2, 12 ), Vector2D( Width - 1, 2 ),
						g_Vars.esp.dropped_weapons_color.ToRegularColor( ).OverrideAlpha( static_cast< int >( initial_alpha ) ) );

					if( clip <= static_cast< int >( MaxClip * 0.75 ) ) {
						Render::Engine::pixel_reg.string( out.x + Width, out.y + 8, Color::White( ).OverrideAlpha( static_cast< int >( initial_alpha ) ), std::to_string( clip ) );
					}
				}
			}
		}
	}
}

void CEsp::SetAlpha( int idx ) {
	m_bAlphaFix[ idx ] = true;
}

float CEsp::GetAlpha( int idx ) {
	return m_flAlpha[ idx ];
}

#include "../Rage/Resolver.hpp"
void CEsp::AmmoBar( C_CSPlayer* player, BBox_t bbox ) {
	if( !player )
		return;

	C_WeaponCSBaseGun* pWeapon = ( C_WeaponCSBaseGun* )player->m_hActiveWeapon( ).Get( );

	if( !pWeapon )
		return;

	auto pWeaponData = pWeapon->GetCSWeaponData( );
	if( !pWeaponData.IsValid( ) )
		return;

	int index = 0;

	if( g_Vars.esp.draw_ammo_bar ) {
		auto animLayer = player->m_AnimOverlay( ).Element( 1 );
		if( animLayer.m_pOwner && pWeaponData->m_iWeaponType != WEAPONTYPE_GRENADE && pWeaponData->m_iWeaponType != WEAPONTYPE_KNIFE && pWeaponData->m_iWeaponType != WEAPONTYPE_C4 ) {
			index += 6;

			auto activity = player->GetSequenceActivity( animLayer.m_nSequence );

			int current = pWeapon->m_iClip1( );
			int max = pWeaponData->m_iMaxClip;
			bool reloading = activity == 967 && animLayer.m_flWeight != 0.f;
			int reload_percentage = reloading ? std::ceil( 100 * animLayer.m_flCycle ) : 100;
			float scale;

			// check for reload.
			if( reloading )
				scale = animLayer.m_flCycle;

			// not reloading.
			// make the division of 2 ints produce a float instead of another int.
			else
				scale = max != -1 ? ( float )current / max : 1.f;

			// relative to bar.
			int bar = ( int )std::round( ( bbox.w - 2 ) * scale );

			// draw.
			Render::Engine::RectFilled( bbox.x, bbox.y + bbox.h + 2, bbox.w, 4, Color( 0, 0, 0, 180 * this->m_flAlpha[ player->EntIndex( ) ] ) );

			Color clr;
			if (!player->IsDormant()) {
				clr = g_Vars.esp.ammo_color.ToRegularColor();
			}
			else
				clr = Color(112, 112, 112, 180);

			clr.RGBA[ 3 ] *= this->m_flAlpha[ player->EntIndex( ) ];

			Render::Engine::Rect( bbox.x + 1, bbox.y + bbox.h + 3, bar, 2, clr );

			// less then a 5th of the bullets left.
			if( current < max || reloading ) {
				if (!player->IsDormant()) {
					Render::Engine::pixel_reg.string(bbox.x + bar, bbox.y + bbox.h, Color(255, 255, 255, 180 * this->m_flAlpha[player->EntIndex()]), std::to_string(reloading ? reload_percentage : current).append(reloading ? XorStr("%") : XorStr("")), Render::Engine::ALIGN_CENTER);
				}
				else
					Render::Engine::pixel_reg.string(bbox.x + bar, bbox.y + bbox.h, Color(112, 112, 112, 180 * this->m_flAlpha[player->EntIndex()]), std::to_string(reloading ? reload_percentage : current).append(reloading ? XorStr("%") : XorStr("")), Render::Engine::ALIGN_CENTER);
			}
		}
	}

	if( g_Vars.esp.draw_lby_bar ) {
		int current = pWeapon->m_iClip1( );
		int max = pWeaponData->m_iMaxClip;
		float scale;

		float flUpdateTime = Engine::g_ResolverData[ player->EntIndex( ) ].m_flNextBodyUpdate - player->m_flAnimationTime( );

		// check for pred.
		scale = ( 1.1f - flUpdateTime ) / 1.1f;

		// relative to bar.
		int bar = std::clamp( ( int )std::round( ( bbox.w - 2 ) * scale ), 0, bbox.w - 2 );

		// draw.
		Render::Engine::RectFilled( bbox.x, bbox.y + bbox.h + 2 + index, bbox.w, 4, Color( 0, 0, 0, 180 * this->m_flAlpha[ player->EntIndex( ) ] ) );

		Color clr;
		if (!player->IsDormant()) {
			clr = g_Vars.esp.lby_color.ToRegularColor( );
		}
		else
			clr = Color(112, 112, 112, 180);

		clr.RGBA[ 3 ] *= this->m_flAlpha[ player->EntIndex( ) ];

		Render::Engine::Rect( bbox.x + 1, bbox.y + bbox.h + 3 + index, bar, 2, clr );
	}
}

void CEsp::RenderNades(C_WeaponCSBaseGun* nade) {
	if (!g_Vars.esp.nades)
		return;

	const model_t* model = nade->GetModel();
	if (!model)
		return;

	studiohdr_t* hdr = Interfaces::m_pModelInfo->GetStudiomodel(model);
	if (!hdr)
		return;

	int item_definition = 0;
	bool dont_render = false;
	C_SmokeGrenadeProjectile* pSmokeEffect = nullptr;
	C_Inferno* pMolotov = nullptr;
	Color Nadecolor;
	std::string Name = hdr->szName;
	switch (nade->GetClientClass()->m_ClassID) {
	case ClassId_t::CBaseCSGrenadeProjectile:
		if (Name[16] == 's') {
			Name = XorStr("k");
			item_definition = WEAPON_FLASHBANG;
		}
		else {
			Name = XorStr("l");
			item_definition = WEAPON_HEGRENADE;
		}
		break;
	case ClassId_t::CSmokeGrenadeProjectile:
		Name = XorStr("m");
		item_definition = WEAPON_SMOKE;
		pSmokeEffect = reinterpret_cast<C_SmokeGrenadeProjectile*>(nade);
		if (pSmokeEffect) {
			const auto spawn_time = TICKS_TO_TIME(pSmokeEffect->m_nSmokeEffectTickBegin());
			const auto time = (spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime()) - Interfaces::m_pGlobalVars->curtime;
			const auto factor = ((spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime()) - Interfaces::m_pGlobalVars->curtime) / C_SmokeGrenadeProjectile::GetExpiryTime();

			if (factor > 0.0f)
				dont_render = true;
		}
		else {
			dont_render = false;
		}
		break;
	case ClassId_t::CMolotovProjectile:
		Name = XorStr("n");
		// bich
		if (nade && (nade->m_hOwnerEntity().Get()) && ((C_CSPlayer*)(nade->m_hOwnerEntity().Get()))) {
			item_definition = ((C_CSPlayer*)(nade->m_hOwnerEntity().Get()))->m_iTeamNum() == TEAM_CT ? WEAPON_FIREBOMB : WEAPON_MOLOTOV;
		}
		pMolotov = reinterpret_cast<C_Inferno*>(nade);
		if (pMolotov) {
			const auto spawn_time = pMolotov->m_flSpawnTime();
			const auto time = ((spawn_time + C_Inferno::GetExpiryTime()) - Interfaces::m_pGlobalVars->curtime);

			if (time <= 0.05f)
				dont_render = true;
		}
		else {
			dont_render = false;
		}
		break;
	case ClassId_t::CDecoyProjectile:
		Name = XorStr("o");
		item_definition = WEAPON_DECOY;
		break;
	default:
		return;
	}

	Vector2D points_transformed[8];
	BBox_t size;

	if (!GetBBox(nade, points_transformed, size) || dont_render)
		return;

	Render::Engine::cs_huge.string(size.x + 10, size.y - 15, Color(255, 255, 255, 220), Name.c_str(), Render::Engine::ALIGN_CENTER);
}

int orientation(ImVec2 p, ImVec2 q, ImVec2 r)
{
	int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;
	return (val > 0) ? 1 : 2;
}

std::vector<ImVec2> convexHull(std::vector<ImVec2> points, int n)
{
	std::vector<ImVec2> hull;

	if (n < 3) return hull;

	int l = 0;
	for (int i = 1; i < n; i++)
		if (points[i].x < points[l].x)
			l = i;
	int p = l, q;
	do
	{
		hull.push_back(points[p]);

		q = (p + 1) % n;
		for (int i = 0; i < n; i++)
		{
			if (orientation(points[p], points[i], points[q]) == 2)
				q = i;
		}
		p = q;

	} while (p != l);

	return hull;
}


void CEsp::RenderImGuiNades() {

	for (int i = 0; i <= Interfaces::m_pEntList->GetHighestEntityIndex(); ++i) {
		auto entity = (C_BaseEntity*)Interfaces::m_pEntList->GetClientEntity(i);

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		if (!entity)
			continue;

		if (!entity->GetClientClass() /*|| !entity->GetClientClass( )->m_ClassID*/)
			continue;

		if (g_Vars.esp.nades) {

			if (entity->GetClientClass()->m_ClassID == CInferno) {
				C_Inferno* pInferno = reinterpret_cast<C_Inferno*>(entity);
				C_CSPlayer* player = (C_CSPlayer*)entity->m_hOwnerEntity().Get();
				auto origin = pInferno->GetAbsOrigin();
				auto spawn_time = pInferno->m_flSpawnTime();
				auto factor = (spawn_time + C_Inferno::GetExpiryTime() - Interfaces::m_pGlobalVars->curtime) / C_Inferno::GetExpiryTime();
				Color col;
				if (player) {
					if (player->m_iTeamNum() == local->m_iTeamNum() && player->EntIndex() != local->EntIndex()) {
						col = Color(66, 123, 245, 204);
					}
					else {
						col = Color(255, 0, 0, 204);
					}
				}
				else
					col = Color(255, 0, 0, 204); // no player? wtf?

				auto dpos = origin;
				Vector mins, maxs;
				pInferno->GetClientRenderable()->GetRenderBounds(mins, maxs);

				int* m_fireXDelta = pInferno->m_fireXDelta();
				int* m_fireYDelta = pInferno->m_fireYDelta();
				int* m_fireZDelta = pInferno->m_fireZDelta();

				int dist = local->GetAbsOrigin().Distance(origin);

				if (dist > 2000)
					return;

				static const auto flame_polygon = [] {
					std::array<Vector, 3> points;
					for (std::size_t i = 0; i < points.size(); ++i) {
						points[i] = Vector{ 60.0f * std::cos(DEG2RAD(i * (360.0f / points.size()))),
											60.0f * std::sin(DEG2RAD(i * (360.0f / points.size()))),
											0.0f };
					}
					return points;
				}();

				std::vector<Vector> points;

				for (int i = 0; i <= pInferno->m_fireCount(); i++)
					points.push_back(entity->m_vecOrigin() + Vector(m_fireXDelta[i], m_fireYDelta[i], m_fireZDelta[i]));

				std::vector<ImVec2> screen_points;

				for (const auto& pos : points) {
					for (const auto& point : flame_polygon) {
						Vector2D screen;

						if (Render::Engine::WorldToScreen(pos + point, screen))
							screen_points.push_back(ImVec2(screen.x, screen.y));
					}
				}
				std::vector<ImVec2> hull_points;
				hull_points = convexHull(screen_points, screen_points.size());
				if (g_Vars.esp.molotov_radius) {
					if (!hull_points.empty())
						g_ImGuiRender->PolyLine(hull_points.data(), hull_points.size(), Color(col.r(), col.g(), col.b(), 255), true, 2.f, Color(col.r(), col.g(), col.b(), 35));
					else {
						auto usize = Vector(maxs - mins).Length2D() * 0.5;
						g_ImGuiRender->DrawRing3D(dpos.x, dpos.y, dpos.z, usize, 360, Color(col.r(), col.g(), col.b(), 255), Color(col.r(), col.g(), col.b(), 35), 2, factor);
					}
				}

				Vector2D screen_origin = Vector2D();

				if (!Render::Engine::WorldToScreen(origin, screen_origin))
					return;

				static auto size = Vector2D(35.0f, 5.0f);
				//g_Render->CircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 21, Color(25, 25, 25, col.a()), 60);
				g_ImGuiRender->FilledRect(screen_origin.x - 18.0f, screen_origin.y - size.y * 0.5f - 12.f, 32.f, 4.f, Color(0, 0, 0, 100), 3.f);
				g_ImGuiRender->FilledRect(screen_origin.x - 18.0f, screen_origin.y - size.y * 0.5f - 12.f, 32.f * factor, 4.f, Color(255, 255, 255), 3.f);
				g_ImGuiRender->DrawString(screen_origin.x, screen_origin.y - size.y * 0.5f - 25, Color(255, 255, 255), render2::centered_x | render2::centered_y | render2::outline, Menu::fonts.cs_huge, XorStr("n"));
			}

			if (entity->GetClientClass()->m_ClassID == CSmokeGrenadeProjectile) {
				C_SmokeGrenadeProjectile* smoke = reinterpret_cast<C_SmokeGrenadeProjectile*>(entity);

				if (!smoke->m_nSmokeEffectTickBegin())
					return;

				auto spawn_time = TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin());
				auto factor = (spawn_time + C_SmokeGrenadeProjectile::GetExpiryTime() - Interfaces::m_pGlobalVars->curtime) / C_SmokeGrenadeProjectile::GetExpiryTime();
				auto origin = smoke->GetAbsOrigin();

				int dist = local->GetAbsOrigin().Distance(origin);

				if (dist > 2000)
					return;

				g_ImGuiRender->DrawRing3D(origin.x, origin.y, origin.z, 150, 360, Color(255, 255, 255, 255), Color(255, 255, 255, 35), 2, factor, true);
				Vector2D screen_origin = Vector2D();

				if (!Render::Engine::WorldToScreen(origin, screen_origin))
					return;

				static auto size = Vector2D(35.0f, 5.0f);
				//g_ImGuiRender->CircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 19, Color(25, 25, 25, col.a()), 60);
				//g_ImGuiRender->two_sided_arc(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, 18, 1.f - factor, Color(col.r(), col.g(), col.b()), 2);
				g_ImGuiRender->DrawString(screen_origin.x, screen_origin.y - size.y * 0.5f - 12, Color(255, 255, 255), render2::centered_x | render2::centered_y | render2::outline, Menu::fonts.cs_huge, XorStr("m"));

			}
		}
	}
}

void CEsp::DrawBox( BBox_t bbox, const FloatColor& clr, C_CSPlayer* player ) {
	if( !player )
		return;

	FloatColor color;

	if (!player->IsDormant()) {
		color = clr;
	}
	else
		color = FloatColor(112, 112, 112, 180);

	color.a *= ( m_flAlpha[ player->EntIndex( ) ] );

	FloatColor outline = FloatColor( 0.0f, 0.0f, 0.0f, color.a * 0.68f );
	//if( g_Vars.esp.box_type == 0 ) {
	Render::Engine::Rect( bbox.x - 1, bbox.y - 1, bbox.w + 2, bbox.h + 2, Color( 0, 0, 0, 180 * m_flAlpha[ player->EntIndex( ) ] ) );
	Render::Engine::Rect( bbox.x + 1, bbox.y + 1, bbox.w - 2, bbox.h - 2, Color( 0, 0, 0, 180 * m_flAlpha[ player->EntIndex( ) ] ) );
	Render::Engine::Rect( bbox.x, bbox.y, bbox.w, bbox.h, color.ToRegularColor( ) );
	//}
}

void CEsp::DrawHealthBar( C_CSPlayer* player, BBox_t bbox ) {
	int y = bbox.y + 1;
	int h = bbox.h - 2;

	// retarded servers that go above 100 hp..
	int hp = std::min( 100, player->m_iHealth( ) );

	//https://yougame.biz/threads/252733/#post-2601993

	// initialize our previous hp value ( we will use this later as this will be the value we interpolate )
	static float m_flPreviousHP[65] = { };
	// anim speed.
	constexpr float SPEED_FREQ = 255 / 1.f;

	// if our stored value is greater than stored hp value then we want to decrement it as shown below until we reach
	// the current hp value giving us our super cool haxor smoothed/interpolated health value
	if (m_flPreviousHP[player->EntIndex()] > hp)
		m_flPreviousHP[player->EntIndex()] -= SPEED_FREQ * Interfaces::m_pGlobalVars->frametime;
	else
		m_flPreviousHP[player->EntIndex()] = hp;

	int hp_animated = m_flPreviousHP[player->EntIndex()];


	// calculate hp bar color.
	int r = std::min( ( 510 * ( 100 - hp_animated ) ) / 100, 255 );
	int g = std::min( ( 510 * hp_animated ) / 100, 255 );

	// get hp bar height.
	int fill = ( int )std::round( hp_animated * h / 100.f );

	// render background.
	Render::Engine::RectFilled( bbox.x - 6, y - 1, 4, h + 2, Color( 10, 10, 10, 180 * GetAlpha( player->EntIndex( ) ) ) );

	// render actual bar.
	if (!player->IsDormant()) {
		Render::Engine::Rect(bbox.x - 5, y + h - fill, 2, fill, g_Vars.esp.health_override ? g_Vars.esp.health_color.ToRegularColor().OverrideAlpha(210 * GetAlpha(player->EntIndex()), true)
			: Color(r, g, 0, 210 * GetAlpha(player->EntIndex())));
	}
	else {
		Render::Engine::Rect(bbox.x - 5, y + h - fill, 2, fill, Color(112, 112, 112, 210 * GetAlpha(player->EntIndex())));
	}

	// if hp is below max, draw a string.
	if( hp_animated < 100 ) {
		if (!player->IsDormant()) {
			Render::Engine::pixel_reg.string(bbox.x - 5, y + (h - fill) - 5, Color(255, 255, 255, 200 * GetAlpha(player->EntIndex())), std::to_string(hp), Render::Engine::ALIGN_CENTER);
		}
		else
			Render::Engine::pixel_reg.string(bbox.x - 5, y + (h - fill) - 5, Color(112, 112, 112, 200 * GetAlpha(player->EntIndex())), std::to_string(hp), Render::Engine::ALIGN_CENTER);
	}
}

void CEsp::DrawInfo( C_CSPlayer* player, BBox_t bbox, player_info_t player_info ) {
	auto animState = player->m_PlayerAnimState( );
	if( !animState )
		return;

	auto color = FloatColor( 0, 150, 255, ( int )( 180 * m_flAlpha[ player->EntIndex( ) ] ) );
	color.a *= m_flAlpha[ player->EntIndex( ) ];

	auto anim_data = Engine::AnimationSystem::Get( )->GetAnimationData( player->m_entIndex );
	auto lag_data = Engine::LagCompensation::Get( )->GetLagData( player->m_entIndex );
	if( anim_data && lag_data.IsValid( ) && g_Vars.esp.draw_resolver ) {
		if( !anim_data->m_AnimationRecord.empty( ) ) {
			auto current = &anim_data->m_AnimationRecord.front( );
			if( current ) {
				if (!player->IsDormant()) {
					if (current->m_bResolved)
						g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(0, 255, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("R"));
					else
						g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 0, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("R"));
				}
				else {
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("R"));
				}

				//g_Vars.globals.m_vecTextInfo[ player->EntIndex( ) ].emplace_back( FloatColor( 255, 255, 255, ( int )( 180 * m_flAlpha[ player->EntIndex( ) ] ) ), std::to_string( current->m_iResolverMode ) );
			}
		}
	}

	if (g_Vars.rage.override_reoslver.enabled && Engine::g_ResolverData->m_bInOverride[player->EntIndex()]) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("OVERRIDE"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("OVERRIDE"));
	}

	if (g_Vars.esp.draw_ping) {
		if (!player->IsDormant()) {
			if (((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) <= 99) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) + XorStr("MS"));
			}
			else if (((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) <= 199) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 128, 0, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) + XorStr("MS"));
			}
			else if (((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) <= 999) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 0, 0, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) + XorStr("MS"));
			}
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string((*Interfaces::m_pPlayerResource.Xor())->GetPlayerPing(player->EntIndex())) + XorStr("MS"));
	}


	if (player->m_bIsScoped() && g_Vars.esp.draw_scoped) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(0, 150, 255, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("SCOPED"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("SCOPED"));
	}

	if (g_Vars.esp.draw_money) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(133, 198, 22, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("$") + std::to_string(player->m_iAccount()));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("$") + std::to_string(player->m_iAccount()));
	}

	if( g_Vars.esp.draw_armor && player->m_ArmorValue( ) > 0 ) {
		std::string name = player->m_bHasHelmet( ) ? XorStr( "HK" ) : XorStr( "K" );
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), name.c_str());
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), name.c_str());
	}

	if( g_Vars.esp.draw_defusing && player->m_bHasDefuser( ) ) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(105, 218, 204, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("KIT"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("KIT"));
	}

	if (g_Vars.esp.draw_flashed && player->m_flFlashDuration() > 1.f) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 216, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("FLASH"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("FLASH"));
	}

	if( g_Vars.esp.draw_defusing && player->m_bIsDefusing( ) ) {
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(235, 82, 82, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("DEFUSER"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("DEFUSER"));
	}

	if (g_Vars.esp.draw_exploiting && (player->m_flOldSimulationTime() > player->m_flSimulationTime()))
	{
		if (!player->IsDormant()) {
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("EXPLOIT"));
		}
		else
			g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("EXPLOIT"));
	}

	if (g_Vars.misc.ingame_radar) {
		player->m_bSpotted() = true;
	}
#if defined(BETA_MODE) || defined(DEV)
	if (g_Vars.misc.resolver_flags) {
		if (anim_data && lag_data.IsValid() && !anim_data->m_AnimationRecord.empty()) {
			auto current = &anim_data->m_AnimationRecord.front();
			if (current) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 0, 0, (int)(180 * m_flAlpha[player->EntIndex()])), current->m_iResolverText); // draw resolver mode
				if (current->m_bFakeWalking) {
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("FAKEWALKING")); // draw fakewalk
				}
				if (current->m_bUnsafeVelocityTransition) {
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(0, 255, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("MICROMOVE")); // draw micromove
				}
			}
		}
	}
#endif

#if defined(BETA_MODE) || defined(DEV) 

	if (!g_Vars.misc.undercover_flags) {
		//g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[12].m_flWeight));
		//g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[6].m_flWeight));
		//g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[6].m_flCycle));
		//g_Vars.globals.m_vecTextInfo[ player->EntIndex( ) ].emplace_back( FloatColor( 255, 255, 255, ( int )( 180 * m_flAlpha[ player->EntIndex( ) ] ) ), std::to_string( player->m_AnimOverlay( )[ 6 ].m_flPrevCycle ) );
		//g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[6].m_flPlaybackRate));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_flPlaybackRate));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_flWeight));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_flCycle));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_flPrevCycle));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_flWeightDeltaRate));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_nOrder));
		g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), std::to_string(player->m_AnimOverlay()[3].m_nSequence));
		//g_Vars.globals.m_vecTextInfo[ player->EntIndex( ) ].emplace_back( FloatColor( 255, 255, 255, ( int )( 180 * m_flAlpha[ player->EntIndex( ) ] ) ), std::to_string( player->m_AnimOverlay( )[ 6 ].m_flWeightDeltaRate ) );
	}

#endif


	if( g_Vars.esp.draw_distance ) {
		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if( local && !local->IsDead( ) ) {
			auto round_to_multiple = [ & ] ( int in, int multiple ) {
				const auto ratio = static_cast< double >( in ) / multiple;
				const auto iratio = std::lround( ratio );
				return static_cast< int >( iratio * multiple );
			};

			float distance = local->m_vecOrigin( ).Distance( player->m_vecOrigin( ) );

			auto meters = distance * 0.0254f;
			auto feet = meters * 3.281f;

			std::string str = std::to_string( round_to_multiple( feet, 5 ) ) + XorStr( " FT" );
			if (!player->IsDormant()) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), str.c_str());
			}
			else
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), str.c_str());
		}
	}

	auto weapons = player->m_hMyWeapons( );
	for( size_t i = 0; i < 48; ++i ) {
		auto weapon_handle = weapons[ i ];
		if( !weapon_handle.IsValid( ) )
			break;

		auto weapon = ( C_BaseCombatWeapon* )weapon_handle.Get( );
		if( !weapon )
			continue;

		auto definition_index = weapon->m_Item( ).m_iItemDefinitionIndex( );

		if (definition_index == WEAPON_C4 && g_Vars.esp.draw_bombc4) {
			if (!player->IsDormant()) {
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 255, 255, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("C4"));
			}
			else
				g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("C4"));
		}
	}

	auto pWeapon = ( C_WeaponCSBaseGun* )player->m_hActiveWeapon( ).Get( );
	if( g_Vars.esp.draw_grenade_pin && pWeapon ) {
		auto pWeaponData = pWeapon->GetCSWeaponData( );
		if( pWeaponData.IsValid( ) ) {
			if( pWeaponData->m_iWeaponType == WEAPONTYPE_GRENADE && pWeapon->m_bPinPulled( ) ) {
				if (!player->IsDormant()) {
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(255, 0, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("PIN"));
				}
				else
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("PIN"));
			}
		}
	}

	if (g_Vars.esp.draw_vulnerable && pWeapon)
	{
		auto pWeaponData = pWeapon->GetCSWeaponData( );
		if (pWeaponData.IsValid()) {
			if (pWeapon->GetCSWeaponData()->m_iWeaponType == WEAPONTYPE_C4 || pWeapon->GetCSWeaponData()->m_iWeaponType == WEAPONTYPE_GRENADE || pWeapon->GetCSWeaponData()->m_iWeaponType == WEAPONTYPE_KNIFE) {
				if (!player->IsDormant()) {
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(120, 240, 0, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("VULNERABLE"));
				}
				else
					g_Vars.globals.m_vecTextInfo[player->EntIndex()].emplace_back(FloatColor(112, 112, 112, (int)(180 * m_flAlpha[player->EntIndex()])), XorStr("VULNERABLE"));
			}
		}
	}

	int i = 0;
	for( auto text : g_Vars.globals.m_vecTextInfo[ player->EntIndex( ) ] ) {
		Render::Engine::pixel_reg.string( bbox.x + bbox.w + 3, bbox.y + i, text.first.ToRegularColor( ), text.second.c_str( ) );
		i += ( Render::Engine::pixel_reg.m_size.m_height - 1 );
	}
}

std::string GetLocalizedName( CCSWeaponInfo* wpn_data ) {
	if( !wpn_data )
		return XorStr( "ERROR" );

	return Math::WideToMultiByte( Interfaces::m_pLocalize->Find( wpn_data->m_szHudName ) );
}

void CEsp::DrawBottomInfo( C_CSPlayer* player, BBox_t bbox, player_info_t player_info ) {
	std::vector<std::pair<Color, std::pair<std::string, Render::Engine::Font>>> m_vecTextInfo;

	auto pWeapon = ( C_WeaponCSBaseGun* )( player->m_hActiveWeapon( ).Get( ) );
	Color color;
	if (!player->IsDormant()) {
		color = g_Vars.esp.weapon_color.ToRegularColor().OverrideAlpha(180, true);
	}
	else
		color = Color(112, 112, 112, 180);

	color.RGBA[ 3 ] *= m_flAlpha[ player->EntIndex( ) ];

	Color color_icon;
	if (!player->IsDormant()) {
		color_icon = g_Vars.esp.weapon_icon_color.ToRegularColor().OverrideAlpha(180, true);
	}
	else
		color_icon = Color(112, 112, 112, 180);

	color_icon.RGBA[ 3 ] *= m_flAlpha[ player->EntIndex( ) ];

	if( !pWeapon )
		return;

	auto pWeaponData = pWeapon->GetCSWeaponData( );
	if( !pWeaponData.IsValid( ) )
		return;

	float i = 2.f;

	if( g_Vars.esp.draw_ammo_bar && ( pWeaponData->m_iWeaponType != WEAPONTYPE_GRENADE && pWeaponData->m_iWeaponType != WEAPONTYPE_KNIFE && pWeaponData->m_iWeaponType != WEAPONTYPE_C4 ) )
		i += 6.f;

	if( g_Vars.esp.draw_lby_bar )
		i += 6.f;

	if( g_Vars.esp.weapon_icon ) {
		m_vecTextInfo.emplace_back( color_icon, std::pair{ GetWeaponIcon( pWeapon->m_iItemDefinitionIndex( ) ), Render::Engine::cs } );
	}

	if( g_Vars.esp.weapon ) {
		std::string name{ GetLocalizedName( pWeaponData.Xor( ) ) };

		std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

		m_vecTextInfo.emplace_back( color, std::pair{ name, Render::Engine::pixel_reg } );
	}

	for( auto text : m_vecTextInfo ) {
		if( text.second.second.m_handle == Render::Engine::pixel_reg.m_handle ) {
			text.second.second.string( bbox.x + bbox.w / 2, ( bbox.y + bbox.h - 2 ) + i, text.first, text.second.first.c_str( ), Render::Engine::ALIGN_CENTER );
		}
		else {
			text.second.second.string( bbox.x + bbox.w / 2, bbox.y + bbox.h + i, text.first, text.second.first.c_str( ), Render::Engine::ALIGN_CENTER );
		}

		i += text.second.second.m_size.m_height;
	}
}

void CEsp::DrawTeamName(C_CSPlayer* player, BBox_t bbox, player_info_t player_info) {
	// fix retards with their namechange meme 
	// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat

	auto local = C_CSPlayer::GetLocalPlayer();
	if (!local)
		return;

	if (!player->IsTeammate(local))
		return;

	if (player->EntIndex() == local->EntIndex())
		return;

	std::string name;

	name += XorStr("[TEAMMATE] ");

	if (!g_Vars.globals.vader_user.empty()) {
		if (std::find(g_Vars.globals.vader_user.begin(), g_Vars.globals.vader_user.end(), player_info.userId) != g_Vars.globals.vader_user.end()) {

			name += XorStr("[vader] ");
		}
	}

	if (!g_Vars.globals.vader_beta.empty()) {
		if (std::find(g_Vars.globals.vader_beta.begin(), g_Vars.globals.vader_beta.end(), player_info.userId) != g_Vars.globals.vader_beta.end()) {

			name += XorStr("[vader beta] ");
		}
	}

	if (!g_Vars.globals.vader_dev.empty()) {
		if (std::find(g_Vars.globals.vader_dev.begin(), g_Vars.globals.vader_dev.end(), player_info.userId) != g_Vars.globals.vader_dev.end()) {

			name += XorStr("[vader dev] ");
		}
	}

	if (!g_Vars.globals.vader_crack.empty()) {
		if (std::find(g_Vars.globals.vader_crack.begin(), g_Vars.globals.vader_crack.end(), player_info.userId) != g_Vars.globals.vader_crack.end()) {

			name += XorStr("[crack] ");
		}
	}

	name += std::string(player_info.szName).substr(0, 24);

	//#if defined (DEV)
	//	name.append( XorStr( " (" ) ).append( std::to_string( player->m_entIndex ) ).append( XorStr( ")" ) );
	//#endif

	Color clr;

	if (player_info.steamID64 == 76561199057465290) {
		static float rainbow;
		rainbow += 0.001f;
		if (rainbow > 1.f)
			rainbow = 0.f;

		clr = Color::HSBtoRGB(rainbow, 1.0f, 1.0f);
	}
	else {
		if (!player->IsDormant()) {
			clr = g_Vars.esp.team_name_color.ToRegularColor().OverrideAlpha(180, true);
		}
		else
			clr = Color(0, 255, 221, 180);
	}

	clr.RGBA[3] *= m_flAlpha[player->EntIndex()];

	Render::Engine::tahoma_sexy.string(bbox.x + bbox.w / 2, bbox.y - Render::Engine::tahoma_sexy.m_size.m_height - 1, clr, name, Render::Engine::ALIGN_CENTER);

	/*if( g_Vars.globals.nOverrideEnemy == player->EntIndex( ) ) {
		Color clr = Color( 255, 255, 255, 180 );
		if( g_Vars.globals.nOverrideLockedEnemy == player->EntIndex( ) ) {
			clr = Color( 255, 0, 0, 180 );
		}

		clr.RGBA[ 3 ] *= m_flAlpha[ player->EntIndex( ) ];

		name = XorStr( "OVERRIDE" );
		Render::Engine::segoe.string( bbox.x + bbox.w / 2, ( bbox.y - Render::Engine::segoe.m_size.m_height - 1 ) - 14, clr, name, Render::Engine::ALIGN_CENTER );
	}*/
}

void CEsp::DrawName( C_CSPlayer* player, BBox_t bbox, player_info_t player_info ) {
	// fix retards with their namechange meme 
	// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat

	std::string name;

	if (!g_Vars.globals.vader_user.empty()) {
		if (std::find(g_Vars.globals.vader_user.begin(), g_Vars.globals.vader_user.end(), player_info.userId) != g_Vars.globals.vader_user.end()) {

			name += XorStr("[vader] ");
		}
	}

	if (!g_Vars.globals.vader_beta.empty()) {
		if (std::find(g_Vars.globals.vader_beta.begin(), g_Vars.globals.vader_beta.end(), player_info.userId) != g_Vars.globals.vader_beta.end()) {

			name += XorStr("[vader beta] ");
		}
	}

	if (!g_Vars.globals.vader_dev.empty()) {
		if (std::find(g_Vars.globals.vader_dev.begin(), g_Vars.globals.vader_dev.end(), player_info.userId) != g_Vars.globals.vader_dev.end()) {

			name += XorStr("[vader dev] ");
		}
	}

	if (!g_Vars.globals.vader_crack.empty()) {
		if (std::find(g_Vars.globals.vader_crack.begin(), g_Vars.globals.vader_crack.end(), player_info.userId) != g_Vars.globals.vader_crack.end()) {

			name += XorStr("[crack] ");
		}
	}

	name += std::string( player_info.szName ).substr( 0, 24 );

	Color clr;

	if (player_info.steamID64 == 76561199057465290) {
		static float rainbow;
		rainbow += 0.001f;
		if (rainbow > 1.f)
			rainbow = 0.f;

		clr = Color::HSBtoRGB(rainbow, 1.0f, 1.0f);
	}
	else {
		if (!player->IsDormant()) {
			clr = g_Vars.esp.name_color.ToRegularColor().OverrideAlpha(180, true);
		}
		else
			clr = Color(112, 112, 112, 180);
	}

	clr.RGBA[3] *= m_flAlpha[player->EntIndex()];

	Render::Engine::tahoma_sexy.string( bbox.x + bbox.w / 2, bbox.y - Render::Engine::tahoma_sexy.m_size.m_height - 1, clr, name, Render::Engine::ALIGN_CENTER );

	/*if( g_Vars.globals.nOverrideEnemy == player->EntIndex( ) ) {
		Color clr = Color( 255, 255, 255, 180 );
		if( g_Vars.globals.nOverrideLockedEnemy == player->EntIndex( ) ) {
			clr = Color( 255, 0, 0, 180 );
		}

		clr.RGBA[ 3 ] *= m_flAlpha[ player->EntIndex( ) ];

		name = XorStr( "OVERRIDE" );
		Render::Engine::segoe.string( bbox.x + bbox.w / 2, ( bbox.y - Render::Engine::segoe.m_size.m_height - 1 ) - 14, clr, name, Render::Engine::ALIGN_CENTER );
	}*/
}

void CEsp::DrawSkeleton( C_CSPlayer* player ) {
	auto model = player->GetModel( );
	if( !model )
		return;

	if( player->IsDormant( ) )
		return;

	auto* hdr = Interfaces::m_pModelInfo->GetStudiomodel( model );
	if( !hdr )
		return;

	// render skeleton
	Vector2D bone1, bone2;
	for( size_t n{ }; n < hdr->numbones; ++n ) {
		auto* bone = hdr->pBone( n );
		if( !bone || !( bone->flags & 256 ) || bone->parent == -1 ) {
			continue;
		}

		auto BonePos = [ & ] ( int n ) -> Vector {
			return Vector(
				player->m_CachedBoneData( ).m_Memory.m_pMemory[ n ][ 0 ][ 3 ],
				player->m_CachedBoneData( ).m_Memory.m_pMemory[ n ][ 1 ][ 3 ],
				player->m_CachedBoneData( ).m_Memory.m_pMemory[ n ][ 2 ][ 3 ]
			);
		};

		if( !WorldToScreen( BonePos( n ), bone1 ) || !WorldToScreen( BonePos( bone->parent ), bone2 ) ) {
			continue;
		}

		auto color = g_Vars.esp.skeleton_color.ToRegularColor( ).OverrideAlpha( 180, true );
		color.RGBA[ 3 ] *= m_flAlpha[ player->EntIndex( ) ];

		Render::Engine::Line( bone1, bone2, color );
	}
}

void CEsp::DrawHitSkeleton( ) {
	auto pLocal = C_CSPlayer::GetLocalPlayer( );

	if( !g_Vars.globals.HackIsReady || !Interfaces::m_pEngine->IsConnected( ) || !Interfaces::m_pEngine->IsInGame( ) || !pLocal || !pLocal->IsAlive() ) {
		m_Hitmatrix.clear( );
		return;
	}

	if( m_Hitmatrix.empty( ) )
		return;

	Vector2D bone1, bone2;
	for( size_t i{ }; i < m_Hitmatrix.size( ); ++i ) {
		auto delta = Interfaces::m_pGlobalVars->realtime - m_Hitmatrix[ i ].m_flTime;
		if( delta > 0.0f && delta < 1.0f ) {
			m_Hitmatrix[ i ].m_flAlpha -= delta;
		}

		if( m_Hitmatrix[ i ].m_flAlpha <= 0.0f || !m_Hitmatrix[ i ].pBoneToWorld || !m_Hitmatrix[ i ].m_pEntity->GetModel( ) || !m_Hitmatrix[ i ].m_pEntity ) {
			continue;
		}

		auto* model = Interfaces::m_pModelInfo->GetStudiomodel( m_Hitmatrix[ i ].m_pEntity->GetModel( ) );
		if( !model ) {
			continue;
		}

		// render hurt skeleton
		for( size_t n{ }; n < model->numbones; ++n ) {
			auto* bone = model->pBone( n );
			if( !bone || !( bone->flags & 256 ) || bone->parent == -1 ) {
				continue;
			}

			auto BonePos = [ & ] ( int n ) -> Vector {
				return Vector(
					m_Hitmatrix[ i ].pBoneToWorld[ n ][ 0 ][ 3 ],
					m_Hitmatrix[ i ].pBoneToWorld[ n ][ 1 ][ 3 ],
					m_Hitmatrix[ i ].pBoneToWorld[ n ][ 2 ][ 3 ]
				);
			};

			if( !WorldToScreen( BonePos( n ), bone1 ) || !WorldToScreen( BonePos( bone->parent ), bone2 ) ) {
				continue;
			}

			Render::Engine::Line( bone1, bone2, g_Vars.esp.hitskeleton_color.ToRegularColor( ).OverrideAlpha( 220 * m_Hitmatrix[ i ].m_flAlpha, true ) );
		}
	}
}

void CEsp::DrawLocalSkeleton() {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	Vector        bone_pos, parent_pos;
	Vector2D        bone_pos_screen, parent_pos_screen;

	auto pLocal = C_CSPlayer::GetLocalPlayer();

	if (!pLocal || !pLocal->IsAlive())
		return;

	if (g_Vars.misc.third_person_bind.enabled && g_Vars.misc.third_person && g_Vars.esp.local_skeleton) {

		// get player's model.
		model = pLocal->GetModel();
		if (!model)
			return;

		// get studio model.
		hdr = Interfaces::m_pModelInfo->GetStudiomodel(model);
		if (!hdr)
			return;

		// get bone matrix.
		if (!pLocal->SetupBones(matrix, 128, BONE_USED_BY_ANYTHING, Interfaces::m_pGlobalVars->curtime))
			return;

		for (int i{ }; i < hdr->numbones; ++i) {
			// get bone.
			bone = hdr->pBone(i);
			if (!bone || !(bone->flags & BONE_USED_BY_HITBOX))
				continue;

			// get parent bone.
			parent = bone->parent;
			if (parent == -1)
				continue;

			// resolve main bone and parent bone positions.
			matrix->GetBone(bone_pos, i);
			matrix->GetBone(parent_pos, parent);

			Color clr = g_Vars.esp.local_skeleton_color.ToRegularColor();

			// world to screen both the bone parent bone then draw.
			if (Render::Engine::WorldToScreen(bone_pos, bone_pos_screen) && Render::Engine::WorldToScreen(parent_pos, parent_pos_screen))
				Render::Engine::Line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
		}

		//for (int j = 0; j < hdr->numbones; j++) // fixed shoulders but because of that breaks head bone.
		//{
		//	mstudiobone_t* pBone = hdr->pBone(j);

		//	// get bone.
		//	bone = hdr->pBone(j);
		//	if (!bone || !(bone->flags & BONE_USED_BY_HITBOX | BONE_USED_BY_ATTACHMENT))
		//		continue;

		//	// get parent bone.
		//	parent = bone->parent;
		//	if (parent == -1)
		//		continue;

		//	if(j == 83 || j == 85 || j == 76 || j == 75 || j == 68 || j == 69 || j == 9 || j == 37 || j == 87 || j == 88)
		//		continue;

		//	// resolve main bone and parent bone positions.
		//	matrix->GetBone(bone_pos, j);
		//	matrix->GetBone(parent_pos, parent);

		//	//bone_pos = pLocal->GetBonePos(j);
		//	//parent_pos = pLocal->GetBonePos(pBone->parent);

		//	int iChestBone = 6;  // Parameter of relevant Bone number
		//	Vector vBreastBone; // New reference Point for connecting many bones
		//	Vector vUpperDirection = pLocal->GetBonePos(iChestBone + 1) - pLocal->GetBonePos(iChestBone); // direction vector from chest to neck
		//	vBreastBone = pLocal->GetBonePos(iChestBone) + vUpperDirection / 2;
		//	Vector vDeltaChild = bone_pos - vBreastBone; // Used to determine close bones to the reference point
		//	Vector vDeltaParent = parent_pos - vBreastBone;

		//	// Eliminating / Converting all disturbing bone positions in three steps.
		//	if ((vDeltaParent.Length() < 10 && vDeltaChild.Length() < 10) && !(j == 63 || j == 35))
		//		parent_pos = vBreastBone;

		//	if (j == iChestBone - 1)
		//		bone_pos = vBreastBone;

		//	if (j == iChestBone || j == 7)
		//		continue;

		//	//if ((pBone->flags & BONE_USED_BY_HITBOX ^ BONE_USED_BY_HITBOX) && (vDeltaParent.Length() < 19 && vDeltaChild.Length() < 19))
		//	//	continue;

		//	Color clr = g_Vars.esp.local_skeleton_color.ToRegularColor();

		//	if (Render::Engine::WorldToScreen(bone_pos, bone_pos_screen) && Render::Engine::WorldToScreen(parent_pos, parent_pos_screen)) {
		//		Render::Engine::Line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
		//		//Render::Engine::esp.string(bone_pos_screen.x, bone_pos_screen.y, Color(255, 0, 25, 255), std::to_string(j));
		//	}
		//}

	}
}


void CEsp::Offscreen( ) {
	C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer( );

	if( !LocalPlayer )
		return;

	QAngle viewangles;
	int width = Render::GetScreenSize( ).x, height = Render::GetScreenSize( ).y;

	Interfaces::m_pEngine.Xor( )->GetViewAngles( viewangles );

	auto rotate_arrow = [ ] ( std::array< Vector2D, 3 >& points, float rotation ) {
		const auto points_center = ( points.at( 0 ) + points.at( 1 ) + points.at( 2 ) ) / 3;
		for( auto& point : points ) {
			point -= points_center;

			const auto temp_x = point.x;
			const auto temp_y = point.y;

			const auto theta = DEG2RAD( rotation );
			const auto c = cos( theta );
			const auto s = sin( theta );

			point.x = temp_x * c - temp_y * s;
			point.y = temp_x * s + temp_y * c;

			point += points_center;
		}
	};

	auto m_width = Render::GetScreenSize( ).x, m_height = Render::GetScreenSize( ).y;
	for( auto i = 1; i <= Interfaces::m_pGlobalVars->maxClients; i++ ) {
		auto entity = C_CSPlayer::GetPlayerByIndex( i );
		if( !entity || !entity->IsPlayer( ) || entity == LocalPlayer || entity->IsDead( )
			|| ( g_Vars.esp.team_check && entity->m_iTeamNum( ) == LocalPlayer->m_iTeamNum( ) ) )
			continue;

		// get the player's center screen position.
		auto target_pos = entity->WorldSpaceCenter( );
		Vector2D screen_pos;
		auto is_on_screen = WorldToScreen( target_pos, screen_pos );

		// give some extra room for screen position to be off screen.
		auto leeway_x = m_width / 18.f;
		auto leeway_y = m_height / 18.f;

		if( !is_on_screen
			|| screen_pos.x < -leeway_x
			|| screen_pos.x >( m_width + leeway_x )
			|| screen_pos.y < -leeway_y
			|| screen_pos.y >( m_height + leeway_y ) ) {

			const auto screen_center = Vector2D( width * .5f, height * .5f );
			const auto angle_yaw_rad = DEG2RAD( viewangles.y - Math::CalcAngle( LocalPlayer->GetEyePosition( ), entity->WorldSpaceCenter( ), true ).y - 90 );

			float radius = std::max( 10.f, g_Vars.esp.offscren_distance );
			float size = std::max( 5.f, g_Vars.esp.offscren_size );

			const auto new_point_x = screen_center.x + ( ( ( ( width - ( size * 3 ) ) * .5f ) * ( radius / 100.0f ) ) * cos( angle_yaw_rad ) ) + ( int )( 6.0f * ( ( ( float )size - 4.f ) / 16.0f ) );
			const auto new_point_y = screen_center.y + ( ( ( ( height - ( size * 3 ) ) * .5f ) * ( radius / 100.0f ) ) * sin( angle_yaw_rad ) );

			std::array< Vector2D, 3 >points{ Vector2D( new_point_x - size, new_point_y - size ),
				Vector2D( new_point_x + size, new_point_y ),
				Vector2D( new_point_x - size, new_point_y + size ) };

			rotate_arrow( points, viewangles.y - Math::CalcAngle( LocalPlayer->GetEyePosition( ), entity->WorldSpaceCenter( ), true ).y - 90 );

			std::array< Vertex_t, 3 >vertices{ Vertex_t( points.at( 0 ) ), Vertex_t( points.at( 1 ) ), Vertex_t( points.at( 2 ) ) };
			static int texture_id = Interfaces::m_pSurface.Xor( )->CreateNewTextureID( true );
			static unsigned char buf[ 4 ] = { 255, 255, 255, 255 };

			Color clr;

			if (!entity->IsDormant()) {
				clr = g_Vars.esp.offscreen_color.ToRegularColor();
			}
			else {
				clr = Color(255, 255, 255, 44);
			}

			// fill
			Interfaces::m_pSurface.Xor( )->DrawSetColor( clr.r( ), clr.g( ), clr.b( ), ( clr.a( ) ) * GetAlpha( i ) );
			Interfaces::m_pSurface.Xor( )->DrawSetTexture( texture_id );
			Interfaces::m_pSurface.Xor( )->DrawTexturedPolygon( 3, vertices.data( ) );

			// outline
			//Interfaces::m_pSurface.Xor( )->DrawSetColor( clr.r( ), clr.g( ), clr.b( ), ( clr.a( ) ) * GetAlpha( i ) );
			//Interfaces::m_pSurface.Xor( )->DrawSetTexture( texture_id );
			//Interfaces::m_pSurface.Xor( )->DrawTexturedPolyLine( vertices.data( ), 3 );
		}
	}
}

void CEsp::OverlayInfo( ) {
	auto local = C_CSPlayer::GetLocalPlayer( );
	if( !local )
		return;

	C_WeaponCSBaseGun* weapon = ( C_WeaponCSBaseGun* )local->m_hActiveWeapon( ).Get( );
	if( !weapon )
		return;

	if( g_Vars.esp.offscren_enabled )
		Offscreen( );

	PenetrateCrosshair( Render::GetScreenSize( ) / 2 );
}

bool CEsp::GetBBox( C_BaseEntity* entity, Vector2D screen_points[ ], BBox_t& outRect ) {
	BBox_t rect{ };
	auto collideable = entity->GetCollideable( );

	if( !collideable )
		return false;

	if( entity->IsPlayer( ) ) {
		Vector origin, mins, maxs;
		Vector2D bottom, top;

		// get interpolated origin.
		origin = entity->GetAbsOrigin( );

		// get hitbox bounds.
		entity->ComputeHitboxSurroundingBox( &mins, &maxs );

		// correct x and y coordinates.
		mins = { origin.x, origin.y, mins.z };
		maxs = { origin.x, origin.y, maxs.z + 8.f };

		if( !WorldToScreen( mins, bottom ) || !WorldToScreen( maxs, top ) )
			return false;

		// state the box bounds
		outRect.h = bottom.y - top.y;
		outRect.w = outRect.h / 2.f;
		outRect.x = bottom.x - ( outRect.w / 2.f );
		outRect.y = bottom.y - outRect.h;

		return true;
	}
	else {
		auto min = collideable->OBBMins( );
		auto max = collideable->OBBMaxs( );

		const matrix3x4_t& trans = entity->m_rgflCoordinateFrame( );

		Vector points[ ] =
		{
			Vector( min.x, min.y, min.z ),
			Vector( min.x, max.y, min.z ),
			Vector( max.x, max.y, min.z ),
			Vector( max.x, min.y, min.z ),
			Vector( max.x, max.y, max.z ),
			Vector( min.x, max.y, max.z ),
			Vector( min.x, min.y, max.z ),
			Vector( max.x, min.y, max.z )
		};

		for( int i = 0; i < 8; i++ ) {
			points[ i ] = points[ i ].Transform( trans );
		}

		for( int i = 0; i < 8; i++ )
			if( !WorldToScreen( points[ i ], screen_points[ i ] ) )
				return false;

		auto left = screen_points[ 0 ].x;
		auto top = screen_points[ 0 ].y;
		auto right = screen_points[ 0 ].x;
		auto bottom = screen_points[ 0 ].y;

		for( int i = 1; i < 8; i++ ) {
			if( left > screen_points[ i ].x )
				left = screen_points[ i ].x;
			if( top < screen_points[ i ].y )
				top = screen_points[ i ].y;
			if( right < screen_points[ i ].x )
				right = screen_points[ i ].x;
			if( bottom > screen_points[ i ].y )
				bottom = screen_points[ i ].y;
		}

		left = std::ceilf( left );
		top = std::ceilf( top );
		right = std::floorf( right );
		bottom = std::floorf( bottom );

		// state the box bounds.
		outRect.x = left;
		outRect.y = top;
		outRect.w = right - left;
		outRect.h = ( bottom - top );
		return true;
	}

	return false;
}

Encrypted_t<IEsp> IEsp::Get( ) {
	static CEsp instance;
	return &instance;
}