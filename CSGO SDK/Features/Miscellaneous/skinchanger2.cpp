#include "skinchanger2.h"
#include "../../SDK/Classes/PropManager.hpp"
#include <algorithm>
#include <memory.h>
#include "../../SDK/Valve/recv_swap.hpp"
#include <stdlib.h>
#include <unordered_map>
#include "KitParser.hpp"

uintptr_t* rel323(uintptr_t ptr) {
	auto offset = *(uintptr_t*)(ptr + 0x1);
	return (uintptr_t*)(ptr + 5 + offset);
}

void UpdateItem() {
	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || local->IsDead())
		return;

	if (Interfaces::m_pClientState->m_nDeltaTick() == -1)
		return;

	auto ForceUpdate = [](C_BaseCombatWeapon* item) {
		C_EconItemView* view = &item->m_Item();

		if (!view)
			return;

		if (!item->GetClientNetworkable())
			return;

		item->m_bCustomMaterialInitialized() = view->m_nFallbackPaintKit() <= 0;

		item->m_CustomMaterials().RemoveAll();
		view->m_CustomMaterials().RemoveAll();

		size_t count = view->m_VisualsDataProcessors().Count();
		for (size_t i{}; i < count; ++i) {
			auto& elem = view->m_VisualsDataProcessors()[i];
			if (elem) {
				elem->unreference();
				elem = nullptr;
			}
		}

		view->m_VisualsDataProcessors().RemoveAll();

		item->GetClientNetworkable()->PostDataUpdate(0);
		item->GetClientNetworkable()->OnDataChanged(0);

		auto SFWeaponSelection = Interfaces::m_pHud->FindHudElement<std::uintptr_t*>(XorStr("SFWeaponSelection"));

		using ShowAndUpdateSelection_t = void(__thiscall*)(std::uintptr_t*, int, C_BaseCombatWeapon*, bool);
		static auto ShowAndUpdateSelection = (ShowAndUpdateSelection_t)rel323(Memory::Scan(XorStr("client.dll"), XorStr("E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 ? C6 83")));

		ShowAndUpdateSelection(SFWeaponSelection, 0, item, false);
	};

	auto weapons = local->m_hMyWeapons();
	for (size_t i = 0; i < 48; ++i) {
		auto weapon_handle = weapons[i];
		if (!weapon_handle.IsValid())
			break;

		auto weapon = (C_BaseCombatWeapon*)weapon_handle.Get();
		if (!weapon)
			continue;

		ForceUpdate(weapon);
	}
}

#define GET_INDEX Interfaces::m_pModelInfo->GetModelIndex

using namespace std;

#define	LIFE_ALIVE 0

struct knifes {
	int iBayonet;
	int iButterfly;
	int iFlip;
	int iGut;
	int iKarambit;
	int iM9Bayonet;
	int iHuntsman;
	int iFalchion;
	int iDagger;
	int iBowie;
	int iGunGame;
};

int item_def_knifes()
{
	switch (g_Vars.misc.knife_model)
	{
	case 0:
		return 500;
		break;
	case 1:
		return 514;
		break;
	case 2:
		return 515;
		break;
	case 3:
		return 512;
		break;
	case 4:
		return 505;
		break;
	case 5:
		return 506;
		break;
	case 6:
		return 509;
		break;
	case 7:
		return 507;
		break;
	case 8:
		return 508;
		break;
	case 9:
		return 516;
		break;
	default:
		break;
	}
}

struct weapon_info {
	constexpr weapon_info(const char* model, const char* icon = nullptr, int animindex = -1) : model(model), icon(icon), animindex(animindex) {}
	const char* model;
	const char* icon;
	int animindex;
};

