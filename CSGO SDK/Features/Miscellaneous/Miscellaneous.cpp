#include "Miscellaneous.hpp"
#include "../../SDK/displacement.hpp"

#include "../../Source.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../Game/Prediction.hpp"

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

const unsigned short INVALID_STRING_INDEX = (unsigned short)-1;

bool PrecacheModels(const char* szModelName)
{
	INetworkStringTable* m_pModelPrecacheTable = Interfaces::g_pClientStringTableContainer->FindTable(XorStr("modelprecache"));

	if (m_pModelPrecacheTable)
	{
		Interfaces::m_pModelInfo->FindOrLoadModel(szModelName);
		int idx = m_pModelPrecacheTable->AddString(false, szModelName);
		if (idx == INVALID_STRING_INDEX)
			return false;
	}
	return true;
}


namespace Interfaces
{
	inline float rgb_to_srgb( float flLinearValue ) {
		return flLinearValue;
		// float x = Math::Clamp( flLinearValue, 0.0f, 1.0f );
		// return ( x <= 0.0031308f ) ? ( x * 12.92f ) : ( 1.055f * powf( x, ( 1.0f / 2.4f ) ) ) - 0.055f;
	}

	class C_Miscellaneous : public Miscellaneous {
	public:
		static Miscellaneous* Get( );
		virtual void Main( );

		C_Miscellaneous( ) { };
		virtual ~C_Miscellaneous( ) {
		}

	private:
		const char* skynames[ 16 ] = {
			"Default",
			"cs_baggage_skybox_",
			"cs_tibet",
			"embassy",
			"italy",
			"jungle",
			"nukeblank",
			"office",
			"sky_csgo_cloudy01",
			"sky_csgo_night02",
			"sky_csgo_night02b",
			"sky_dust",
			"sky_venice",
			"vertigo",
			"vietnam",
			"sky_descent"
		};

		// wall modulation stuff
		FloatColor walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		FloatColor props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		FloatColor skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );

		// clantag
		int clantag_step = 0;

