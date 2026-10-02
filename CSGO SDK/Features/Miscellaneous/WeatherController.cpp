#include "WeatherController.hpp"
#include "../../source.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../Renderer/Render.hpp"


namespace Engine
{
	class C_WeatherController : public WeatherController {
	public:
		C_WeatherController( ) { }
		virtual ~C_WeatherController( ) { }

		virtual void ResetWeather( );
		virtual void UpdateWeather( ); // call on overrideview
		virtual void ResetData( );
	};

	WeatherController* WeatherController::Get( ) {
		static C_WeatherController instance;
		return &instance;
	}

	void C_WeatherController::ResetWeather( ) {
		if (!g_Vars.globals.bCreatedRain) {
			return;
		}

		for( int i = 0; i <= Interfaces::m_pEntList->GetHighestEntityIndex( ); i++ ) {
			C_BaseEntity* pEntity = ( C_BaseEntity* )Interfaces::m_pEntList->GetClientEntity( i );
			if( !pEntity )
				continue;

			const ClientClass* pClientClass = pEntity->GetClientClass( );
			if( !pClientClass )
				continue;

			if( pClientClass->m_ClassID == ClassId_t::CPrecipitation ) {
				if( pEntity->GetClientNetworkable( ) )
					pEntity->GetClientNetworkable( )->Release( );
			}
		}
	}

	void C_WeatherController::UpdateWeather() {
		if (!g_Vars.esp.weather) {
			return;
		}

		if (g_Vars.globals.bCreatedRain) {
			return;
		}

		ClientClass* Class = Interfaces::m_pClient->GetAllClasses();
		IClientNetworkable* m_Networkable = nullptr;

		if (!g_Vars.globals.bCreatedRain) {
			while ((int32_t)(Class->m_ClassID) != ClassId_t::CPrecipitation)
				Class = Class->m_pNext;

			m_Networkable = ((IClientNetworkable * (*)(int, int))Class->m_pCreateFn)(Interfaces::m_pEntList->GetHighestEntityIndex() + 1, RandomInt(0, 4096));
			if (!m_Networkable || !((IClientRenderable*)m_Networkable)->GetIClientUnknown())
				return;
		}

		if (m_Networkable) {

			IClientUnknown* pRainUnknown = ((IClientRenderable*)m_Networkable)->GetIClientUnknown();
			if (!pRainUnknown) {
				return;
			}

			C_BaseEntity* m_Precipitation = pRainUnknown->GetBaseEntity();
			if (!m_Precipitation) {
				return;
			}

			m_Networkable->PreDataUpdate(NULL);
			m_Networkable->OnPreDataChanged(NULL);

			// null da callbacks
			if (g_Vars.r_RainRadius->fnChangeCallback.m_Size != 0)
				g_Vars.r_RainRadius->fnChangeCallback.m_Size = 0;

			// limit the render distance of da rain
			if (g_Vars.r_RainRadius->GetFloat() != 1000.f)
				g_Vars.r_RainRadius->SetValueFloat(1000.f);

			// only PRECIPITATION_TYPE_RAIN and PRECIPITATION_TYPE_SNOW work..?
			m_Precipitation->m_nPrecipType() = PrecipitationType_t::PRECIPITATION_TYPE_SNOW;
			m_Precipitation->OBBMins() = Vector(-16384.0f, -16384.0f, -16384.0f);
			m_Precipitation->OBBMaxs() = Vector(16384.0f, 16384.0f, 16384.0f);

			m_Networkable->OnDataChanged(NULL);
			m_Networkable->PostDataUpdate(NULL);

			g_Vars.globals.bCreatedRain = true;
		}
	}

	void C_WeatherController::ResetData()
	{
		Engine::WeatherController::Get()->ResetWeather();
		g_Vars.globals.bCreatedRain = false;
	}

}