const weapon_info* GetWeaponInfo(int defindex) {
	const static std::map<int, weapon_info> Info = {
		{ WEAPON_KNIFE_CT,{ XorStr("models/weapons/v_knife_default_ct.mdl"), XorStr("knife_default_ct"), 2 } },
		{ WEAPON_KNIFE_T,{ XorStr("models/weapons/v_knife_default_t.mdl"), XorStr("knife_t"), 12 } },
		{ WEAPON_KNIFE_BAYONET,{ XorStr("models/weapons/v_knife_bayonet.mdl"), XorStr("bayonet"), 0 } },
		{ WEAPON_KNIFE_FLIP,{ XorStr("models/weapons/v_knife_flip.mdl"), XorStr("knife_flip"), 4 } },
		{ WEAPON_KNIFE_GUT,{ XorStr("models/weapons/v_knife_gut.mdl"), XorStr("knife_gut"), 5 } },
		{ WEAPON_KNIFE_KARAMBIT,{ XorStr("models/weapons/v_knife_karam.mdl"), XorStr("knife_karambit"), 7 } },
		{ WEAPON_KNIFE_M9_BAYONET,{ XorStr("models/weapons/v_knife_m9_bay.mdl"), XorStr("knife_m9_bayonet"), 8 } },
		{ WEAPON_KNIFE_HUNTSMAN,{ XorStr("models/weapons/v_knife_tactical.mdl"), XorStr("knife_tactical") } },
		{ WEAPON_KNIFE_FALCHION,{ XorStr("models/weapons/v_knife_falchion_advanced.mdl"), XorStr("knife_falchion"), 3 } },
		{ WEAPON_KNIFE_BOWIE,{ XorStr("models/weapons/v_knife_survival_bowie.mdl"), XorStr("knife_survival_bowie"), 11 } },
		{ WEAPON_KNIFE_BUTTERFLY,{ XorStr("models/weapons/v_knife_butterfly.mdl"), XorStr("knife_butterfly"), 1 } },
		{ WEAPON_KNIFE_SHADOW_DAGGERS,{ XorStr("models/weapons/v_knife_push.mdl"), XorStr("knife_push"), 9 } },
		{ GLOVE_STUDDED_BLOODHOUND,{ XorStr("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl") } },
		{ GLOVE_T_SIDE,{ XorStr("models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl") } },
		{ GLOVE_CT_SIDE,{ XorStr("models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl") } },
		{ GLOVE_SPORTY,{ XorStr("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl") } },
		{ GLOVE_SLICK,{ XorStr("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl") } },
		{ GLOVE_LEATHER_WRAP,{ XorStr("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl") } },
		{ GLOVE_MOTORCYCLE,{ XorStr("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl") } },
		{ GLOVE_SPECIALIST,{ XorStr("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl") } }
	};
	const auto entry = Info.find(defindex);
	return entry == end(Info) ? nullptr : &entry->second;
}

bool apply_glove(C_BaseAttributableItem* glove, const char* model, int item_definition_index, int paint_kit, int model_index, int entity_quality, float fallback_wear) {
	//auto local = C_CSPlayer::GetLocalPlayer();

	//if (!local || local->IsDead())
	//	return false;

	player_info_t info;
	Interfaces::m_pEngine->GetPlayerInfo(Interfaces::m_pEngine->GetLocalPlayer(), &info);
	glove->m_Item().m_iItemIDHigh() = -1;
	glove->m_Item().m_iAccountID() = info.xuid_low;
	glove->m_Item().m_flFallbackWear() = fallback_wear;
	glove->m_Item().m_iEntityQuality() = entity_quality;
	glove->m_Item().m_nFallbackPaintKit() = paint_kit;
	glove->m_Item().m_nFallbackStatTrak() = -1;
	glove->m_Item().m_iItemDefinitionIndex() = item_definition_index;
	//glove->m_Item().m_bInitialized() = true;
	glove->SetModelIndex(model_index /*+ 2*/);

	//static auto fnEquip
	//	= reinterpret_cast<int(__thiscall*)(void*, void*)>(
	//		Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 EC 10 53 8B 5D 08 57 8B F9"))
	//		);

	//static auto fnInitializeAttributes
	//	= reinterpret_cast<int(__thiscall*)(void*)>(
	//		Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F8 83 EC 0C 53 56 8B F1 8B 86"))
	//		);

	//fnEquip(glove, local); //follow entity, owner etc.
	//*(int*)((uintptr_t)local + 0xA20) = 1; // remove default arms in 3th person mode dword_15268230 = (int)"m_nBody";
	//fnInitializeAttributes(glove); //set paintkit etc. from inventory changer by itemid

	auto networkable = glove->GetClientNetworkable();
	if (networkable) {
		networkable->PreDataUpdate(0);
	}

	return true;
}

