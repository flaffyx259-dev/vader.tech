#include "../Hooked.hpp"
#include "../../SDK/sdk.hpp"
#include "../../Features/Visuals/EventLogger.hpp"


std::vector<std::string> PrintedMats;
namespace Hooked
{
	void __stdcall PreEntity(const char* szMapName)
	{
		oPreEntity(Interfaces::m_pClient.pointer, szMapName);
		g_Vars.globals.m_CachedMapMaterials.clear(); PrintedMats.clear();
		
#ifdef DEV
		ILoggerEvent::Get()->PushEvent("Map Material List:\n", FloatColor(0.f, 0.f, 1.f), true); 
#endif
		
		KeyValues* pKeyValues = new KeyValues("LightmappedGeneric"); if (g_Vars.esp.custom_world_textures <= 0 || !pKeyValues) return;
		pKeyValues->SetString(XorStr("$basetexture"), g_Vars.esp.custom_world_textures == 3 ? g_Vars.esp.custom_world_texture_string.c_str() : (g_Vars.esp.custom_world_textures == 2 ? XorStr("dev/dev_measuregeneric01") : XorStr("dev/dev_measuregeneric01b")));

		for (MaterialHandle_t i = Interfaces::m_pMatSystem->FirstMaterial(); i != Interfaces::m_pMatSystem->InvalidMaterial(); i = Interfaces::m_pMatSystem->NextMaterial(i)) {
			IMaterial* pMaterial = Interfaces::m_pMatSystem->GetMaterial(i);
			if (pMaterial->IsErrorMaterial() || pMaterial->IsTranslucent() || pMaterial->IsSpriteCard() || std::string(pMaterial->GetTextureGroupName()).find(XorStr("World")) == std::string::npos) continue;

			std::string sName = std::string(pMaterial->GetName());
				
#ifdef DEV
			if (std::find(std::begin(PrintedMats), std::end(PrintedMats), sName) == std::end(PrintedMats))
			{
				ILoggerEvent::Get()->PushEvent((sName + "\n").c_str(), FloatColor(1.f, 1.f, 1.f), false);
				PrintedMats.emplace_back(sName);
			}
#endif
			
			if (FindString(sName, "water") || FindString(sName, "glass")
				|| FindString(sName, "door") || FindString(sName, "tools")
				|| FindString(sName, "player") || FindString(sName, "wall28")
				|| FindString(sName, "wall26") || FindString(sName, "decal")
				|| FindString(sName, "overlay") || FindString(sName, "hay")
				|| FindString(sName, "ladder") || FindString(sName, "fence")
				|| FindString(sName, "gate") || FindString(sName, "rail"))
				continue;

			pMaterial->SetShaderAndParams(pKeyValues);
			g_Vars.globals.m_CachedMapMaterials.emplace_back(pMaterial);
		}
	}
}