		virtual void ModulateWorld( );
		virtual void ClantagChanger( );
		virtual void ViewModelChanger( );
		virtual void SkyboxChanger( );
		virtual void RainSoundEffects( );
		virtual void ModelChanger( );
	};

	C_Miscellaneous g_Misc;
	Miscellaneous* C_Miscellaneous::Get( ) {
		return &g_Misc;
	}

	void C_Miscellaneous::Main( ) {
		ModulateWorld( );
		SkyboxChanger( );


		// Note for exon from cal: add to menu, maybe fix it to work with night mode idk
		// if ( g_Vars.esp.custom_world_textures > 0 )
		// {
		//	for (int i = 0; i < g_Vars.globals.m_CachedMapMaterials.size(); i++)
		//	{
		//		g_Vars.globals.m_CachedMapMaterials.at(i)->AlphaModulate(1.f);
		//		g_Vars.globals.m_CachedMapMaterials.at(i)->ColorModulate(1.f, 0.f, 0.f);
		//	}
		// }

		if( !g_Vars.globals.HackIsReady )
			return;

		ClantagChanger( );
		ViewModelChanger( );
		//RainSoundEffects( );
		ModelChanger( );

		ConVar* m_bNigger = Interfaces::m_pCvar->FindVar(XorStr("cl_mute_all_but_friends_and_party"));
		ConVar* m_bNigger2 = Interfaces::m_pCvar->FindVar(XorStr("voice_modenable"));

		if (m_bNigger->GetInt() != 0)
			m_bNigger->SetValueInt(0);

		if (m_bNigger2->GetInt() != 1)
			m_bNigger2->SetValueInt(1);
	}

	void C_Miscellaneous::ModulateWorld( ) {
		if( !g_Vars.globals.HackIsReady ) {
			walls = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			props = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			skybox = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			return;
		}

		if( !C_CSPlayer::GetLocalPlayer( ) )
			return;

		static auto w = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		static auto p = FloatColor( 0.9f, 0.9f, 0.9f, 1.0f );
		static auto s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );;

		if( g_Vars.esp.night_mode ) {
			float power = g_Vars.esp.world_adjustement_value / 100.f;
			float power_props = g_Vars.esp.prop_adjustement_value / 100.f;

			w = FloatColor( power, power, power, 1.f );
			p = FloatColor( power_props, power_props, power_props, std::clamp<float>( ( g_Vars.esp.transparent_props + 0.1f ) / 100.f, 0.f, 1.f ) );
		}
		else {
			w = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			p = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
			//s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( g_Vars.esp.skybox ) {
			s = g_Vars.esp.skybox_modulation;
		}
		else {
			s = FloatColor( 1.0f, 1.0f, 1.0f, 1.0f );
		}

		if( walls != w || props != p || skybox != s ) {
			walls = w;
			props = p;
			skybox = s;

			auto invalid_material = Interfaces::m_pMatSystem->InvalidMaterial( );
			for( auto i = Interfaces::m_pMatSystem->FirstMaterial( );
				i != invalid_material;
				i = Interfaces::m_pMatSystem->NextMaterial( i ) ) {
				auto material = Interfaces::m_pMatSystem->GetMaterial( i );

				if( !material || material->IsErrorMaterial( ) )
					continue;

				FloatColor color = walls;
				auto group = material->GetTextureGroupName( );

				if( !material->GetName( ) )
					continue;

				if( *group == 'W' ) { // world textures
					if( group[ 4 ] != 'd' )
						continue;
					color = walls;
				}
				else if( *group == 'S' ) { // staticprops & skybox
					auto thirdCharacter = group[ 3 ];
					if( thirdCharacter == 'B' ) {
						color = skybox;
					}
					else if( thirdCharacter == 't' && group[ 6 ] == 'P' ) {
						color = props;
					}
					else {
						continue;
					}
				}
				else {
					continue;
				}

				color.r = rgb_to_srgb( color.r );
				color.g = rgb_to_srgb( color.g );
				color.b = rgb_to_srgb( color.b );

				material->AlphaModulate( color.a );
				material->ColorModulate( color.r, color.g, color.b );
			}
		}
	}

	void C_Miscellaneous::ClantagChanger() {
		static bool run_once = false;
		static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*)) Engine::Displacement.Function.m_uClanTagChange;
		static std::string tag;
		static int length;

		switch (g_Vars.misc.clantag_changer)
		{
		case 0:
		{

			if (run_once) {
				fnClantagChanged(XorStr(""), XorStr(""));
				run_once = false;
			}
			return;

			break;
		}
		case 1:
		{
			run_once = true;
			tag = XorStr("vader.tech");
			length = tag.length();
			break;
		}
		case 2:
		{
			run_once = true;
			if (g_Vars.misc.custom_clantag != XorStr("")) {
				tag = g_Vars.misc.custom_clantag;
				length = tag.length();
			}
			else {
				tag = XorStr(" ");
				length = tag.length();
			}
			break;
		}

		}

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local)
			return;

		int pos = (int)(local->m_nTickBase() * Interfaces::m_pGlobalVars->interval_per_tick * 2) % (length * 2);

		fnClantagChanged((pos <= length ? tag.substr(0, pos) : tag.substr(pos - length, length - (pos - length))).c_str(), tag.c_str());
	}

	void C_Miscellaneous::ViewModelChanger( ) {
		g_Vars.viewmodel_fov->SetValue( g_Vars.misc.viewmodel_fov );

		static auto backup_viewmodel_x = g_Vars.viewmodel_offset_x->GetFloat();
		static auto backup_viewmodel_y = g_Vars.viewmodel_offset_y->GetFloat();
		static auto backup_viewmodel_z = g_Vars.viewmodel_offset_z->GetFloat();

		if (g_Vars.misc.viewmodel_change) {
			g_Vars.viewmodel_offset_x->SetValue(g_Vars.misc.viewmodel_x);
			g_Vars.viewmodel_offset_y->SetValue(g_Vars.misc.viewmodel_y);
			g_Vars.viewmodel_offset_z->SetValue(g_Vars.misc.viewmodel_z);
		}
		else {
			g_Vars.viewmodel_offset_x->SetValue(backup_viewmodel_x);
			g_Vars.viewmodel_offset_y->SetValue(backup_viewmodel_y);
			g_Vars.viewmodel_offset_z->SetValue(backup_viewmodel_z);
		}
	}

	void C_Miscellaneous::SkyboxChanger( ) {
		static int iOldSky = 0;

		if( !g_Vars.globals.HackIsReady ) {
			iOldSky = 0;
			return;
		}

		static auto fnLoadNamedSkys = ( void( __fastcall* )( const char* ) )Engine::Displacement.Function.m_uLoadNamedSkys;
		static ConVar* default_skyname = Interfaces::m_pCvar->FindVar( XorStr( "sv_skyname" ) );
		if( default_skyname ) {
			if( iOldSky != g_Vars.esp.sky_changer ) {
				if (g_Vars.esp.sky_changer != 15) {
					const char* sky_name = g_Vars.esp.sky_changer != 0 ? skynames[g_Vars.esp.sky_changer] : default_skyname->GetString();
					fnLoadNamedSkys(sky_name);
					iOldSky = g_Vars.esp.sky_changer;
				}
				else {
					fnLoadNamedSkys(g_Vars.esp.custom_skybox.c_str());
					iOldSky = g_Vars.esp.sky_changer;
				}
			}
		}
	}

	void C_Miscellaneous::RainSoundEffects( ) {

		if ( !g_Vars.esp.weather || !g_Vars.esp.weather_thunder || !g_Vars.globals.HackIsReady || !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->IsConnected( ) )
			return;

		static clock_t start_t = clock();

		double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;

		static int choose;

		if (timeSoFar > 6.0)
		{
			switch (choose)
			{
			case 1:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder4.wav")); break;
			case 2:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder5.wav")); break;
			case 3:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder6.wav")); break;
			case 4:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder_distant_01.wav")); break;
			case 5:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder_distant_02.wav")); break;
			case 6:Interfaces::m_pSurface->PlaySound_(XorStr("ambient/playonce/weather/thunder_distant_06.wav")); break;
			}
			start_t = clock();
		}
		else
		{
			int randomnum = 1 + (rand() % 6);

			if (randomnum == choose)
				choose = 1 + (rand() % 6); // could still be the same number but less likely
			else
				choose = randomnum;
		}


	}

	void C_Miscellaneous::ModelChanger() {
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || !local->IsAlive() || !g_Vars.misc.model_changer)
			return;

		switch (g_Vars.misc.models)
		{
		case 0:
		{
			break;
		}
		case 1:
		{
			PrecacheModels(XorStr("models/player/custom_player/kuristaja/vader/vader.mdl"));
			local->SetModelIndex(Interfaces::m_pModelInfo->GetModelIndex(XorStr("models/player/custom_player/kuristaja/vader/vader.mdl")));
			break;
		}
		case 2:
		{
			PrecacheModels(XorStr("models/player/custom_player/kuristaja/stormtrooper/stormtrooper.mdl"));
			local->SetModelIndex(Interfaces::m_pModelInfo->GetModelIndex(XorStr("models/player/custom_player/kuristaja/stormtrooper/stormtrooper.mdl")));
			break;
		}
		case 3:
		{
			if (g_Vars.misc.custom_model.c_str() != XorStr("") && g_Vars.misc.custom_model.size() > 1)
			{
				PrecacheModels(g_Vars.misc.custom_model.c_str());
				local->SetModelIndex(Interfaces::m_pModelInfo->GetModelIndex(g_Vars.misc.custom_model.c_str()));

			}
			break;
		}

		}

	}

	Miscellaneous* Miscellaneous::Get( ) {
		static C_Miscellaneous instance;
		return &instance;
	}
}