bool apply_skin(C_CSPlayer* local, C_BaseAttributableItem* skin, const char* model, int item_definition_index, int paint_kit, int model_index, int entity_quality, float FallbackWear) {
	skin->m_Item().m_iItemDefinitionIndex() = item_definition_index;
	skin->m_Item().m_nFallbackPaintKit() = paint_kit;
	skin->m_Item().m_iEntityQuality() = entity_quality;
	skin->m_Item().m_flFallbackWear() = FallbackWear;
	skin->m_nModelIndex() = model_index;

	auto local_player = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(Interfaces::m_pEngine->GetLocalPlayer()));
	if (!local_player) {
		return false;
	}

	auto viewmodel = (C_BaseViewModel*)local->m_hViewModel().Get();

	if (!viewmodel)
		return false;

	static int offset = Engine::PropManager::Instance()->GetOffset(XorStr("DT_BaseViewModel"), XorStr("m_hWeapon"));
	auto m_hWeapon = *(CBaseHandle*)(uintptr_t(viewmodel) + offset);

	auto view_model_weapon = (C_BaseViewModel*)(Interfaces::m_pEntList->GetClientEntityFromHandle(m_hWeapon));

	if (view_model_weapon != skin)
		return false;

	auto GetWorldModel = (C_BaseAttributableItem*)Interfaces::m_pEntList->GetClientEntityFromHandle(skin->m_hWeaponWorldModel());

	if (!GetWorldModel)
		return false;

	viewmodel->m_nModelIndex() = Interfaces::m_pModelInfo->GetModelIndex(model);

	GetWorldModel->m_nModelIndex() = Interfaces::m_pModelInfo->GetModelIndex(model) + 1;

	return true;
}

void skins_speedy::Skinchanger()
{
	auto model_bayonet = XorStr("models/weapons/v_knife_bayonet.mdl");
	auto model_m9 = XorStr("models/weapons/v_knife_m9_bay.mdl");
	auto model_karambit = XorStr("models/weapons/v_knife_karam.mdl");
	auto model_bowie = XorStr("models/weapons/v_knife_survival_bowie.mdl");
	auto model_butterfly = XorStr("models/weapons/v_knife_butterfly.mdl");
	auto model_falchion = XorStr("models/weapons/v_knife_falchion_advanced.mdl");
	auto model_flip = XorStr("models/weapons/v_knife_flip.mdl");
	auto model_gut = XorStr("models/weapons/v_knife_gut.mdl");
	auto model_huntsman = XorStr("models/weapons/v_knife_tactical.mdl");
	auto model_daggers = XorStr("models/weapons/v_knife_push.mdl");

	auto model_blood = XorStr("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl");
	auto model_sport = XorStr("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl");
	auto model_slick = XorStr("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl");
	auto model_leath = XorStr("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl");
	auto model_moto = XorStr("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl");
	auto model_speci = XorStr("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl");

	if (!Interfaces::m_pEngine->IsConnected() && !Interfaces::m_pEngine->IsInGame()) {
		return;
	}

	auto local = C_CSPlayer::GetLocalPlayer();
	if (!local)
		return;

	auto local_player = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntity(Interfaces::m_pEngine->GetLocalPlayer()));
	if (!local_player) {
		return;
	}

	if (g_Vars.misc.enable_gloves && g_Vars.misc.enable_skins) {
		auto const wearables = local_player->m_hMyWearables();
		if (!wearables) {
			return;
		}
		static CHandle< C_BaseCombatWeapon > glove_handle = std::uint32_t(0);
		auto glove = (C_BaseAttributableItem*)wearables[0].Get();

		if (!glove) {
			const auto our_glove = (C_BaseAttributableItem*)glove_handle.Get();

			if (our_glove) {
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}
		if (!local_player->IsAlive()) {
			// We are dead but we have a glove, destroy it
			if (glove) {
				auto networkable = glove->GetClientNetworkable();
				if (networkable) {
					networkable->SetDestroyedOnRecreateEntities();
					networkable->Release();
				}
			}

			return;
		}
		if (!glove) {
			auto get_wearable_create_fn = []() -> CreateClientClassFn {
				auto clazz = Interfaces::m_pClient->GetAllClasses();
					while( clazz->m_ClassID != CEconWearable )
						clazz = clazz->m_pNext;

				return (CreateClientClassFn)clazz->m_pCreateFn;
			};

			static auto create_wearable_fn = get_wearable_create_fn();

			auto entry = Interfaces::m_pEntList->GetHighestEntityIndex() + 1;

			//for (int i = 0; i < Interfaces::m_pEntList->GetHighestEntityIndex(); i++)
			//{
			//	auto pEntity = Interfaces::m_pEntList->GetClientEntity(i);

			//	if (pEntity && pEntity->GetClientClass()->m_ClassID == CRopeKeyframe)
			//	{
			//		entry = i;
			//		break;
			//	}
			//}

			const auto serial = rand() % 0x1000;
			create_wearable_fn(entry, serial);

			glove = static_cast<C_BaseAttributableItem*>(Interfaces::m_pEntList->GetClientEntity(entry));

			if (!glove)
				return;

			Vector new_pos = Vector{ 10000.0f, 10000.0f, 10000.f };
			glove->SetAbsOrigin(new_pos);

			wearables[0] = CBaseHandle(entry | (serial << 16));
			glove_handle = wearables[0];
		}
		if (glove) {
			*(int*)((uintptr_t)glove + 0x64) = -1;

			auto g_wear = 0.01f;

			int paintkit = 1;
			if (g_Vars.misc.gloves_model == 1) {
				switch (g_Vars.misc.gloves_skin) {
				case 0:
					paintkit = 10006;
					break;
				case 1:
					paintkit = 10007;
					break;
				case 2:
					paintkit = 10008;
					break;
				case 3:
					paintkit = 10039;
					break;
				default:
					paintkit = 0; break;
				}
			}
			else if (g_Vars.misc.gloves_model == 2) {
				switch (g_Vars.misc.gloves_skin)
				{
				case 0:
					paintkit = 10038; break;
				case 1:
					paintkit = 10037; break;
				case 2:
					paintkit = 10018; break;
				case 3:
					paintkit = 10019; break;
				case 4:
					paintkit = 10048; break;
				case 5:
					paintkit = 10047; break;
				case 6:
					paintkit = 10045; break;
				case 7:
					paintkit = 10046; break;
				default:
					paintkit = 0; break;
				}
			}
			else if (g_Vars.misc.gloves_model == 3) {
				switch (g_Vars.misc.gloves_skin)
				{
				case 0:
					paintkit = 10013; break;
				case 1:
					paintkit = 10015; break;
				case 2:
					paintkit = 10016; break;
				case 3:
					paintkit = 10040; break;
				case 4:
					paintkit = 10043; break;
				case 5:
					paintkit = 10044; break;
				case 6:
					paintkit = 10041; break;
				case 7:
					paintkit = 10042; break;
				default:
					paintkit = 0; break;
				}
			}
			else if (g_Vars.misc.gloves_model == 4) {
				switch (g_Vars.misc.gloves_skin)
				{
				case 0:
					paintkit = 10009; break;
				case 1:
					paintkit = 10010; break;
				case 2:
					paintkit = 10021; break;
				case 3:
					paintkit = 10036; break;
				case 4:
					paintkit = 10053; break;
				case 5:
					paintkit = 10054; break;
				case 6:
					paintkit = 10055; break;
				case 7:
					paintkit = 10056; break;
				default:
					paintkit = 0; break;
				}
			}
			else if (g_Vars.misc.gloves_model == 5) {
				switch (g_Vars.misc.gloves_skin)
				{
				case 0:
					paintkit = 10024; break;
				case 1:
					paintkit = 10026; break;
				case 2:
					paintkit = 10027; break;
				case 3:
					paintkit = 10028; break;
				case 4:
					paintkit = 10050; break;
				case 5:
					paintkit = 10051; break;
				case 6:
					paintkit = 10052; break;
				case 7:
					paintkit = 10049; break;
				default:
					paintkit = 0; break;
				}
			}
			else if (g_Vars.misc.gloves_model == 6) {
				switch (g_Vars.misc.gloves_skin)
				{
				case 0:
					paintkit = 10030; break;
				case 1:
					paintkit = 10033; break;
				case 2:
					paintkit = 10034; break;
				case 3:
					paintkit = 10035; break;
				case 4:
					paintkit = 10061; break;
				case 5:
					paintkit = 10062; break;
				case 6:
					paintkit = 10063; break;
				case 7:
					paintkit = 10064; break;
				default:
					paintkit = 0; break;
				}
			}
			switch (g_Vars.misc.gloves_model) {
			case 1:
				apply_glove(glove, model_blood, GLOVE_STUDDED_BLOODHOUND, paintkit, GET_INDEX(model_blood), 3, g_wear);
				break;
			case 2:
				apply_glove(glove, model_sport, GLOVE_SPORTY, paintkit, GET_INDEX(model_sport), 3, g_wear);
				break;
			case 3:
				apply_glove(glove, model_slick, GLOVE_SLICK, paintkit, GET_INDEX(model_slick), 3, g_wear);
				break;
			case 4:
				apply_glove(glove, model_leath, GLOVE_LEATHER_WRAP, paintkit, GET_INDEX(model_leath), 3, g_wear);
				break;
			case 5:
				apply_glove(glove, model_moto, GLOVE_MOTORCYCLE, paintkit, GET_INDEX(model_moto), 3, g_wear);
				break;
			case 6:
				apply_glove(glove, model_speci, GLOVE_SPECIALIST, paintkit, GET_INDEX(model_speci), 3, g_wear);
				break;
			}
			//glovenet->PreDataUpdate(0);
		}
	}

	if (g_Vars.misc.enable_skins) {
		auto active_weapon = local_player->m_hActiveWeapon().Get();

		if (!active_weapon) {
			return;
		}
		auto my_weapons = local_player->m_hMyWeapons();

		for (int i = 0; i < 48; ++i) {
			auto weapon = (C_BaseAttributableItem*)my_weapons[i].Get();

			if (!weapon) {
				continue;
			}

			auto wear = 0.001f;

			if (weapon->IsKnife()) {
				switch (g_Vars.misc.knife_model) {
				case 0:
					apply_skin(local, weapon, model_bayonet, WEAPON_KNIFE_BAYONET, g_Vars.misc.knife_skin, GET_INDEX(model_bayonet), 3, wear);
					break;
				case 1:
					apply_skin(local, weapon, model_bowie, WEAPON_KNIFE_BOWIE, g_Vars.misc.knife_skin, GET_INDEX(model_bowie), 3, wear);
					break;
				case 2:
					apply_skin(local, weapon, model_butterfly, WEAPON_KNIFE_BUTTERFLY, g_Vars.misc.knife_skin, GET_INDEX(model_butterfly), 3, wear);
					break;
				case 3:
					apply_skin(local, weapon, model_falchion, WEAPON_KNIFE_FALCHION, g_Vars.misc.knife_skin, GET_INDEX(model_falchion), 3, wear);
					break;
				case 4:
					apply_skin(local, weapon, model_flip, WEAPON_KNIFE_FLIP, g_Vars.misc.knife_skin, GET_INDEX(model_flip), 3, wear);
					break;
				case 5:
					apply_skin(local, weapon, model_gut, WEAPON_KNIFE_GUT, g_Vars.misc.knife_skin, GET_INDEX(model_gut), 3, wear);
					break;
				case 6:
					apply_skin(local, weapon, model_huntsman, WEAPON_KNIFE_HUNTSMAN, g_Vars.misc.knife_skin, GET_INDEX(model_huntsman), 3, wear);
					break;
				case 7:
					apply_skin(local, weapon, model_karambit, WEAPON_KNIFE_KARAMBIT, g_Vars.misc.knife_skin, GET_INDEX(model_karambit), 3, wear);
					break;
				case 8:
					apply_skin(local, weapon, model_m9, WEAPON_KNIFE_M9_BAYONET, g_Vars.misc.knife_skin, GET_INDEX(model_m9), 3, wear);
					break;
				case 9:
					apply_skin(local, weapon, model_daggers, WEAPON_KNIFE_SHADOW_DAGGERS, g_Vars.misc.knife_skin, GET_INDEX(model_daggers), 3, wear);
					break;
				}
			}

			switch (weapon->m_Item().m_iItemDefinitionIndex()) {
			case WEAPON_USPS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.usp_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P2000:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p2k_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_GLOCK:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.glock_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P250:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p250_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_FIVESEVEN:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.fiveseven_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_TEC9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.tec9_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_CZ75A:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.cz75a_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_ELITE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.elite_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_DEAGLE:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.deagle_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_REVOLVER:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.revolver_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_FAMAS:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.famas_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_GALIL:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.galilar_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M4A4:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m4a1_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M4A1S:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m4a1s_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AK47:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ak47_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SG553:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.sg556_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AUG:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.aug_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SSG08:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ssg08_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_AWP:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.awp_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SCAR20:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.scar20_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_G3SG1:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.g3sg1_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_SAWEDOFF:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.sawedoff_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_M249:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.m249_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_NEGEV:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.negev_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MAG7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mag7_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_XM1014:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.xm1014_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_NOVA:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.nova_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_BIZON:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.bizon_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MP7:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mp7_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MP9:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mp9_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_MAC10:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.mac10_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_P90:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.p90_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			case WEAPON_UMP45:
				weapon->m_Item().m_nFallbackPaintKit() = g_Vars.misc.ump45_skin, weapon->m_Item().m_flFallbackWear() = wear;
				break;
			}
			weapon->m_OriginalOwnerXuidLow() = 0;
			weapon->m_OriginalOwnerXuidHigh() = 0;
			weapon->m_nFallbackSeed() = 661;
			weapon->m_Item().m_iItemIDHigh() = -1;
		}

		auto& global = g_Vars.m_global_skin_changer;

		if (global.m_update_skins) {
			UpdateItem();
			global.m_update_skins = false;
		}

	}
}