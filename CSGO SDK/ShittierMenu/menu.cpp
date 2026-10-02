//#include "Menu.hpp"
//#include "Fonts.h"
//#include <chrono>
//#include "../Utils/Config.hpp"
//#define IMGUI_DEFINE_MATH_OPERATORS
//#include "IMGAY/imgui_internal.h"
//#include "IMGAY/impl/imgui_impl_dx9.h"
//#include "IMGAY/impl/imgui_impl_win32.h"
//#include "IMGAY/imgui_stdlib.h"
//#include "../Utils/logo.h"
//#include "../Utils/Config.hpp"
//#include "../source.hpp"
//#include "../Features/Miscellaneous/walkbot.h"
//#include "../Features/Visuals/EventLogger.hpp"
//#include "../SDK/Classes/Player.hpp"
//#include "../Utils/lazy_importer.hpp"
//#include "../SDK/CVariables.hpp"
//
////lua
////void dmt(std::string key) {
////	if (ImGui::IsItemHovered()) {
////		ImGui::BeginTooltip();
////		ImGui::Text(key.c_str());
////		ImGui::EndTooltip();
////	}
////}
////
////void draw_lua_items(std::string tab, std::string container) {
////	for (auto i : g_lua.menu_items[tab][container]) {
////		if (!i.is_visible)
////			continue;
////
////		auto type = i.type;
////		switch (type)
////		{
////		case MENUITEM_CHECKBOX:
////			if (ImGui::Checkbox(i.label.c_str(), &LuaConfigSystem::C_BOOL[i.key])) {
////				if (i.callback != sol::nil)
////					i.callback(LuaConfigSystem::C_BOOL[i.key]);
////			}
////
////			dmt(i.key);
////			break;
////
////			break;
////		case MENUITEM_SLIDERINT:
////			ImGui::NewLine(); ImGui::PushItemWidth(159.f);
////			if (ImGui::SliderInt(i.label.c_str(), &LuaConfigSystem::C_INT[i.key], i.i_min, i.i_max, i.format.c_str())) {
////				if (i.callback != sol::nil)
////					i.callback(LuaConfigSystem::C_INT[i.key]);
////			}
////			ImGui::PopItemWidth();
////
////			dmt(i.key);
////			break;
////		case MENUITEM_SLIDERFLOAT:
////			ImGui::NewLine(); ImGui::PushItemWidth(159.f);
////			if (ImGui::SliderFloat(i.label.c_str(), &LuaConfigSystem::C_FLOAT[i.key], i.f_min, i.f_max, i.format.c_str())) {
////				if (i.callback != sol::nil)
////					i.callback(LuaConfigSystem::C_FLOAT[i.key]);
////			}
////			ImGui::PopItemWidth();
////
////			dmt(i.key);
////			break;
////		case MENUITEM_KEYBIND:
////			if (ImGui::Keybind(i.label.c_str(), &LuaConfigSystem::C_INT[i.key], i.allow_style_change ? &LuaConfigSystem::C_INT[i.key + XorStr("style")] : NULL)) {
////				if (i.callback != sol::nil)
////					i.callback(LuaConfigSystem::C_INT[i.key], LuaConfigSystem::C_INT[i.key + XorStr("style")]);
////			}
////
////			dmt(i.key + (i.allow_style_change ? XorStr(" | ") + i.key + XorStr("style") : XorStr("")));
////			break;
////		case MENUITEM_TEXT:
////			ImGui::Text(i.label.c_str());
////			break;
////		case MENUITEM_COLORPICKER:
////
////			if (ImGui::CalcTextSize(i.label.c_str()).x > 0.00f)
////			{
////				ImGui::Text(i.label.c_str());
////			}
////			ImGui::SameLine(0.0f, -1.0f, 0.0f);
////			if (ImGui::ColorEdit4(std::string{ XorStr("##") }.append(i.label.c_str()).append(XorStr("Lua")).c_str(), LuaConfigSystem::C_COLOR[i.key], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip)) {
////				if (i.callback != sol::nil)
////					i.callback(LuaConfigSystem::C_COLOR[i.key][0] * 255, LuaConfigSystem::C_COLOR[i.key][1] * 255, LuaConfigSystem::C_COLOR[i.key][2] * 255, LuaConfigSystem::C_COLOR[i.key][3] * 255);
////			}
////
////			dmt(i.key);
////			break;
////		case MENUITEM_BUTTON:
////			if (ImGui::Button(i.label.c_str())) {
////				if (i.callback != sol::nil)
////					i.callback();
////			}
////			break;
////		default:
////			break;
////		}
////	}
////}
////
//
//IDirect3DTexture9* logo_nuts;
// 
//int tab = 0, aimbotTab = 1, rageTab = -1, legitTab = 0, AAtab = 0, visualsSubTab = 0, miscSubtabs = 0, skinsSubtabs = 0, scriptsSubtabs = 0, PlayerListSubTabs = 0;
//
//void ColorPicker(const char* name, float* color, bool alpha, bool combo) {
//
//	ImGuiWindow* window = ImGui::GetCurrentWindow();
//	ImGuiStyle* style = &ImGui::GetStyle();
//
//	auto alphaSliderFlag = alpha ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;
//	//if (ImGui::CalcTextSize(name).x > 0.00f)
//	//{
//	//	ImGui::Text(name);
//	//}
//	ImGui::SameLine(combo ? 170.f : 0.0f, -1.0f, 0.0f);
//	ImGui::ColorEdit4(std::string{ XorStr("##") }.append(name).append(XorStr("Picker")).c_str(), color, alphaSliderFlag | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
//}
//
//void ColorPicker_w_name(const char* name, float* color, bool alpha, bool combo) {
//
//	ImGuiWindow* window = ImGui::GetCurrentWindow();
//	ImGuiStyle* style = &ImGui::GetStyle();
//
//	auto alphaSliderFlag = alpha ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;
//	if (ImGui::CalcTextSize(name).x > 0.00f)
//	{
//		ImGui::Text(name);
//	}
//	ImGui::SameLine(combo ? 170.f : 0.0f, -1.0f, 0.0f);
//	ImGui::ColorEdit4(std::string{ XorStr("##") }.append(name).append(XorStr("Picker")).c_str(), color, alphaSliderFlag | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
//}
//
//enum WeaponGroup_t {
//	WEAPONGROUP_FAGGOT,
//	WEAPONGROUP_PISTOL,
//	WEAPONGROUP_HEAVYPISTOL,
//	WEAPONGROUP_RIFLE,
//	WEAPONGROUP_SNIPER,
//	WEAPONGROUP_AUTOSNIPER,
//	WEAPONGROUP_SUBMACHINE,
//	WEAPONGROUP_HEAVY,
//	WEAPONGROUP_SHOTGUN,
//	WEAPONGROUP_MAX
//};
//
//void biggestMeme2()
//{
//	ImGui::Text("    ");
//	ImGui::SameLine();
//}
//
//bool testbox = false, testbox1 = false;
//void Ragebot()
//{
//	//autosniper, sniper, pistol, heavy pistol, other
//
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//	ImGui::NewLine();
//	{
//		static int current_weapon = 0, rage_current_group = 0, current_group = 0;
//
//		CVariables::RAGE* rbot = nullptr;
//		switch (rageTab) {
//		case WEAPONGROUP_PISTOL:
//			rbot = &g_Vars.rage_pistols;
//			break;
//		case WEAPONGROUP_HEAVYPISTOL:
//			rbot = &g_Vars.rage_heavypistols;
//			break;
//		case WEAPONGROUP_SUBMACHINE + 1:
//			rbot = &g_Vars.rage_smgs;
//			break;
//		case WEAPONGROUP_RIFLE:
//			rbot = &g_Vars.rage_rifles;
//			break;
//		case WEAPONGROUP_SHOTGUN + 1:
//			rbot = &g_Vars.rage_shotguns;
//			break;
//		case WEAPONGROUP_SNIPER:
//			rbot = &g_Vars.rage_awp;
//			break;
//		case WEAPONGROUP_SNIPER + 1:
//			rbot = &g_Vars.rage_scout;
//			break;
//		case WEAPONGROUP_HEAVY + 1:
//			rbot = &g_Vars.rage_heavys;
//			break;
//		case WEAPONGROUP_AUTOSNIPER + 1:
//			rbot = &g_Vars.rage_autosnipers;
//			break;
//		default:
//			rbot = &g_Vars.rage_default;
//			break;
//		}
//
//		if (rageTab == -1) {
//			InsertCheckbox(EnableRagebot, XorStr("Aimbot"), &g_Vars.rage.enabled);
//
//			InsertCheckbox(SilentAim, XorStr("Silent aim"), &g_Vars.rage.silent_aim);
//			InsertCheckbox(Autoshoot, XorStr("Automatic fire"), &g_Vars.rage.auto_fire);
//			InsertCheckbox(NoSpreadMode, XorStr("No-spread Mode"), &g_Vars.rage.nospread_mode);
//		}
//
//		if (rageTab != -1) {
//
//			InsertCheckbox(OverrideWeaponGroup, XorStr("Enable") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->active);
//			const char* TargetSelection[] = { XorStr("Highest damage"), XorStr("Nearest to crosshair"), XorStr("Lowest health"), XorStr("Lowest distance"), XorStr("Lowest latency") };
//			InsertCombo(std::string(XorStr("Target selection") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->target_selection, TargetSelection);
//
//			std::vector<MultiItem_t> hitboxes = {
//				{ XorStr("Head"), &rbot->hitboxes_head },
//				{ XorStr("Chest"), &rbot->hitboxes_chest },
//				{ XorStr("Stomach"), &rbot->hitboxes_stomach },
//				{ XorStr("Pelvis"), &rbot->hitboxes_pelvis },
//				{ XorStr("Arms"), &rbot->hitboxes_arms },
//				{ XorStr("Legs"), &rbot->hitboxes_legs },
//				{ XorStr("Feet"), &rbot->hitboxes_feets },
//			};
//
//			std::vector<MultiItem_t> multipoints = {
//				{ XorStr("Head"), &rbot->mp_hitboxes_head },
//				{ XorStr("Chest"), &rbot->mp_hitboxes_chest },
//				{ XorStr("Stomach"), &rbot->mp_hitboxes_stomach },
//				{ XorStr("Legs"), &rbot->mp_hitboxes_legs },
//				{ XorStr("Feet"), &rbot->mp_hitboxes_feets },
//			};
//
//			InsertMultiCombo(std::string(XorStr("Hitboxes") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), hitboxes);
//
//			InsertMultiCombo(std::string(XorStr("Multipoints") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), multipoints);
//
//			InsertCheckbox(IgnoreLimbs, XorStr("Ignore limbs when moving") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->ignorelimbs_ifwalking);
//
//			std::vector<MultiItem_t> mp_safety_hitboxes = {
//				{ XorStr("Head"), &rbot->mp_hitboxes_head },
//				{ XorStr("Chest"), &rbot->mp_hitboxes_chest },
//				{ XorStr("Stomach"), &rbot->mp_hitboxes_stomach },
//				{ XorStr("Legs"), &rbot->mp_hitboxes_legs },
//				{ XorStr("Feet"), &rbot->mp_hitboxes_feets },
//			};
//
//			//	if (!mp_safety_hitboxes.empty())
//			//		GUI::Controls::MultiDropdown(XorStr("Multipoints") + std::string(XorStr("#") + std::to_string(rage_current_group)), mp_safety_hitboxes);
//
//			InsertCheckbox(StaticPointscale, XorStr("Static pointscale") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->static_point_scale);
//			if (rbot->static_point_scale) {
//				InsertSliderFloat(std::string(XorStr("Head scale##687687675") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
//				InsertSliderFloat(std::string(XorStr("Body scale##68776678") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->body_point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
//			}
//			InsertSliderFloat(std::string(XorStr("Minimum hitchance") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->hitchance, 0.f, 100.f, XorStr("%.0f%%"));
//			InsertSliderInt(std::string(XorStr("Minimum dmg") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->min_damage_visible, 1, 130, std::string(rbot->min_damage_visible > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_visible - 100)))) : XorStr("%d hp")).c_str());
//			InsertCheckbox(AutomaticPenetration, XorStr("Automatic penetration") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->autowall);
//			if (rbot->autowall) {
//				InsertSliderInt(std::string(XorStr("Minimum penetration") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->min_damage, 1.f, 130.f, std::string(rbot->min_damage > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage - 100)))) : XorStr("%d hp")).c_str());
//			}
//		}
//
//		ImGui::NextColumn();
//		ImGui::NewLine();
//
//		if (rageTab == -1) {
//			InsertCheckbox(ExtendedBT, std::string(XorStr("Ping-spike") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &g_Vars.misc.extended_backtrack);
//			if (g_Vars.misc.extended_backtrack) {
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(std::string(XorStr("##Ping-spike Key") + std::to_string(rageTab)).c_str(), &g_Vars.misc.extended_backtrack_key.key, &g_Vars.misc.extended_backtrack_key.cond, ImVec2{ 40,20 });
//				InsertSliderFloat(std::string(XorStr("Ping-spike amount") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &g_Vars.misc.extended_backtrack_time, 0.f, 1.f, XorStr("%.2fs"));
//			}
//		}
//
//		//InsertCheckbox(CompensateSpreadRage, std::string(XorStr("Compensate spread") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->compensate_spread);
//
//		if (rageTab != -1) {
//
//
//			InsertCheckbox(Autostop, XorStr("Automatic stop"), &rbot->autostop_check);
//			std::vector<MultiItem_t> stop_options = {
//				{ XorStr("Force Accuracy"), &rbot->always_stop },
//				{ XorStr("Between shots"), &rbot->between_shots },
//			};
//
//			if (rbot->autostop_check) {
//				InsertMultiCombo(XorStr("Automatic stop options"), stop_options);
//			}
//
//			//InsertCheckbox(Autoscope, XorStr("Automatic scope") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->autoscope);
//
//			const char* autoscopeoptions[] = { XorStr("Off"), XorStr("Always Scope"), XorStr("Hitchance Fail") };
//
//			if (rageTab == WEAPONGROUP_RIFLE || rageTab == WEAPONGROUP_SNIPER + 1 || rageTab == WEAPONGROUP_AUTOSNIPER + 1 || rageTab == WEAPONGROUP_SNIPER || rageTab == 0) {
//				InsertCombo(XorStr("Automatic Scope Options"), &rbot->autoscope, autoscopeoptions);
//			}
//
//			InsertCheckbox(prefer_body, XorStr("Prefer body-aim") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->prefer_body);
//
//			std::vector<MultiItem_t> prefer_body_cond = {
//				//	{ XorStr( "Target firing" ), &rbot->prefer_body_disable_shot },
//				{ XorStr("Target resolved"), &rbot->prefer_body_disable_resolved },
//				//	{ XorStr( "Safe point headshot" ), &rbot->prefer_body_disable_safepoint_head }
//			};
//
//			if (rbot->prefer_body) {
//				InsertMultiCombo(XorStr("Prefer body-aim disablers##PreferBody"), prefer_body_cond);
//			}
//
//			//InsertCheckbox(AccuracyBoost, XorStr("Accuracy boost") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->accry_boost_on_shot);
//
//			//if (rbot->accry_boost_on_shot) {
//			//	InsertSliderFloat(std::string(XorStr("Accuracy Boost amount") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->hitchance_accuracy, 0.f, 100.f, XorStr("%.0f%%"));
//			//}
//
//			//const char* accuracyonshotmodes[] = { XorStr("Off"), XorStr("Low"), XorStr("Medium"), XorStr("High") };
//			//if (rbot->accry_boost_on_shot) {
//			//	InsertCombo(XorStr("Accuracy boost modes"), &rbot->accry_boost_on_shot_modes, accuracyonshotmodes);
//			//}
//
//			InsertCheckbox(shotdelay, XorStr("Delay hitbox selection") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->shotdelay);
//			InsertCheckbox(delay_shot_on_unducking, XorStr("Delay shot on unduck") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->delay_shot_on_unducking);
//		}
//
//		if (rageTab == -1) {
//
//			InsertCheckbox(knife_bot, XorStr("Knife bot"), &g_Vars.misc.knife_bot);
//			const char* knife_bot_type[] = { XorStr("Default"), XorStr("Backstab"), XorStr("Quick") };
//			if (g_Vars.misc.knife_bot) {
//				InsertCombo(XorStr("Knifebot type##Knife bot type"), &g_Vars.misc.knife_bot_type, knife_bot_type);
//			}
//
//			InsertCheckbox(zeus_bot, XorStr("Zeus bot"), &g_Vars.misc.zeus_bot);
//			if (g_Vars.misc.zeus_bot) {
//				InsertSliderFloat(XorStr("Zeus bot hitchance"), &g_Vars.misc.zeus_bot_hitchance, 1.f, 80.f, XorStr("%.0f%%"));
//			}
//		}
//
//		ImGui::NextColumn();
//		ImGui::NewLine();
//
//		if (rageTab == -1) {
//			ImGui::Text(XorStr("Force bodyaim"));
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(std::string(XorStr("##Force bodyaim key") + std::to_string(rageTab)).c_str(), &g_Vars.rage.prefer_body.key, &g_Vars.rage.prefer_body.cond, ImVec2{ 40,20 });
//
//
//			ImGui::Text(std::string(XorStr("Override resolver")).c_str());
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(std::string(XorStr("##Override resolver key") + std::to_string(rageTab)).c_str(), &g_Vars.rage.override_reoslver.key, &g_Vars.rage.override_reoslver.cond, ImVec2{ 40,20 });
//		}
//
//		if (rageTab == -1) {
//			InsertCheckbox(Doubletap, XorStr("Doubletap") + std::string(XorStr("##") + std::to_string(rageTab)), &g_Vars.rage.exploit);
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey("##DTkey", &g_Vars.rage.key_dt.key, &g_Vars.rage.key_dt.cond, ImVec2{ 40,20 });
//		}
//
//		if (g_Vars.rage.exploit) {
//			if (rageTab != -1) {
//				InsertSliderFloat(XorStr("Doubletap Hitchance"), &rbot->doubletap_hitchance, 1.f, 100.f, XorStr("%.0f%%"));
//				InsertSliderInt(XorStr("Doubletap Minimum Dmg"), &rbot->doubletap_dmg, 1, 100, XorStr("%d"));
//				InsertSliderInt(XorStr("Doubletap Speed"), &rbot->doubletap_speed, 11, 18, XorStr("%d"));
//			}
//
//			if (rageTab == -1) {
//
//				InsertCheckbox(DisableBTonDT, XorStr("Disable Backtrack on DT") + std::string(XorStr("##") + std::to_string(rageTab)), &g_Vars.misc.disablebtondt);
//				InsertCheckbox(exploits_enable, XorStr("DT Exploits"), &g_Vars.rage.dt_exploits);
//				InsertCheckbox(DoubletapBoostTeleport, XorStr("Boost Teleport"), &g_Vars.rage.dt_defensive_teleport);
//
//				InsertCheckbox(DesyncOnDT, XorStr("Desync on DT"), &g_Vars.antiaim.desync_on_dt);
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##Desync Inverter key"), &g_Vars.antiaim.desync_on_dt_invert.key, &g_Vars.antiaim.desync_on_dt_invert.cond, ImVec2{ 40,20 });
//
//				if (g_Vars.rage.dt_exploits) {
//					std::vector<MultiItem_t> exploits = {
//						{ XorStr("Lag exploit"), &g_Vars.rage.exploit_lag },
//						{ XorStr("Lag peek"), &g_Vars.rage.exploit_lag_peek },
//						{ XorStr("Break Lag Compensation"), &g_Vars.rage.exploit_lagcomp },
//					};
//
//					InsertMultiCombo(XorStr("Exploits"), exploits);
//				}
//			}
//		}
//		
//
//		if (rageTab != -1) {
//			InsertCheckbox(MinDmgOverride, XorStr("Damage override") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->min_damage_override);
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(XorStr("##MinDamageOverride"), &g_Vars.rage.key_dmg_override.key, &g_Vars.rage.key_dmg_override.cond, ImVec2{ 40,20 });
//			//ImGui::Keybind(std::string(XorStr("Minimum dmg override key#key") + std::string(XorStr("#") + std::to_string(rage_current_group))).c_str(), &g_Vars.rage.key_dmg_override.key);
//			if (rbot->min_damage_override) {
//				InsertSliderInt(std::string(XorStr("Dmg override amount##slider") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), &rbot->min_damage_override_amount, 1, 130, std::string(rbot->min_damage_override_amount > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_override_amount - 100)))) : XorStr("%d hp")).c_str());
//			}
//
//			InsertCheckbox(OverrideHitscan, XorStr("Override hitscan") + std::string(XorStr("##") + std::to_string(rageTab)), &rbot->override_hitscan);
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(XorStr("##OverrideHitscanKey"), &g_Vars.rage.override_key.key, &g_Vars.rage.override_key.cond, ImVec2{ 40,20 });
//
//			if (rbot->override_hitscan) {
//				std::vector<MultiItem_t> override_hitboxes = {
//					{ XorStr("Head"), &rbot->bt_hitboxes_head },
//					{ XorStr("Chest"), &rbot->bt_hitboxes_chest },
//					{ XorStr("Stomach"), &rbot->bt_hitboxes_stomach },
//					{ XorStr("Pelvis"), &rbot->bt_hitboxes_pelvis },
//					{ XorStr("Arms"), &rbot->bt_hitboxes_arms },
//					{ XorStr("Legs"), &rbot->bt_hitboxes_legs },
//					{ XorStr("Feet"), &rbot->bt_hitboxes_feets },
//				};
//
//				InsertMultiCombo(std::string(XorStr("Override hitboxes") + std::string(XorStr("##") + std::to_string(rageTab))).c_str(), override_hitboxes);
//			}
//		}
//
//	}
//	ImGui::EndColumns();
//}
//
//void HvH()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//
//	const char* styles[]{ XorStr("Static"), XorStr("Jitter"), XorStr("Spin") };
//
//	const char* pitches[] = { XorStr("Off"), XorStr("Down"), XorStr("Up"), XorStr("Zero") };
//
//	const char* real_yaw[] = { XorStr("Off"), XorStr("180"), XorStr("Rotate"), XorStr("Jitter"), XorStr("180z") };
//
//	const char* fake_yaw[] = { XorStr("Off"), XorStr("Static") };
//
//	const char* freestand_mode[] = { XorStr("Crosshair"), XorStr("Thickness") };
//
//	const char* mind_trick_mode[] = { XorStr("Offensive"), XorStr("Defensive") };
//
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//	ImGui::NewLine();
//	{
//		switch (AAtab) {
//			case 0:
//			{
//				CVariables::ANTIAIM_STATE* settings = &g_Vars.antiaim_stand;
//				// enable AA.
//				InsertCheckbox(AntiAim, XorStr("Anti-Aim"), &g_Vars.antiaim.enabled)
//
//				InsertCheckbox(EdgeAntiaim, XorStr("Edge antiaim"), &g_Vars.antiaim.edge_aa)
//
//				InsertCombo(XorStr("Pitch"), &settings->pitch, pitches);
//
//				InsertCombo(XorStr("Real yaw"), &settings->base_yaw, real_yaw);
//
//				if (settings->base_yaw == 3) {
//					InsertSliderInt(XorStr("Jitter range"), &g_Vars.antiaim.Jitter_range, -100, 100, XorStr("%d"));
//				}
//
//				if (settings->base_yaw == 2) {
//					InsertSliderInt(XorStr("Rotation range"), &g_Vars.antiaim.rot_range, 1, 360, XorStr("%d"));
//					InsertSliderInt(XorStr("Rotation speed"), &g_Vars.antiaim.rot_speed, 1, 100, XorStr("%d"));
//				}
//
//				InsertCombo(XorStr("Fake yaw"), &settings->yaw, fake_yaw);
//
//				// static lets choose our own vaule.
//				if (settings->yaw == 1) {
//					InsertSliderInt(XorStr("Break angle"), &g_Vars.antiaim.break_lby, -180, 180, XorStr("%d"));
//					InsertSliderInt(XorStr("First break angle"), &g_Vars.antiaim.break_lby_first, -180, 180, XorStr("%d"));
//					InsertCheckbox(StaticAngle, XorStr("Static angle"), &g_Vars.antiaim.static_angle)
//				}
//
//				//InsertCheckbox(FlickUp, XorStr("Flick LBY Up"), &g_Vars.antiaim.flickup);
//
//				//InsertCheckbox(AntiAimPreserve, XorStr("Preserve stand yaw"), &g_Vars.antiaim.preserve);
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCheckbox(BackwardsInAir, XorStr("Backwards in air"), &g_Vars.antiaim.backwards_in_air);
//
//				InsertCheckbox(AtTargets, XorStr("At targets"), &g_Vars.antiaim.at_targets);
//
//				InsertCheckbox(AntiLastmove, XorStr("Anti lastmove"), &g_Vars.antiaim.anti_lastmove);
//
//				InsertCheckbox(AntiAimFreestand, XorStr("Freestand yaw"), &g_Vars.antiaim.freestand);
//				if (g_Vars.antiaim.freestand) {
//					InsertCombo(XorStr("Freestand mode"), &g_Vars.antiaim.freestand_mode, freestand_mode);
//
//					InsertSliderInt(XorStr("Timeout time"), &g_Vars.antiaim.timeout_time, 0, 10, XorStr("%d"));
//
//					InsertSliderInt(XorStr("Add yaw"), &g_Vars.antiaim.add_yaw, -180.f, 180.f, XorStr("%d"));
//
//					std::vector<MultiItem_t> freestand_disablers = {
//						{ XorStr("Fakewalking"), &g_Vars.antiaim.freestand_disable_fakewalk },
//						{ XorStr("Running"), &g_Vars.antiaim.freestand_disable_run },
//						{ XorStr("Airborne"), &g_Vars.antiaim.freestand_disable_air },
//					};
//
//					InsertCheckbox(LockFreestand, XorStr( "Lock freestand" ), &g_Vars.antiaim.freestand_lock );
//					InsertMultiCombo(XorStr("Freestand disablers"), freestand_disablers);
//				}
//
//				// distortion.
//				InsertCheckbox(Distortion, XorStr("Distortion###sse"), &g_Vars.antiaim.distort)
//				if (g_Vars.antiaim.distort) {
//					InsertCheckbox(AAManualOverride, XorStr("Override Manual Distortion"), &g_Vars.antiaim.distort_manual_aa);
//					InsertCheckbox(Twist, XorStr("Twist"), &g_Vars.antiaim.distort_twist);
//					InsertSliderFloat(XorStr("Speed"), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr("%.1fs"));
//					InsertSliderFloat(XorStr("Max time"), &g_Vars.antiaim.distort_max_time, 0.f, 10.f, XorStr("%.f"));
//					InsertSliderFloat(XorStr("Range"), &g_Vars.antiaim.distort_range, -360.f, 360.f, XorStr("%.f"));
//
//					std::vector<MultiItem_t> distort_disablers = {
//						{ XorStr("Fakewalking"), &g_Vars.antiaim.distort_disable_fakewalk },
//						{ XorStr("Running"), &g_Vars.antiaim.distort_disable_run },
//						{ XorStr("Airborne"), &g_Vars.antiaim.distort_disable_air },
//					};
//
//					InsertMultiCombo(XorStr("Distortion disablers"), distort_disablers);
//				}
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCheckbox(AntiAimManual, XorStr("Manual"), &g_Vars.antiaim.manual);
//				if (g_Vars.antiaim.manual) {
//					InsertCheckbox(AntiAimManualarrows, XorStr("Manual arrows"), &g_Vars.antiaim.manual_arrows);
//					ColorPicker(XorStr("Manual color"), g_Vars.antiaim.manual_color, true, false);
//
//					ImGui::Text(XorStr("Left"));
//					ImGui::SameLine();
//					biggestMeme2();
//					ImGui::Hotkey(XorStr("##Left key"), &g_Vars.antiaim.manual_left_bind.key, &g_Vars.antiaim.manual_left_bind.cond, ImVec2{ 40,20 });
//
//					ImGui::Text(XorStr("Right"));
//					ImGui::SameLine();
//					biggestMeme2();
//					ImGui::Hotkey(XorStr("##Right key"), &g_Vars.antiaim.manual_right_bind.key, &g_Vars.antiaim.manual_right_bind.cond, ImVec2{ 40,20 });
//
//					ImGui::Text(XorStr("Back"));
//					ImGui::SameLine();
//					biggestMeme2();
//					ImGui::Hotkey(XorStr("##Back key"), &g_Vars.antiaim.manual_back_bind.key, &g_Vars.antiaim.manual_back_bind.cond, ImVec2{ 40,20 });
//				}
//
//				InsertCheckbox(JediMindTrick, XorStr("Jedi Mind-Trick"), &g_Vars.misc.mind_trick);
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##Mind-Trick key"), &g_Vars.misc.mind_trick_bind.key, &g_Vars.misc.mind_trick_bind.cond, ImVec2{ 40,20 });
//				if (g_Vars.misc.mind_trick) {
//					InsertCombo(XorStr("Mind-Trick Mode"), &g_Vars.misc.mind_trick_mode, mind_trick_mode);
//					InsertCheckbox(BypassMrx, XorStr("Bypass Mrx"), &g_Vars.misc.bypass_mrx);
//					//if (g_Vars.misc.mind_trick_mode == 1) {
//					//	InsertSliderInt(XorStr("Factor"), &g_Vars.misc.mind_trick_factor, 1, 16, XorStr("%d"));
//					//}
//				}
//					//ImGui::Text(XorStr("Invert"));
//					//ImGui::SameLine();
//					//biggestMeme2();
//					//ImGui::Hotkey(XorStr("##Mind-Trick invert"), &g_Vars.misc.mind_trick_invert.key, &g_Vars.misc.mind_trick_invert.cond, ImVec2{ 40,20 });
//
//				InsertCheckbox(MoveExploit, XorStr("Move Exploit"), &g_Vars.misc.move_exploit);
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##Move Exploit key"), &g_Vars.misc.move_exploit_key.key, &g_Vars.misc.move_exploit_key.cond, ImVec2{ 40,20 });
//				if (g_Vars.misc.move_exploit) {
//					InsertSliderInt(XorStr("Move Exploit intensity"), &g_Vars.misc.move_exploit_intensity, 1, 16, XorStr("%d"));
//				}
//
//
//				InsertCheckbox(FakeWalk, XorStr("Fake-walk"), &g_Vars.misc.slow_walk);
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##Fake-walk key"), &g_Vars.misc.slow_walk_bind.key, &g_Vars.misc.slow_walk_bind.cond, ImVec2{ 40,20 });
//				if (g_Vars.misc.slow_walk) {
//					InsertSliderInt(XorStr("Fake-walk speed"), &g_Vars.misc.slow_walk_speed, 2, 17, XorStr("%d"));
//					InsertSliderFloat(XorStr("Slow-walk speed"), &g_Vars.misc.slowwalk_speed, 0.1f, 0.3f, XorStr("%.1f"));
//				}
//
//				break;
//
//			}
//			case 1:
//			{
//				InsertCheckbox(Fakelag, XorStr("Fakelag##fakeKURWAlag"), &g_Vars.fakelag.enabled);
//
//				std::vector<MultiItem_t> fakelag_cond = { { XorStr("Standing"), &g_Vars.fakelag.when_standing }, { XorStr("Moving"), &g_Vars.fakelag.when_moving }, { XorStr("In air"), &g_Vars.fakelag.when_air } };
//				std::vector<MultiItem_t> fakelag_triggers = { { XorStr("On Peek"), &g_Vars.fakelag.trigger_on_peek }, { XorStr("On Duck"), &g_Vars.fakelag.trigger_duck }, { XorStr("On Land"),& g_Vars.fakelag.trigger_land }, { XorStr("On Weapon Activity"),&g_Vars.fakelag.trigger_weapon_activity } };
//				InsertMultiCombo(XorStr("Conditions"), fakelag_cond);
//				InsertMultiCombo(XorStr("Triggers"), fakelag_triggers);
//
//				const char* FakelagType[] = { XorStr("Maximum"), XorStr("Dynamic"), XorStr("Fluctuate") };
//				InsertCombo(XorStr("Type"), &g_Vars.fakelag.choke_type, FakelagType);
//				InsertSliderInt(XorStr("Limit"), &g_Vars.fakelag.choke, 0, 16, "%d");
//				InsertSliderInt(XorStr("Trigger Limit"), &g_Vars.fakelag.alternative_choke, 0, 16, "%d");
//
//				//g_Vars.fakelag.trigger_duck = g_Vars.fakelag.trigger_weapon_activity = g_Vars.fakelag.trigger_shooting = false;
//				//g_Vars.fakelag.trigger_land = true;
//				//g_Vars.fakelag.alternative_choke = 0;
//
//				InsertSliderFloat(XorStr("Variance"), &g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr("%.0f %%"));
//
//				InsertCheckbox(Fakelagonshot, XorStr("Fakelag on shot"), &g_Vars.fakelag.fakelag_onshot);
//
//				InsertCheckbox(Visuallizelag, XorStr("Visualize lag"), &g_Vars.fakelag.vis_lag);
//				ColorPicker(XorStr("##Visualizelagcolor"), g_Vars.fakelag.vis_lag_color, true, false);
//				break;
//
//			}
//		}
//		ImGui::EndColumns();
//	}
//}
//void Visuals()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//
//	ImGui::NewLine();
//	{
//		switch (visualsSubTab) {
//			case 0:
//			{
//				InsertCheckbox(esp_enable, XorStr("ESP"), &g_Vars.esp.esp_enable);
//				g_Vars.esp.team_check = true;
//
//				InsertCheckbox(fade_esp, XorStr("Dormant"), &g_Vars.esp.fade_esp);
//				//ImGuiEx::ColorEdit4( XorStr( "dormant clr" ), &g_Vars.esp.dormant_color );
//
//				InsertCheckbox(box, XorStr("Box"), &g_Vars.esp.box);
//				if (g_Vars.esp.box) {
//					ColorPicker(XorStr("##Box"), g_Vars.esp.box_color, false, false);
//				}
//
//				InsertCheckbox(offscren_enabled, XorStr("Offscreen"), &g_Vars.esp.offscren_enabled);
//				if (g_Vars.esp.offscren_enabled) {
//					ColorPicker(XorStr("##OffscreenColor"), g_Vars.esp.offscreen_color, false, false);
//					InsertSliderFloat(XorStr("Offscreen distance"), &g_Vars.esp.offscren_distance, 10, 100.f, XorStr("%.0f%%"));
//					InsertSliderFloat(XorStr("Offscreen size"), &g_Vars.esp.offscren_size, 4, 16.f, XorStr("%.0fpx"));
//				}
//
//				InsertCheckbox(name, XorStr("Name"), &g_Vars.esp.name);
//				if (g_Vars.esp.name) {
//					ColorPicker(XorStr("##NameColor"), g_Vars.esp.name_color, false, false);
//				}
//
//				InsertCheckbox(teamname, XorStr("Teammate Name"), &g_Vars.esp.teamname);
//				if (g_Vars.esp.teamname) {
//					ColorPicker(XorStr("##Name2Color"), g_Vars.esp.team_name_color, false, false);
//				}
//
//				InsertCheckbox(health, XorStr("Health"), &g_Vars.esp.health);
//				if (g_Vars.esp.health) {
//					ColorPicker(XorStr("##HealthColor"), g_Vars.esp.health_color, false, false);
//					InsertCheckbox(health_override, XorStr("Health color override"), &g_Vars.esp.health_override);
//				}
//
//				InsertCheckbox(skeleton, XorStr("Skeleton"), &g_Vars.esp.skeleton);
//				if (g_Vars.esp.skeleton) {
//					ColorPicker(XorStr("##SkeletonColor"), g_Vars.esp.skeleton_color, true, false);
//				}
//
//				InsertCheckbox(ammo, XorStr("Ammo"), &g_Vars.esp.draw_ammo_bar);
//				if (g_Vars.esp.draw_ammo_bar) {
//					ColorPicker(XorStr("##AmmoColor"), g_Vars.esp.ammo_color, false, false);
//				}
//				InsertCheckbox(lby_timer, XorStr("LBY timer"), &g_Vars.esp.draw_lby_bar);
//				if (g_Vars.esp.draw_lby_bar) {
//					ColorPicker("##LBYColor", g_Vars.esp.lby_color, false, false);
//				}
//
//				InsertCheckbox(weapon, XorStr("Weapon"), &g_Vars.esp.weapon);
//				if (g_Vars.esp.weapon) {
//					ColorPicker(XorStr("##WeaponColor"), g_Vars.esp.weapon_color, false, false);
//				}
//				InsertCheckbox(weapon_icon, XorStr("Weapon icon"), &g_Vars.esp.weapon_icon);
//				if (g_Vars.esp.weapon_icon) {
//					ColorPicker(XorStr("##WeaponIconColor"), g_Vars.esp.weapon_icon_color, false, false);
//				}
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				std::vector<MultiItem_t> flags = {
//					{ XorStr("Zoom"), &g_Vars.esp.draw_scoped },
//					{ XorStr("Vulnerable"), &g_Vars.esp.draw_vulnerable },
//					{ XorStr("Flashed"), &g_Vars.esp.draw_flashed },
//					{ XorStr("Money"), &g_Vars.esp.draw_money },
//					{ XorStr("Kevlar"), &g_Vars.esp.draw_armor },
//					{ XorStr("Bomb"), &g_Vars.esp.draw_bombc4 },
//					{ XorStr("Defusing"), &g_Vars.esp.draw_defusing },
//					{ XorStr("Distance"), &g_Vars.esp.draw_distance },
//					{ XorStr("Grenade pin"), &g_Vars.esp.draw_grenade_pin },
//					{ XorStr("Resolved"), &g_Vars.esp.draw_resolver },
//					{ XorStr("Ping"), &g_Vars.esp.draw_ping },
//					{ XorStr("Exploiting"), &g_Vars.esp.draw_exploiting },
//				};
//
//				InsertMultiCombo(XorStr("Flags"), flags);
//
//				std::vector<MultiItem_t> hitmarkers = {
//						{ XorStr("World"), &g_Vars.esp.visualize_hitmarker_world },
//						{ XorStr("Screen"), &g_Vars.esp.vizualize_hitmarker },
//				};
//
//				InsertMultiCombo(XorStr("Hitmarkers"), hitmarkers);
//
//				InsertCheckbox(Hitsound, XorStr("Hitsound"), &g_Vars.misc.hitsound);
//				if (g_Vars.misc.hitsound) {
//					const char* hitsounds[] = { XorStr("Default"), XorStr("Custom"), XorStr("Bameware"), XorStr("Vader")};
//					InsertCombo(XorStr("Type"), &g_Vars.misc.hitsound_type, hitsounds);
//
//					if (g_Vars.misc.hitsound_type == 1) {
//						static char custom_hitsound[64] = "\0";
//
//						if (!g_Vars.misc.custom_hitsound.empty())
//							strcpy_s(custom_hitsound, sizeof(custom_hitsound), g_Vars.misc.custom_hitsound.c_str());
//
//						if (ImGui::InputText(XorStr("##Custom Hitsound"), custom_hitsound, sizeof(custom_hitsound)))
//							g_Vars.misc.custom_hitsound = custom_hitsound;
//
//						InsertSliderFloat(XorStr("Hitsound volume"), &g_Vars.misc.hitsound_volume, 1.f, 100.f, XorStr("%.f"));
//					}
//				}
//				InsertCheckbox(killsound, XorStr("Killsound"), &g_Vars.misc.killsound);
//				if (g_Vars.misc.killsound) {
//					static char custom_killsound[64] = "\0";
//
//					if (!g_Vars.misc.custom_killsound.empty())
//						strcpy_s(custom_killsound, sizeof(custom_killsound), g_Vars.misc.custom_killsound.c_str());
//
//					if (ImGui::InputText(XorStr("##Custom Killsound"), custom_killsound, sizeof(custom_killsound)))
//						g_Vars.misc.custom_killsound = custom_killsound;
//
//					InsertSliderFloat(XorStr("Killsound volume"), &g_Vars.misc.killsound_volume, 1.f, 100.f, XorStr("%.f"));
//				}
//				InsertCheckbox(FootSteps, XorStr("Footsteps"), &g_Vars.esp.footsteps);
//				if (g_Vars.esp.footsteps) {
//					ImGui::SameLine();
//					ColorPicker(XorStr("footsteps color"), g_Vars.esp.footsteps_color, true, false);
//					InsertSliderFloat(XorStr("Footsteps thickness"), &g_Vars.esp.footsteps_thickness, 1.f, 10.f, XorStr("%.f"));
//					InsertSliderFloat(XorStr("Footsteps radius"), &g_Vars.esp.footsteps_radius, 50.f, 500.f, XorStr("%.f"));
//
//				}
//
//				break;
//			}
//			case 1:
//			{
//				const char* skyboxes[]{ XorStr("Default"), XorStr("cs_baggage_skybox"), XorStr("cs_tibet"), XorStr("embassy"), XorStr("italy"), XorStr("jungle"), XorStr("nukeblank"), XorStr("office"), XorStr("sky_csgo_cloudy01"), XorStr("sky_csgo_night02"), XorStr("sky_csgo_night02b"), XorStr("sky_dust"), XorStr("sky_venice"), XorStr("vertigo"), XorStr("vietnamsky_descent"), XorStr("Custom")};
//				const char* override_textures[]{ XorStr("Disabled"), XorStr("Dev (Grey)"), XorStr("Dev (Orange)"), XorStr("Custom") };
//
//				std::vector<MultiItem_t> worldAdjustment = {
//					{ XorStr("Nightmode"), &g_Vars.esp.night_mode },
//					{ XorStr("Fullbright"), &g_Vars.esp.fullbright },
//					{ XorStr("Skybox Color"), &g_Vars.esp.skybox },
//				};
//
//				InsertMultiCombo(std::string(XorStr("World Adjustment")).c_str(), worldAdjustment);
//
//				if (g_Vars.esp.skybox) {
//					ColorPicker_w_name(XorStr("Skybox Color"), g_Vars.esp.skybox_modulation, false, false);
//				}
//
//				if (g_Vars.esp.night_mode) {
//					InsertSliderFloat(XorStr("World brightness"), &g_Vars.esp.world_adjustement_value, 1.f, 100.0f, XorStr("%.0f%%"));
//					InsertSliderFloat(XorStr("Prop brightness"), &g_Vars.esp.prop_adjustement_value, 1.f, 100.0f, XorStr("%.0f%%"));
//					InsertSliderFloat(XorStr("Prop transparency"), &g_Vars.esp.transparent_props, 0.f, 100.0f, XorStr("%.0f%%"));
//				}
//				
//
//				InsertCombo(XorStr("Override world textures"), &g_Vars.esp.custom_world_textures, override_textures);
//				if (g_Vars.esp.custom_world_textures == 3) {
//					ImGui::InputText(XorStr("##CustomTexture"), &g_Vars.esp.custom_world_texture_string);
//				}
//				
//				//InsertCheckbox(EnableRain, XorStr("Rain"), &g_Vars.esp.weather);
//				//if (g_Vars.esp.weather) {
//				//	InsertSliderFloat(XorStr("Rain alpha"), &g_Vars.esp.weather_alpha, 0.f, 100.0f, XorStr("%.f"));
//				//	InsertCheckbox(EnableThunderSounds, XorStr("Thunder Sounds"), &g_Vars.esp.weather_thunder);
//				//}
//
//				InsertCheckbox(Enablemolotovcolor, XorStr("Molotov Color"), &g_Vars.esp.molotov_color_enable);
//				ColorPicker(XorStr("##MolotovColor"), g_Vars.esp.molotov_color, false, false);
//
//				InsertCheckbox(EnableDlight, XorStr("Dlight Players"), &g_Vars.esp.dlight_players_enable);
//				if (g_Vars.esp.dlight_players_enable) {
//					InsertCheckbox(EnableDlightlocal, XorStr("Dlight local"), &g_Vars.esp.dlight_local_enable);
//					if (g_Vars.esp.dlight_local_enable) {
//						ColorPicker(XorStr("Local Dlight"), g_Vars.esp.dlight_local_color, true, false);
//						InsertSliderInt(XorStr("Local Dlight radius"), &g_Vars.esp.dlight_local_radius, 0, 275, XorStr("%d"));
//					}
//					InsertCheckbox(EnableDlightenemy, XorStr("Dlight enemy"), &g_Vars.esp.dlight_enemy_enable);
//					if (g_Vars.esp.dlight_enemy_enable) {
//						ColorPicker(XorStr("Enemy Dlight"), g_Vars.esp.dlight_enemy_color, true, false);
//						InsertSliderInt(XorStr("Enemy Dlight radius"), &g_Vars.esp.dlight_enemy_radius, 0, 275, XorStr("%d"));
//					}
//				}
//
//				InsertCheckbox(EnableSunset, XorStr("Sunset mode"), &g_Vars.esp.sunset_enable);
//				if (g_Vars.esp.sunset_enable) {
//					InsertSliderFloat(XorStr("Sunset y"), &g_Vars.esp.sunset_rot_y, 0.f, 360.0f, XorStr("%.1f"));
//					InsertSliderFloat(XorStr("Sunset x"), &g_Vars.esp.sunset_rot_x, 0.f, 360.0f, XorStr("%.1f"));
//				}
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCombo(XorStr("Skybox Changer"), &g_Vars.esp.sky_changer, skyboxes);
//				if (g_Vars.esp.sky_changer == 15) {
//
//					static char sky_custom[64] = "\0";
//
//					if (!g_Vars.esp.custom_skybox.empty())
//						strcpy_s(sky_custom, sizeof(sky_custom), g_Vars.esp.custom_skybox.c_str());
//
//					ImGui::Text(XorStr("Name"));
//					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
//
//					if (ImGui::InputText(XorStr("##customsky"), sky_custom, sizeof(sky_custom)))
//						g_Vars.esp.custom_skybox = sky_custom;
//
//					ImGui::PopStyleVar();
//
//				}
//
//				InsertCheckbox(bomb_bar, XorStr("Bomb Timer"), &g_Vars.esp.draw_c4_bar);
//				if (g_Vars.esp.draw_c4_bar) {
//					ColorPicker(XorStr("##BombColor"), g_Vars.esp.c4_color, false, false);
//				}
//
//				std::vector<MultiItem_t> droppedWeapons = {
//					{ XorStr("Text"), &g_Vars.esp.dropped_weapons },
//					{ XorStr("Ammo"), &g_Vars.esp.dropped_weapons_ammo },
//				};
//
//				InsertMultiCombo(std::string(XorStr("Dropped Weapons")).c_str(), droppedWeapons);
//				ColorPicker(XorStr("##DropperWeaponsColor"), g_Vars.esp.dropped_weapons_color, false, true);
//
//				const char* shot_visualization_options[] = { XorStr("Off"), XorStr("Hitboxes"), XorStr("Chams") };
//
//				InsertCombo(XorStr("Shot Visualization"), &g_Vars.esp.shot_visualization, shot_visualization_options);
//				switch (g_Vars.esp.shot_visualization) {
//				case 0:
//				{
//
//
//					break;
//				}
//				case 1:
//				{
//					ColorPicker_w_name(XorStr("Hitboxes color"), g_Vars.esp.hitboxes_color, true, false);
//					break;
//				}
//				case 2:
//				{
//					const char* shot_chams_mat[] = { XorStr("Glow"), XorStr("Flat") };
//
//					InsertCombo(XorStr("Shot Material"), &g_Vars.esp.new_chams_onshot_mat, shot_chams_mat); 
//					if(g_Vars.esp.new_chams_onshot_mat == 0) {
//						InsertSliderFloat(XorStr("Glow strength ##shot"), &g_Vars.esp.new_chams_onshot_mat_glow_value, 0.f, 100.f, XorStr("%.f"));
//					}
//					ColorPicker_w_name(XorStr("Shot chams color"), g_Vars.esp.hitmatrix_color, true, false);
//					InsertSliderFloat(XorStr("Expire time ##chams"), &g_Vars.esp.hitmatrix_time, 1.f, 10.f, XorStr("%0.0f seconds"));
//					break;
//				}
//				//case 3:
//				//{
//				//	ColorPicker_w_name(XorStr("Shot Skeleton color"), g_Vars.esp.hitskeleton_color, true, false);
//				//	InsertSliderFloat(XorStr("Expire time ##skeleton"), &g_Vars.esp.hitskeleton_time, 1.f, 10.f, XorStr("%0.0f seconds"));
//				//	break;
//				//}
//
//
//				}
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCheckbox(Grenades, XorStr("Grenade ESP"), &g_Vars.esp.nades);
//				InsertCheckbox(GrenadesPrediction, XorStr("Grenade Prediction"), &g_Vars.esp.NadePred);
//				if (g_Vars.esp.NadePred) {
//					ColorPicker(XorStr("##GrenadeColor"), g_Vars.esp.nade_pred_color, false, false);
//				}
//				//InsertCheckbox(GrenadesTracer, XorStr("Grenade Tracer"), &g_Vars.esp.NadeTracer);
//				//if (g_Vars.esp.NadeTracer) {
//				//	ColorPicker(XorStr("##GrenadeTracerColor"), g_Vars.esp.nade_tracer_color, true, false);
//				//}
//				InsertCheckbox(GrenadeWarning, XorStr("Grenade Warning"), &g_Vars.esp.Grenadewarning);
//				if (g_Vars.esp.Grenadewarning) {
//					ColorPicker(XorStr("##GrenadeWarningColor"), g_Vars.esp.Grenadewarning_color, false, false);
//					InsertCheckbox(GrenadeWarningTracer, XorStr("Tracer"), &g_Vars.esp.Grenadetracer);
//					ColorPicker(XorStr("##GrenadeWarningTracerColor"), g_Vars.esp.Grenadetracer_color, true, false);
//				}
//
//				InsertSliderFloat(XorStr("Override FOV"), &g_Vars.esp.world_fov, 0.f, 200.f, XorStr("%.0f degress"));
//				InsertSliderFloat(XorStr("Viewmodel FOV"), &g_Vars.misc.viewmodel_fov, 0.f, 200.f, XorStr("%.0f degress"));
//				InsertCheckbox(AspectRatio, XorStr("Aspect Ratio"), &g_Vars.esp.aspect_ratio);
//				if (g_Vars.esp.aspect_ratio) {
//					InsertSliderFloat(XorStr("Aspect ratio value"), &g_Vars.esp.aspect_ratio_value, 0.02f, 5.f, XorStr("%.2f"), 0.01f);
//				}
//
//				break;
//			}
//			case 2:
//			{
//				InsertCheckbox(PreserveKillfeed, XorStr("Preserve Killfeed"), &g_Vars.esp.preserve_killfeed);
//				//InsertCheckbox(VisualizeDamage, XorStr("Visualize Damage"), &g_Vars.esp.visualize_damage); // disabled for now, very buggy.
//				InsertCheckbox(SpreadCrosshair, XorStr("Spread Crosshair"), &g_Vars.esp.spread_crosshair);
//				if (g_Vars.esp.spread_crosshair) {
//					ColorPicker(XorStr("##SpreadCrosshairColor"), g_Vars.esp.spread_crosshair_color, true, false);
//				}
//				InsertCheckbox(TaserRange, XorStr("Taser Range"), &g_Vars.esp.zeus_distance);
//				InsertCheckbox(KeybindList, XorStr("Keybind List"), &g_Vars.esp.keybind_window_enabled);
//				InsertCheckbox(SpectatorList, XorStr("Spectator List"), &g_Vars.esp.spec_window_enabled);
//				InsertCheckbox(Radar, XorStr("Radar"), &g_Vars.misc.ingame_radar);
//				InsertCheckbox(thirdperson, XorStr("Thirdperson"), &g_Vars.misc.third_person);
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##Thirdperson Key"), &g_Vars.misc.third_person_bind.key, &g_Vars.misc.third_person_bind.cond, ImVec2{ 40,20 });
//				if (g_Vars.misc.third_person) {
//					InsertCheckbox(DisableOnGrenade, XorStr("Disable On Grenade"), &g_Vars.misc.third_person_on_grenade);
//					InsertSliderFloat(XorStr("Thirdperson Distance"), &g_Vars.misc.third_person_dist, 0.f, 250.f, XorStr("%.0f%%"));
//				}
//
//
//				std::vector<MultiItem_t> removals = {
//					{ XorStr("Smoke"), &g_Vars.esp.remove_smoke },
//					{ XorStr("Flashbang"), &g_Vars.esp.remove_flash },
//					{ XorStr("Scope"), &g_Vars.esp.remove_scope },
//					{ XorStr("Zoom"), &g_Vars.esp.remove_scope_zoom },
//					{ XorStr("Recoil Shake"), &g_Vars.esp.remove_recoil_shake },
//					{ XorStr("Recoil Punch"), &g_Vars.esp.remove_recoil_punch },
//					{ XorStr("View Bob"), &g_Vars.esp.remove_bob },
//					{ XorStr("Sleeves"), &g_Vars.esp.remove_sleeves },
//					{ XorStr("Post Processing"), &g_Vars.esp.remove_post_proccesing },
//				};
//
//				InsertMultiCombo(std::string(XorStr("Removals")).c_str(), removals);
//
//				const char* tracers[]{ XorStr("Line"), XorStr("Beam") };
//				const char* beam_models[]{ XorStr("Laserbeam"), XorStr("Purplelaser"), XorStr("Physbeam") };
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCheckbox(ForceCrosshair, XorStr("Force Crosshair"), &g_Vars.esp.force_sniper_crosshair);
//				InsertCheckbox(PenetrationCrosshair, XorStr("Penetration Crosshair"), &g_Vars.esp.autowall_crosshair);
//				InsertCheckbox(ViewModelOnScope, XorStr("Force Viewmodel On Scope"), &g_Vars.esp.force_viewmodel_scoped);
//				InsertCheckbox(ClientImpacts, XorStr("Client Impacts"), &g_Vars.misc.impacts_spoof);
//				if (g_Vars.misc.impacts_spoof) {
//					ColorPicker(XorStr("##ClientImpactsColor"), g_Vars.esp.client_impacts, true, false);
//				}
//
//				InsertCheckbox(ServerImpacts, XorStr("Server Impacts"), &g_Vars.misc.server_impacts_spoof);
//				if (g_Vars.misc.server_impacts_spoof) {
//					ColorPicker(XorStr("##ServerColor"), g_Vars.esp.server_impacts, true, false);
//				}
//
//				InsertCheckbox(BulletTracers, XorStr("Bullet Tracers"), &g_Vars.esp.beam_enabled);
//				if (g_Vars.esp.beam_enabled) {
//					InsertCheckbox(EnemyTracers, XorStr("Enemy Tracers"), &g_Vars.esp.beam_enemy_enable);
//					if (g_Vars.esp.beam_enemy_enable) {
//						ColorPicker(XorStr("Enemy Tracer Color"), g_Vars.esp.beam_color_enemy, false, false);
//					}
//					InsertCheckbox(LocalTracers, XorStr("Local Tracers"), &g_Vars.esp.beam_local_enable);
//					if (g_Vars.esp.beam_local_enable) {
//						ColorPicker(XorStr("Local Tracer Color"), g_Vars.esp.beam_color_local, false, false);
//						//InsertCheckbox(RainbowTracerColor, XorStr("Rainbow Local Color"), &g_Vars.esp.beam_color_rainbow);
//					}
//
//
//					InsertCombo(XorStr("Bullet Tracer Type"), &g_Vars.esp.beam_type, tracers);
//				}
//				if (g_Vars.esp.beam_type == 1 && g_Vars.esp.beam_enabled) {
//					InsertCombo(XorStr("Beam Type"), &g_Vars.esp.beam_model, beam_models);
//				}
//
//				ImGui::NextColumn();
//				ImGui::NewLine();
//
//				InsertCheckbox(AmbientLighting, XorStr("Ambient Lighting"), &g_Vars.esp.ambient_ligtning);
//				if (g_Vars.esp.ambient_ligtning) {
//					ColorPicker(XorStr("##AmbientColor"), g_Vars.esp.ambient_ligtning_color, false, false);
//				}
//
//				InsertCheckbox(SkipOcclusion, XorStr("Skip Occlusion"), &g_Vars.esp.skip_occulusion);
//
//				InsertCheckbox(HaloAboveHead, XorStr("Halo"), &g_Vars.esp.halo_above_head);
//				if (g_Vars.esp.halo_above_head) {
//					ColorPicker(XorStr("Halo Color"), g_Vars.esp.halo_above_head_color, true, false);
//				}
//
//
//				InsertCheckbox(DrawAAAngles, XorStr("Draw Antiaim Angles"), &g_Vars.esp.draw_antiaim_angles);
//				if (g_Vars.esp.draw_antiaim_angles) {
//					ColorPicker_w_name(XorStr("Real Color"), g_Vars.esp.draw_antiaim_angles_real, false, false);
//					ColorPicker_w_name(XorStr("Lby Color"), g_Vars.esp.draw_antiaim_angles_lby, false, false);
//				}
//
//				InsertCheckbox(LocalSkeleton, XorStr("Local Skeleton"), &g_Vars.esp.local_skeleton);
//				if (g_Vars.esp.local_skeleton) {
//					ColorPicker(XorStr("Skeleton Color"), g_Vars.esp.local_skeleton_color, false, false);
//				}
//
//				InsertCheckbox(TeslaImpact, XorStr("Tesla on Impact"), &g_Vars.esp.tesla_impact);
//
//				InsertCheckbox(TeslaKill, XorStr("Tesla on Kill"), &g_Vars.esp.tesla_kill);
//				if (g_Vars.esp.tesla_kill) {
//					ColorPicker(XorStr("tesla kill color"), g_Vars.esp.tesla_kill_color, false, false);
//					InsertSliderFloat(XorStr("Tesla Width"), &g_Vars.esp.tesla_kill_width, 0.f, 10.f, XorStr("%.f"));
//					InsertSliderFloat(XorStr("Tesla Beams"), &g_Vars.esp.tesla_kill_beams, 0.f, 20.f, XorStr("%.f"));
//					InsertSliderFloat(XorStr("Tesla Radius"), &g_Vars.esp.tesla_kill_radius, 0.f, 200.f, XorStr("%.f"));
//					InsertSliderFloat(XorStr("Tesla Time"), &g_Vars.esp.tesla_kill_time, 0.1f, 3.f, XorStr("%.1f"));
//
//				}
//
//				break;
//			}
//			case 3:
//			{
//
//				const char* chams_mats[] = { XorStr("Disabled"),  XorStr("Texture"), XorStr("Flat"), XorStr("Custom") };
//				const char* chams_mats_local[] = { XorStr("Disabled"),  XorStr("Texture"), XorStr("Flat"), XorStr("Custom") };
//				const char* chams_mats_overlay_local[] = { XorStr("Disabled"), XorStr("Glow"), XorStr("Animated") };
//				const char* chams_mats_overlay[] = { XorStr("Disabled"), XorStr("Glow") };
//				const char* chams_mats_overlay_misc[] = { XorStr("Disabled"), XorStr("Glow") };
//				const char* glow_types[] = { XorStr("Standard"), XorStr("Pulse"), XorStr("Inner") };
//				const char* chams_mats_overlay_viewmodel[] = { XorStr("Disabled"), XorStr("Glow"), XorStr("Animated") };
//				const char* chams_filter_menu[] = { XorStr("Enemy"), XorStr("Local"), XorStr("Viewmodel"), XorStr("Glow") };
//				static int chams_filter = 0;
//
//				InsertCombo(XorStr("chams"), &chams_filter, chams_filter_menu);
//				switch (chams_filter)
//				{
//				case 0:
//				{
//
//					InsertCheckbox(chams_enemy, XorStr("Enemy chams"), &g_Vars.esp.chams_enemy);
//					InsertCheckbox(enemy_backtrack_chams, XorStr("Backtrack Chams"), &g_Vars.esp.chams_history);
//					ColorPicker(XorStr("##enemybacktrackcolor"), g_Vars.esp.chams_history_color, true, false);
//
//					if (g_Vars.esp.chams_enemy) {
//
//
//						ImGui::NextColumn();
//						ImGui::NewLine();
//
//						InsertCheckbox(enemy_vis_chams, XorStr("Enemy visible chams"), &g_Vars.esp.enemy_chams_vis);
//						if (g_Vars.esp.enemy_chams_vis) {
//
//							InsertCombo(XorStr("Enemy Visible"), &g_Vars.esp.new_chams_enemy, chams_mats);
//							ColorPicker(XorStr("##enemy_vis_color"), g_Vars.esp.new_chams_enemy_color, true, true);
//							if (g_Vars.esp.new_chams_enemy == 3) {
//
//								InsertSliderFloat(XorStr("Enemy visible pearlescence"), &g_Vars.esp.chams_enemy_pearlescence, 0.f, 100.f, XorStr("%.f"));
//								ColorPicker_w_name(XorStr("Visible pearlescence color"), g_Vars.esp.chams_enemy_pearlescence_color, true, false);
//								InsertSliderFloat(XorStr("Enemy visible shine"), &g_Vars.esp.chams_enemy_shine, 0.f, 100.f, XorStr("%.f"));
//
//							}
//							InsertCombo(XorStr("Enemy visible overlay"), &g_Vars.esp.new_chams_enemy_overlay, chams_mats_overlay);
//							ColorPicker(XorStr("##enemy_vis_overlay_color"), g_Vars.esp.new_chams_enemy_overlay_color, true, true);
//							if (g_Vars.esp.new_chams_enemy_overlay == 1) {
//
//								InsertSliderFloat(XorStr("Glow strength ##enemy"), &g_Vars.esp.chams_enemy_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//							}
//							InsertCheckbox(enemy_overlay_wireframe, XorStr("Enemy visible overlay wireframe"), &g_Vars.esp.chams_enemy_outline_wireframe);
//						}
//
//						ImGui::NextColumn();
//						ImGui::NewLine();
//
//						InsertCheckbox(enemy_xqz_chams, XorStr("Enemy XQZ chams"), &g_Vars.esp.enemy_chams_xqz);
//						if (g_Vars.esp.enemy_chams_xqz) {
//							InsertCombo(XorStr("Enemy XQZ"), &g_Vars.esp.new_chams_enemy_xqz, chams_mats);
//							ColorPicker(XorStr("##enemyxqzcolor"), g_Vars.esp.new_chams_enemy_xqz_color, true, true);
//							if (g_Vars.esp.new_chams_enemy_xqz == 3) {
//
//								InsertSliderFloat(XorStr("Enemy XQZ pearlescence"), &g_Vars.esp.chams_enemy_xqz_pearlescence, 0.f, 100.f, XorStr("%.f"));
//								ColorPicker_w_name(XorStr("XQZ pearlescence color"), g_Vars.esp.chams_enemy_xqz_pearlescence_color, true, false);
//								InsertSliderFloat(XorStr("Enemy xqz shine"), &g_Vars.esp.chams_enemy_xqz_shine, 0.f, 100.f, XorStr("%.f"));
//
//							}
//							InsertCombo(XorStr("Enemy xqz overlay"), &g_Vars.esp.new_chams_enemy_xqz_overlay, chams_mats_overlay);
//							ColorPicker(XorStr("##enemy_xqz_overlay_color"), g_Vars.esp.new_chams_enemy_xqz_overlay_color, true, true);
//							if (g_Vars.esp.new_chams_enemy_xqz_overlay == 1) {
//
//								InsertSliderFloat(XorStr("Glow strength ##enemyxqz"), &g_Vars.esp.chams_enemy_xqz_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//							}
//							InsertCheckbox(enemy_xqz_overlay_wireframe, XorStr("Enemy XQZ overlay wireframe"), &g_Vars.esp.chams_enemy_outline_xqz_wireframe);
//						}
//
//					}
//
//
//					break;
//				}
//				case 1: //local
//				{
//					ImGui::NextColumn();
//					ImGui::NewLine();
//
//					InsertCheckbox(enable_local_chams, XorStr("Local chams"), &g_Vars.esp.chams_local);
//					InsertCheckbox(chams_local_scoped_enabled, XorStr("Transparency when scoped"), &g_Vars.esp.blur_in_scoped);
//
//					if (g_Vars.esp.blur_in_scoped) {
//						InsertSliderFloat(XorStr("Scoped transparency"), &g_Vars.esp.blur_in_scoped_value, 0.0f, 100.f, XorStr("%.f"));
//					}
//
//					if (g_Vars.esp.chams_local) {
//
//						InsertCombo(XorStr("Local"), &g_Vars.esp.new_chams_local, chams_mats_local);
//						ColorPicker(XorStr("##local_color"), g_Vars.esp.chams_local_color, true, true);
//						if (g_Vars.esp.new_chams_local == 3) {
//
//							InsertSliderFloat(XorStr("Local pearlescence"), &g_Vars.esp.chams_local_pearlescence, 0.f, 100.f, XorStr("%.f"));
//							ColorPicker_w_name(XorStr("Local pearlescence color"), g_Vars.esp.chams_local_pearlescence_color, true, false);
//							InsertSliderFloat(XorStr("Local shine"), &g_Vars.esp.chams_local_shine, 0.f, 100.f, XorStr("%.f"));
//
//						}
//
//						InsertCombo(XorStr("Local overlay"), &g_Vars.esp.new_chams_local_overlay, chams_mats_overlay_local);
//						ColorPicker(XorStr("##local_overlay_color"), g_Vars.esp.new_chams_local_overlay_color, true, true);
//						if (g_Vars.esp.new_chams_local_overlay == 1) {
//
//							InsertSliderFloat(XorStr("Glow strength ##local"), &g_Vars.esp.chams_local_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//
//						}
//						InsertCheckbox(local_overlay_wireframe, XorStr("Local overlay wireframe"), &g_Vars.esp.chams_local_outline_wireframe);
//						InsertCheckbox(chams_local_original_model, XorStr("Draw original model ##local"), &g_Vars.esp.new_chams_local_original_model);
//					}
//
//					//ImGui::NextColumn();
//					//ImGui::NewLine();
//
//					ImGui::NextColumn();
//					ImGui::NewLine();
//
//					InsertCheckbox(enable_attachments_chams, XorStr("Attachment chams"), &g_Vars.esp.chams_attachments);
//					if (g_Vars.esp.chams_attachments) {
//						InsertCombo(XorStr("Attachment"), &g_Vars.esp.attachments_chams_mat, chams_mats_local);
//						ColorPicker(XorStr("##attachment_color"), g_Vars.esp.attachments_chams_color, true, true);
//						if (g_Vars.esp.attachments_chams_mat == 3) {
//
//							InsertSliderFloat(XorStr("Attachment pearlescence"), &g_Vars.esp.chams_attachments_pearlescence, 0.f, 100.f, XorStr("%.f"));
//							ColorPicker_w_name(XorStr("Attachment pearlescence color"), g_Vars.esp.chams_attachments_pearlescence_color, true, false);
//							InsertSliderFloat(XorStr("Attachment shine"), &g_Vars.esp.chams_attachments_shine, 0.f, 100.f, XorStr("%.f"));
//
//						}
//
//						InsertCombo(XorStr("Attachment overlay"), &g_Vars.esp.new_chams_attachments_overlay, chams_mats_overlay_misc);
//						ColorPicker(XorStr("##attachment_overlay_color"), g_Vars.esp.new_chams_attachments_overlay_color, true, true);
//						if (g_Vars.esp.new_chams_attachments_overlay == 1) {
//
//							InsertSliderFloat(XorStr("Glow strength ##attachments"), &g_Vars.esp.chams_attachments_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//
//						}
//						InsertCheckbox(attachments_overlay_wireframe, XorStr("Attachment overlay wireframe"), &g_Vars.esp.chams_attachments_outline_wireframe);
//						InsertCheckbox(chams_attachments_original_model, XorStr("Draw original model ##attachment"), &g_Vars.esp.new_chams_attachments_original_model);
//
//					}
//
//					break;
//				}
//				case 2: //hands & weapon
//				{
//
//					ImGui::NextColumn();
//					ImGui::NewLine();
//
//					InsertCheckbox(enable_hand_chams, XorStr("Hand chams"), &g_Vars.esp.chams_hands);
//					if (g_Vars.esp.chams_hands) {
//						InsertCombo(XorStr("Hands ##chams"), &g_Vars.esp.hands_chams_mat, chams_mats_local);
//						ColorPicker(XorStr("##hand_color"), g_Vars.esp.hands_chams_color, true, true);
//						if (g_Vars.esp.hands_chams_mat == 3) {
//
//							InsertSliderFloat(XorStr("Hand pearlescence"), &g_Vars.esp.chams_hands_pearlescence, 0.f, 100.f, XorStr("%.f"));
//							ColorPicker_w_name(XorStr("Hand pearlescence color"), g_Vars.esp.chams_hands_pearlescence_color, true, false);
//							InsertSliderFloat(XorStr("Hand shine"), &g_Vars.esp.chams_hands_shine, 0.f, 100.f, XorStr("%.f"));
//
//						}
//
//						InsertCombo(XorStr("Hand overlay"), &g_Vars.esp.new_chams_hands_overlay, chams_mats_overlay_viewmodel);
//						ColorPicker(XorStr("##hand_overlay_color"), g_Vars.esp.new_chams_hands_overlay_color, true, true);
//						if (g_Vars.esp.new_chams_hands_overlay == 1) {
//
//							InsertSliderFloat(XorStr("Glow strength ##hands"), &g_Vars.esp.chams_hands_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//
//						}
//						InsertCheckbox(hand_overlay_wireframe, XorStr("Hand overlay wireframe"), &g_Vars.esp.chams_hands_outline_wireframe);
//						InsertCheckbox(chams_hands_original_model, XorStr("Draw original model ##hands"), &g_Vars.esp.new_chams_hands_original_model);
//					}
//
//					ImGui::NextColumn();
//					ImGui::NewLine();
//
//					InsertCheckbox(enable_weapon_chams, XorStr("Weapon chams"), &g_Vars.esp.chams_weapon);
//					if (g_Vars.esp.chams_weapon) {
//
//						InsertCombo(XorStr("Weapon ##chams"), &g_Vars.esp.weapon_chams_mat, chams_mats_local);
//						ColorPicker(XorStr("##weapon_color"), g_Vars.esp.weapon_chams_color, true, true);
//						if (g_Vars.esp.weapon_chams_mat == 3) {
//
//							InsertSliderFloat(XorStr("Weapon pearlescence"), &g_Vars.esp.chams_weapon_pearlescence, 0.f, 100.f, XorStr("%.f"));
//							ColorPicker_w_name(XorStr("Weapon pearlescence color"), g_Vars.esp.chams_weapon_pearlescence_color, true, false);
//							InsertSliderFloat(XorStr("Weapon shine"), &g_Vars.esp.chams_weapon_shine, 0.f, 100.f, "%.f");
//
//						}
//
//						InsertCombo(XorStr("Weapon overlay"), &g_Vars.esp.new_chams_weapon_overlay, chams_mats_overlay_viewmodel);
//						ColorPicker(XorStr("##weapon_overlay_color"), g_Vars.esp.new_chams_weapon_overlay_color, true, true);
//						if (g_Vars.esp.new_chams_weapon_overlay == 1) {
//
//							InsertSliderFloat(XorStr("Glow strength ##weapon"), &g_Vars.esp.chams_weapon_outline_value, 0.f, 100.f, XorStr("%.f"));
//
//
//						}
//						InsertCheckbox(weapon_overlay_wireframe, XorStr("Weapon overlay wireframe"), &g_Vars.esp.chams_weapon_outline_wireframe);
//						InsertCheckbox(chams_weapon_original_model, XorStr("Draw original model ##weapon"), &g_Vars.esp.new_chams_weapon_original_model);
//					}
//
//					break;
//				}
//				case 3: //hands & weapon
//				{
//					InsertCheckbox(enemy_glow, XorStr("Enemy Glow ##enemy"), &g_Vars.esp.glow_enemy);
//					ColorPicker(XorStr("##enemyglowcolor"), g_Vars.esp.glow_enemy_color, true, false);
//					InsertCombo(XorStr("Enemy Glow Type"), &g_Vars.esp.glow_type_enemy, glow_types);
//
//					InsertCheckbox(local_glow, XorStr("Local Glow ##local"), &g_Vars.esp.glow_local);
//					ColorPicker(XorStr("##localglowcolor"), g_Vars.esp.glow_local_color, true, false);
//					InsertCombo(XorStr("Local Glow Type"), &g_Vars.esp.glow_type_local , glow_types);
//
//					InsertCheckbox(weapons_glow, XorStr("Weapons Glow ##weapons"), &g_Vars.esp.glow_weapons);
//					ColorPicker(XorStr("##weaponsglowcolor"), g_Vars.esp.glow_weapons_color, true, false);
//					InsertCombo(XorStr("Weapons Glow Type"), &g_Vars.esp.glow_type_weapons, glow_types);
//
//					InsertCheckbox(grenades_glow, XorStr("Grenades Glow ##grenades"), &g_Vars.esp.glow_grenade);
//					ColorPicker(XorStr("##grenadesglowcolor"), g_Vars.esp.glow_grenade_color, true, false);
//					InsertCombo(XorStr("Grenades Glow Type"), &g_Vars.esp.glow_type_grenades, glow_types);
//
//					break;
//				}
//				}
//
//				break;
//			}
//		}
//	}
//	ImGui::EndColumns();
//}
//
//void Misc()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//
//
//	ImGui::NewLine();
//	{
//		switch (miscSubtabs)
//		{
//		case 0:
//		{
//			InsertCheckbox(Bhop, XorStr("Auto Jump"), &g_Vars.misc.autojump);
//			InsertCheckbox(AutoStrafe, XorStr("Auto Strafe"), &g_Vars.misc.autostrafer);
//			InsertCheckbox(WASDStrafe, XorStr("WASD-Strafer"), &g_Vars.misc.autostrafer_wasd);
//			InsertCheckbox(AutoPeek, XorStr("Auto-Peek"), &g_Vars.misc.autopeek);
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(XorStr("##AutoPeekKey"), &g_Vars.misc.autopeek_bind.key, &g_Vars.misc.autopeek_bind.cond, ImVec2{ 40,20 });
//
//			if (g_Vars.misc.autopeek) {
//				InsertCheckbox(AutoPeek, XorStr("Auto-Peek Visualize"), &g_Vars.misc.autopeek_visualise);
//				ImGui::SameLine();
//				ColorPicker(XorStr("auto peek color"), g_Vars.misc.autopeek_color, false, false);
//			}
//
//			std::vector<MultiItem_t> local_extras = {
//				{ XorStr("Zero Pitch on Land"), &g_Vars.esp.zeropitch },
//				{ XorStr("Minion"), &g_Vars.esp.break_bones },
//			};
//
//			InsertMultiCombo(std::string(XorStr("Local Extras")).c_str(), local_extras);
//
//			InsertCheckbox(UnlockInventory, XorStr("Unlock Inventory"), &g_Vars.misc.unlock_inventory);
//			InsertCheckbox(FastStop, XorStr("Fast Stop"), &g_Vars.misc.quickstop);
//			InsertCheckbox(Custom_menu, XorStr("Custom Menu"), &g_Vars.misc.custom_menu); if (g_Vars.misc.custom_menu) { ImGui::SameLine(); ColorPicker(XorStr("##accent"), g_Vars.misc.accent_color, false, false); ImGui::SameLine(); ColorPicker(XorStr("##logo"), g_Vars.misc.logo_color, false, false); }
//
//			const char* models[]{ XorStr("Off"), XorStr("Darth Vader"), XorStr("StormTrooper"), XorStr("Custom") };
//
//			InsertCheckbox(ModelChanger, XorStr("Model Changer"), &g_Vars.misc.model_changer);
//			if (g_Vars.misc.model_changer) {
//				InsertCombo(XorStr("Models"), &g_Vars.misc.models, models);
//
//				if (g_Vars.misc.models == 3) {
//					ImGui::InputText(XorStr("##Custom Model"), &g_Vars.misc.custom_model);
//				}
//			}
//
//			ImGui::NextColumn();
//			ImGui::NewLine();
//
//			InsertCheckbox(AccurateWalk, XorStr("Accurate Walk"), &g_Vars.misc.accurate_walk);
//			InsertCheckbox(SlideWalk, XorStr("Slide Walk"), &g_Vars.misc.slide_walk);
//			InsertCheckbox(InfStamina, XorStr("Infinite Stamina"), &g_Vars.misc.fastduck);
//			InsertCheckbox(BuyBot, XorStr("Buy Bot"), &g_Vars.misc.autobuy_enabled);
//
//			const char* first_weapon_str[]{ XorStr("None"), XorStr("SCAR-20 / G3SG1"), XorStr("SSG-08"), XorStr("AWP") };
//			const char* second_weapon_str[]{ XorStr("None"), XorStr("Dualies"), XorStr("Desert Eagle / R8 Revolver") };
//
//			std::vector<MultiItem_t> other_weapon_conditions = {
//				{ XorStr("Armor"), &g_Vars.misc.autobuy_armor },
//				{ XorStr("Flashbang"), &g_Vars.misc.autobuy_flashbang },
//				{ XorStr("HE Grenade"), &g_Vars.misc.autobuy_hegrenade },
//				{ XorStr("Molotov"), &g_Vars.misc.autobuy_molotovgrenade },
//				{ XorStr("Smoke"), &g_Vars.misc.autobuy_smokegreanade },
//				{ XorStr("Decoy"), &g_Vars.misc.autobuy_decoy },
//				{ XorStr("Taser"), &g_Vars.misc.autobuy_zeus },
//				{ XorStr("Defuse kit"), &g_Vars.misc.autobuy_defusekit },
//			};
//
//			if (g_Vars.misc.autobuy_enabled || GUI::ctx->setup) {
//				InsertCombo(XorStr("Primary Weapon"), &g_Vars.misc.autobuy_first_weapon, first_weapon_str);
//				InsertCombo(XorStr("Secondary Weapon"), &g_Vars.misc.autobuy_second_weapon, second_weapon_str);
//				InsertMultiCombo(std::string(XorStr("Utility")).c_str(), other_weapon_conditions);
//			}
//
//			ImGui::NextColumn();
//			ImGui::NewLine();
//
//			InsertCheckbox(Watermark, XorStr("Watermark"), &g_Vars.misc.watermark);
//			const char* clantag_options[] = { XorStr("Off"), XorStr("Default"), XorStr("Custom") };
//
//			InsertCombo(XorStr("Clan-tag"), &g_Vars.misc.clantag_changer, clantag_options);
//			if (g_Vars.misc.clantag_changer == 2) {
//				ImGui::InputText(XorStr("##Custom Clantag"), &g_Vars.misc.custom_clantag);
//			}
//
//			std::vector<MultiItem_t> notifications = {
//				{ XorStr("Damage dealt"), &g_Vars.esp.event_dmg },
//				{ XorStr("Damage taken"), &g_Vars.esp.event_harm },
//				{ XorStr("Misses"), &g_Vars.esp.event_resolver },
//			};
//
//			InsertMultiCombo(std::string(XorStr("Notifications")).c_str(), notifications);
//
//			InsertCheckbox(ExploitThingLOL, XorStr("Instant bomb defuse"), &g_Vars.misc.balls);
//
//
//#if defined(BETA_MODE) || defined(DEV)
//			InsertCheckbox(resolverflags, XorStr("Resolver Flags"), &g_Vars.misc.resolver_flags);
//			InsertCheckbox(hidebetalogs, XorStr("Hide Beta Logs"), &g_Vars.misc.undercover_log);
//			InsertCheckbox(hideflags, XorStr("Hide Debug Flags"), &g_Vars.misc.undercover_flags);
//
//#endif
//			InsertCheckbox(fakeDuck, XorStr("Fake-Duck"), &g_Vars.misc.fakeduck);
//			if (g_Vars.misc.fakeduck) {
//				ImGui::SameLine();
//				biggestMeme2();
//				ImGui::Hotkey(XorStr("##FDkey"), &g_Vars.misc.fakeduck_bind.key, &g_Vars.misc.fakeduck_bind.cond, ImVec2{ 40,20 });
//		    }
//
//			std::vector<MultiItem_t> walkbot_s = {
//				{ XorStr("Enable"), &g_Vars.misc.walkbot_enable },
//				{ XorStr("Cycle walk"), &g_Vars.misc.walkbot_cyclewalk },
//				{ XorStr("Sound alert"), &g_Vars.misc.walkbot_soundalert },
//				{ XorStr("Developer mode"), &g_Vars.misc.walkbot_developermode },
//			};
//
//			InsertMultiCombo(std::string(XorStr("Walkbot")).c_str(), walkbot_s);
//
//			ImGui::Text(XorStr("Create Point"));
//			ImGui::SameLine();
//			biggestMeme2();
//			ImGui::Hotkey(XorStr("##createpoint"), &g_Vars.misc.walkbot_bind.key, &g_Vars.misc.walkbot_bind.cond, ImVec2{ 40,20 });
//
//			static char addictname[64] = "\0";
//
//			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
//			ImGui::InputText(XorStr("##addictname"), addictname, sizeof(addictname));
//			ImGui::PopStyleVar();
//
//			if (ImGui::Button(XorStr("Save path"), ImVec2(100, 0)))
//				walkbot::Instance().file(addictname, file_overwrite);
//
//			if (ImGui::Button(XorStr("Load path"), ImVec2(100, 0)))
//				walkbot::Instance().file(addictname, file_load);
//
//			if (ImGui::Button(XorStr("Clear path"), ImVec2(100, 0)))
//				walkbot::Instance().clear_dots();
//
//			break;
//		}
//		case 1:
//		{
//			ImGui::NewLine();
//			{
//				static int selected_cfg;
//				static std::vector<std::string> cfg_list;
//				static bool initialise_configs = true;
//				bool reinit = false;
//				if (initialise_configs || (GetTickCount() % 10000) == 0) {
//					cfg_list = ConfigManager::GetConfigs();
//					initialise_configs = false;
//					reinit = true;
//				}
//
//				static std::string config_name;
//				ImGui::Text(XorStr("Name")); ImGui::SameLine(); // my ghetto way of having the name first then the input box
//				ImGui::InputText(XorStr("##name"), &config_name);
//
//				
//				if (!cfg_list.empty()) {
//					for (auto penis : cfg_list)
//						if (ImGui::Selectable(penis.c_str(), ConfigManager::GetConfigID(penis) == selected_cfg))
//							selected_cfg = ConfigManager::GetConfigID(penis);
//				}
//				else
//					ImGui::Text(XorStr("No configs"));
//
//				if (reinit) {
//					if (selected_cfg >= cfg_list.size())
//						selected_cfg = cfg_list.size() - 1;
//
//					if (selected_cfg < 0)
//						selected_cfg = 0;
//				}
//
//				ImGui::NewLine();
//
//				if (!cfg_list.empty()) {
//					if (ImGui::Button(XorStr("Save")))
//					{
//						ImGui::OpenPopup(XorStr("Confirmation_Save"));
//					}
//
//					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Save")))
//					{
//						if (ImGui::Button(XorStr("Are you sure?")))
//						{
//							//LuaConfigSystem::Save();
//							ConfigManager::SaveConfig(cfg_list.at(selected_cfg));
//							ILoggerEvent::Get()->PushEvent(XorStr("Saved config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
//							ImGui::CloseCurrentPopup();
//						}
//						//if (ImGui::Button(XorStr("Cancel"), ImVec2(120, 0)))
//						//{
//						//	ImGui::CloseCurrentPopup();
//						//}
//						ImGui::EndPopup();
//					}
//
//					ImGui::SameLine();
//
//					if (ImGui::Button(XorStr("Load")))
//					{
//						ImGui::OpenPopup(XorStr("Confirmation_Load"));
//					}
//
//					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Load")))
//					{
//						if (ImGui::Button(XorStr("Are you sure?")))
//						{
//							if (selected_cfg <= cfg_list.size() && selected_cfg >= 0) {
//								ConfigManager::ResetConfig();
//
//								//LuaConfigSystem::Load();
//								ConfigManager::LoadConfig(cfg_list.at(selected_cfg));
//								ILoggerEvent::Get()->PushEvent(XorStr("Loaded config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
//								g_Vars.m_global_skin_changer.m_update_skins = true;
//								g_Vars.m_global_skin_changer.m_update_gloves = true;
//								ImGui::CloseCurrentPopup();
//							}
//						}
//						ImGui::EndPopup();
//					}
//
//					ImGui::SameLine();
//					if (ImGui::Button(XorStr("Delete")))
//					{
//						ImGui::OpenPopup(XorStr("Confirmation_Delete"));
//					}
//
//					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Delete")))
//					{
//						if (ImGui::Button(XorStr("Are you sure?")))
//						{
//							ConfigManager::RemoveConfig(cfg_list.at(selected_cfg));
//							cfg_list = ConfigManager::GetConfigs();
//							ILoggerEvent::Get()->PushEvent(XorStr("Deleted config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
//							ImGui::CloseCurrentPopup();
//						}
//						ImGui::EndPopup();
//					}
//
//
//					ImGui::SameLine();
//				}
//
//				if (ImGui::Button(XorStr("Create"))) {
//					if (config_name.empty())
//						return;
//
//					ConfigManager::CreateConfig(config_name);
//					cfg_list = ConfigManager::GetConfigs();
//				}
//
//				if (ImGui::Button(XorStr("Reset"))) {
//					ImGui::OpenPopup(XorStr("Confirmation_Reset"));
//				}
//
//				if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Reset")))
//				{
//					if (ImGui::Button(XorStr("Are you sure?")))
//					{
//						ConfigManager::ResetConfig();
//						ILoggerEvent::Get()->PushEvent(XorStr("Reset config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
//						ImGui::CloseCurrentPopup();
//					}
//					ImGui::EndPopup();
//				}
//				ImGui::SameLine();
//
//				if (ImGui::Button(XorStr("Open config folder")))
//				{
//					ConfigManager::OpenConfigFolder();
//				}
//			}
//
//			break;
//		}
//		}
//	}
//	ImGui::EndColumns();
//}
//
//void Skins()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//
//	ImGui::NewLine();
//	{
//		const char* knife_models[]{ XorStr("Bayonet"), XorStr("Bowie"), XorStr("Butterfly"), XorStr("Falchion"), XorStr("Flip"), XorStr("Gut"), XorStr("Tactical"), XorStr("Karambit"), XorStr("M9 Bayonet"), XorStr("Shadow Daggers") };
//		const char* glove_models[]{ XorStr("Default"), XorStr("Bloodhound"), XorStr("Sport"), XorStr("Driver"), XorStr("Hand Wraps"), XorStr("Motorcycle"), XorStr("Specialist") };
//		const char* bloodhound_skins[]{ XorStr("Charred"), XorStr("Snakebite"), XorStr("Bronzed"), XorStr("Guerilla") };
//		const char* sport_skins[]{ XorStr("Hedge Maze"), XorStr("Pandoras Box"), XorStr("Superconductor"), XorStr("Arid"), XorStr("Vice"), XorStr("Omega"), XorStr("Amphibious"), XorStr("Bronze Morph") };
//		const char* driver_skins[]{ XorStr("Lunar Weave"), XorStr("Convoy"), XorStr("Crimson Weave"), XorStr("Diamondback"), XorStr("Overtake"), XorStr("Racing Green"), XorStr("King Snake"), XorStr("Imperial Plaid") };
//		const char* handwarps_skins[]{ XorStr("Leather"), XorStr("Spruce DDPAT"), XorStr("Slaughter"), XorStr("Badlands"), XorStr("Cobalt Skulls"), XorStr("Overprint"), XorStr("Duct Tape"), XorStr("Arboreal") };
//		const char* motorcycle_skins[]{ XorStr("Eclipse"), XorStr("Spearmint"), XorStr("Boom!"), XorStr("Cool Mint"), XorStr("Turtle"), XorStr("Transport"), XorStr("Polygon"), XorStr("POW!") };
//		const char* specialist_skins[]{ XorStr("Forest DDPAT"), XorStr("Crimson Kimono"), XorStr("Emerald Web"), XorStr("Foundation"), XorStr("Crimson Web"), XorStr("Buckshot"), XorStr("Fade"), XorStr("Mogul") };
//
//		switch (skinsSubtabs)
//		{
//		case 0:
//		{
//			InsertCheckbox(enableskins, XorStr("Enable"), &g_Vars.misc.enable_skins);
//			if (g_Vars.misc.enable_skins) {
//				InsertCombo(XorStr("Knife Model"), &g_Vars.misc.knife_model, knife_models);
//				ImGui::InputInt(XorStr("Knife Skin"), &g_Vars.misc.knife_skin);
//
//				InsertCheckbox(enablegloves, XorStr("Enable Gloves"), &g_Vars.misc.enable_gloves);
//
//				if (g_Vars.misc.enable_gloves) {
//					InsertCombo(XorStr("Glove Model"), &g_Vars.misc.gloves_model, glove_models);
//
//					switch (g_Vars.misc.gloves_model) {
//					case 1:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin , bloodhound_skins);
//						break;
//					case 2:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, sport_skins);
//						break;
//					case 3:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, driver_skins);
//						break;
//					case 4:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, handwarps_skins);
//						break;
//					case 5:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, motorcycle_skins);
//						break;
//					case 6:
//						InsertCombo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, specialist_skins);
//						break;
//					}
//
//				}
//
//
//				if (ImGui::Button(XorStr("Apply Skins")))
//				{
//					g_Vars.m_global_skin_changer.m_update_skins = true;
//				}
//			}
//			break;
//		}
//		case 1:
//		{
//			ImGui::NewLine();
//			{
//				ImGui::InputInt(XorStr("Usp Skin"), &g_Vars.misc.usp_skin);
//				ImGui::InputInt(XorStr("P2000 Skin"), &g_Vars.misc.p2k_skin);
//				ImGui::InputInt(XorStr("Glock Skin"), &g_Vars.misc.glock_skin);
//				ImGui::InputInt(XorStr("P250 Skin"), &g_Vars.misc.p250_skin);
//				ImGui::InputInt(XorStr("Five7 Skin"), &g_Vars.misc.fiveseven_skin);
//				ImGui::InputInt(XorStr("Tec9 Skin"), &g_Vars.misc.tec9_skin);
//				ImGui::InputInt(XorStr("CZ75A Skin"), &g_Vars.misc.cz75a_skin);
//				ImGui::InputInt(XorStr("Elite Skin"), &g_Vars.misc.elite_skin);
//				ImGui::InputInt(XorStr("Deagle Skin"), &g_Vars.misc.deagle_skin);
//				ImGui::InputInt(XorStr("R8 Skin"), &g_Vars.misc.revolver_skin);
//				ImGui::InputInt(XorStr("Famas Skin"), &g_Vars.misc.famas_skin);
//				ImGui::InputInt(XorStr("Galil Skin"), &g_Vars.misc.galilar_skin);
//				ImGui::InputInt(XorStr("M4a1 Skin"), &g_Vars.misc.m4a1_skin);
//				ImGui::InputInt(XorStr("M4a1s Skin"), &g_Vars.misc.m4a1s_skin);
//				ImGui::InputInt(XorStr("Ak47 Skin"), &g_Vars.misc.ak47_skin);
//				ImGui::InputInt(XorStr("Sg556 Skin"), &g_Vars.misc.sg556_skin);
//				ImGui::InputInt(XorStr("Aug Skin"), &g_Vars.misc.aug_skin);
//				ImGui::NextColumn();
//				ImGui::NewLine();
//				ImGui::InputInt(XorStr("Ssg08 Skin"), &g_Vars.misc.ssg08_skin);
//				ImGui::InputInt(XorStr("Awp Skin"), &g_Vars.misc.awp_skin);
//				ImGui::InputInt(XorStr("Scar20 Skin"), &g_Vars.misc.scar20_skin);
//				ImGui::InputInt(XorStr("G3sg1 Skin"), &g_Vars.misc.g3sg1_skin);
//				ImGui::InputInt(XorStr("Sawoff Skin"), &g_Vars.misc.sawedoff_skin);
//				ImGui::InputInt(XorStr("M249 Skin"), &g_Vars.misc.m249_skin);
//				ImGui::InputInt(XorStr("Negev Skin"), &g_Vars.misc.negev_skin);
//				ImGui::InputInt(XorStr("Mag7 Skin"), &g_Vars.misc.mag7_skin);
//				ImGui::InputInt(XorStr("Xm Skin"), &g_Vars.misc.xm1014_skin);
//				ImGui::InputInt(XorStr("Nova Skin"), &g_Vars.misc.nova_skin);
//				ImGui::InputInt(XorStr("Bizon Skin"), &g_Vars.misc.bizon_skin);
//				ImGui::InputInt(XorStr("Mp7 Skin"), &g_Vars.misc.mp7_skin);
//				ImGui::InputInt(XorStr("Mp9 Skin"), &g_Vars.misc.mp9_skin);
//				ImGui::InputInt(XorStr("Mac10 Skin"), &g_Vars.misc.mac10_skin);
//				ImGui::InputInt(XorStr("P90 Skin"), &g_Vars.misc.p90_skin);
//				ImGui::InputInt(XorStr("Ump45 Skin"), &g_Vars.misc.ump45_skin);
//			}
//
//
//			break;
//		}
//
//		//std::string base_string = XorStr("skins_");
//
//		//std::string paintkit_string = base_string.append(XorStr("_kit"));
//		//static ImGuiTextFilter filter;
//
//		//if (!skin_kits.empty()) {
//		//	static ImGuiTextFilter filter;
//		//	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 15);
//		//	filter.Draw(XorStr("##searchbar"), 400.f);
//		//	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15);
//
//		//	for (int i = 0; i < skin_kits.size(); i++) {
//		//		auto paintkit = skin_kits.at(i);
//		//		if (filter.PassFilter(paintkit.name.c_str())) {
//		//			if (paintkit == XorStr("-"))
//		//				paintkit = XorStr("Default");
//
//		//			std::string label = paintkit + XorStr("##") + std::to_string(i);
//
//		//			if (ImGui::Selectable(label.c_str(), g_cfg[paintkit_string].get<int>() == i))
//		//				g_cfg[paintkit_string].set<int>(i);
//
//		//		}
//		//	}
//		//}
//		}
//
//
//
//		
//	}
//	ImGui::EndColumns();
//}
//
//void Scripts()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//
//	ImGui::NewLine();
//	{
//		switch (scriptsSubtabs)
//		{
//		case 0:
//		{
//			ImGui::Text(XorStr("Scripts"));
//			ImGui::NewLine();
//
//			{
//				for (auto s : g_lua.scripts)
//				{
//					if (ImGui::Selectable(s.c_str(), g_lua.loaded.at(g_lua.get_script_id(s)), NULL, ImVec2(0, 0))) {
//						auto scriptId = g_lua.get_script_id(s);
//						if (g_lua.loaded.at(scriptId)) g_lua.unload_script(scriptId); else g_lua.load_script(scriptId);
//					}
//				}
//
//				ImGui::NewLine();
//
//				InsertCheckbox(AllowHttpRequests, XorStr("Allow HTTP Requests"), &g_Vars.misc.lua_allow_http_requests);
//
//				if (ImGui::Button(XorStr("Refresh scripts"), ImVec2(100, 0))) g_lua.refresh_scripts();
//
//				if (ImGui::Button(XorStr("Reload active"), ImVec2(100, 0))) g_lua.reload_all_scripts();
//
//				if (ImGui::Button(XorStr("Unload all"), ImVec2(100, 0))) g_lua.unload_all_scripts();
//			}
//
//			ImGui::NextColumn(); ImGui::NewLine();
//
//
//			for (auto& current : g_lua.scripts)
//			{
//				auto& items = g_lua.items.at(g_lua.get_script_id(current));
//
//				for (auto& item : items)
//				{
//					std::string item_name;
//
//					auto first_point = false;
//					auto item_str = false;
//
//					for (auto& c : item.first)
//					{
//						if (c == '.')
//						{
//							if (first_point)
//							{
//								item_str = true;
//								continue;
//							}
//							else
//								first_point = true;
//						}
//
//						if (item_str)
//							item_name.push_back(c);
//					}
//
//					switch (item.second.type)
//					{
//					case NEXT_LINE:
//						break;
//					case CHECK_BOX:
//						ImGui::Checkbox(item_name.c_str(), &LuaConfigSystem::C_BOOL[item.second.key]);
//						break;
//					case SLIDER_INT:
//						InsertSliderInt(item_name.c_str(), &LuaConfigSystem::C_INT[item.second.key], item.second.slider_int_min, item.second.slider_int_max, item.second.format.c_str());
//						break;
//					case SLIDER_FLOAT:
//						InsertSliderFloat(item_name.c_str(), &LuaConfigSystem::C_FLOAT[item.second.key], item.second.slider_float_min, item.second.slider_float_max, item.second.format.c_str());
//						break;
//					case COLOR_PICKER:
//						ColorPicker_w_name(item_name.c_str(), LuaConfigSystem::C_COLOR[item.second.key], true, false);
//						break;
//					case TEXT:
//						ImGui::Text(item_name.c_str());
//						break;
//					}
//				}
//			}
//			break;
//		}
//		}
//	}
//	ImGui::EndColumns();
//}
//
//void PlayerList()
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	float group_w = ImGui::GetCurrentWindow()->Size.x - style->WindowPadding.x * 2;
//	ImGui::Columns(3, nullptr, false);
//	ImGui::SetColumnOffset(1, group_w / 3.0f);
//	ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
//	ImGui::SetColumnOffset(3, group_w);
//
//	ImGui::NewLine();
//	{
//		switch (PlayerListSubTabs)
//		{
//		case 0:
//		{
//			static std::vector <Player_list_data> players;
//
//			if (!g_Vars.globals.player_list.refreshing)
//			{
//				players.clear();
//
//				for (auto player : g_Vars.globals.player_list.players)
//					players.emplace_back(player);
//			}
//
//			static auto current_player = 0;
//
//			static char uname[128];
//			ImGui::InputText(XorStr("search"), uname, 128);
//
//			ImGui::ListBoxHeader(XorStr("##players"), ImVec2(0, ImGui::GetWindowSize().y - 80));
//
//			if (!players.empty())
//			{
//				std::vector <std::string> player_names;
//
//				for (auto player : players) {
//					auto search = std::string(uname);
//					auto name = std::string(player.name);
//
//					std::transform(search.begin(), search.end(), search.begin(), ::tolower);
//					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
//
//					if (search != "" && name.find(search) == std::string::npos)
//						continue;
//
//					player_names.emplace_back(player.name);
//				}
//
//				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
//				ImGui::ListBoxConfigArray(XorStr("##PLAYERLIST"), &current_player, player_names, 14);
//				ImGui::PopStyleVar();
//			}
//
//			ImGui::ListBoxFooter();
//
//			ImGui::NextColumn(); ImGui::NewLine();
//
//			if (!players.empty())
//			{
//				if (current_player >= players.size())
//					current_player = players.size() - 1;
//
//
//				ImGui::Checkbox(XorStr("Whitelist"), &g_Vars.globals.player_list.white_list[players.at(current_player).i]);
//				ImGui::Checkbox(XorStr("Override Pitch"), &g_Vars.globals.player_list.override_pitch[players.at(current_player).i]);
//				if (g_Vars.globals.player_list.override_pitch[players.at(current_player).i]) {
//					InsertSliderInt(XorStr("Pitch Angle"), &g_Vars.globals.player_list.override_pitch_slider[players.at(current_player).i], -89, 89, XorStr("%d"));
//				}
//
//			}
//	
//
//			break;
//		}
//		}
//	}
//	ImGui::EndColumns();
//}
//
//bool IMGUIMenu::Initialize(IDirect3DDevice9* pDevice)
//{
//	if (!Initialized)
//	{
//		ImGui::CreateContext();
//		CreateStyle();
//		IDirect3DSwapChain9* pChain = nullptr;
//		D3DPRESENT_PARAMETERS pp = {};
//		D3DDEVICE_CREATION_PARAMETERS param = {};
//		pDevice->GetCreationParameters(&param);
//		pDevice->GetSwapChain(0, &pChain);
//
//		if (pChain) pChain->GetPresentParameters(&pp);
//
//		ImGui_ImplWin32_Init(param.hFocusWindow);
//		ImGui_ImplDX9_Init(pDevice);
//		ImGui_ImplDX9_CreateDeviceObjects();
//
//
//		if(!logo_nuts)
//				D3DXCreateTextureFromFileInMemoryEx(pDevice, logo_png, sizeof(logo_png), 1000, 1000, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &logo_nuts);
//
//
//
//		Initialized = true;
//	}
//
//	return Initialized;
//}
//
//void IMGUIMenu::Shutdown()
//{
//    ImGui_ImplDX9_Shutdown();
//	ImGui_ImplWin32_Shutdown();
//	ImGui::DestroyContext();
//}
//
//void IMGUIMenu::OnDeviceLost()
//{
//    ImGui_ImplDX9_InvalidateDeviceObjects();
//}
//
//void IMGUIMenu::OnDeviceReset()
//{
//    ImGui_ImplDX9_CreateDeviceObjects();
//}
//
//auto windowFlags = (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoSavedSettings);
//ImFont* gravity, *gravityBold, *StarWars, *watermark;
//
//#define MENU_WIDTH 650.f
//#define MENU_HEIGHT 600.f
//
//bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
//	ImGuiWindow* window = ImGui::GetCurrentWindow();
//	if (window->SkipItems)
//		return false;
//
//	ImGuiContext& g = *GImGui;
//	const ImGuiStyle& style = g.Style;
//	const ImGuiID id = window->GetID(label);
//
//	ImVec2 pos = ImVec2(Render::GetScreenSize().x * 0.5f - 55, Render::GetScreenSize().y * 0.5f + 70);
//	ImVec2 size = size_arg;
//	size.x -= style.FramePadding.x * 2;
//
//	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
//	ImGui::ItemSize(bb, style.FramePadding.y);
//	if (!ImGui::ItemAdd(bb, id))
//		return false;
//
//	// Render
//	const float circleStart = size.x * 0.7f;
//	window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
//	window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);}
//
//
//void IMGUIMenu::Loading()
//{
//
//	Opened = true;
//
//	static bool load = true;
//	static float flTime;
//	static float flTime_bar;
//	static bool init = true;
//	static bool should_fade_out = false;
//
//	if (init) {
//		flTime = ImGui::GetTime();
//		flTime_bar = ImGui::GetTime();
//		init = false;
//	}
//
//	float flSubtractedTime = ImGui::GetTime() - flTime;
//
//	if (flSubtractedTime > 5.5f && load) { // 7 seconds i think is the optimal time
//		should_fade_out = true;
//	}
//
//	static float bar_thing = 0.0f;
//
//	float flSubtractedTime_bar = ImGui::GetTime() - flTime_bar;
//
//	if (flSubtractedTime_bar > 1.f && !(bar_thing > 1.0f)) { // every second increase bar
//		flTime_bar = ImGui::GetTime();
//		bar_thing += 0.1f;
//	}
//
//	static float pulse_alpha = 0.f;
//	static bool change_alpha = false;
//
//	if (pulse_alpha <= 0.f)
//		change_alpha = true;
//	//else if (pulse_alpha >= 1.f) // pulse effect (puke)
//	//	change_alpha = false;
//
//	pulse_alpha = change_alpha ? pulse_alpha + 0.02f : pulse_alpha - 0.02f;
//
//	if (should_fade_out) {
//		change_alpha = false;
//
//		if (pulse_alpha < 0.0f) {
//			load = false;
//			Loaded = true;
//		}
//	}
//
//	if (load)
//	{
//		static bool retard = true;
//
//		ImGui::SetNextWindowPos(ImVec2(Render::GetScreenSize().x * 0.5f, Render::GetScreenSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
//
//		ImGui::SetNextWindowSize(ImVec2(Render::GetScreenSize().x, Render::GetScreenSize().y));
//
//		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.4f)); // Set window background to black
//
//		ImGui::Begin(XorStr("Loading"), &retard, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
//
//		ImGui::PopStyleColor();
//
//		ImGui::PushFont(watermark);
//
//		ImGui::SameLine(Render::GetScreenSize().x * 0.5f - 55, -1.0f, Render::GetScreenSize().y * 0.5f + 40);
//		ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, pulse_alpha), XorStr("Initializing Vader.tech"));
//
//		ImGui::PopFont();
//
//		ImGui::AddCircleImageFilled(logo_nuts, ImVec2(Render::GetScreenSize().x * 0.5f + 15, Render::GetScreenSize().y * 0.5f - 40), 80.f, ImColor(1.f, 1.f, 1.f, pulse_alpha), 360);
//
//		const ImU32 col = ImGui::GetColorU32(ImVec4(1.f, 0.f, 0.f, pulse_alpha));
//		const ImU32 bg = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, pulse_alpha));
//
//		BufferingBar(XorStr("##loading_bar"), bar_thing, ImVec2(210, 5), bg, col);
//
//		ImGui::End();
//
//		return;
//	}
//
//}
//
//enum AnimationTypes { STATIC, DYNAMIC, INTERP };
//
//std::string pChar(std::string first_, std::string second_) {
//	return (std::stringstream{} << first_ << second_).str();
//}
//
//float Animate(const char* label, const char* second_label, bool if_, float Maximal_, float Speed_, int type) {
//
//	auto ID = ImGui::GetID(pChar(label, second_label).c_str());
//
//	static std::map<ImGuiID, float> pValue;
//
//	auto this_e = pValue.find(ID);
//
//	if (this_e == pValue.end()) {
//		pValue.insert({ ID, 0.f });
//		this_e = pValue.find(ID);
//	}
//
//	switch (type) {
//
//	case DYNAMIC: {
//		if (if_) //do
//			this_e->second += abs(Maximal_ - this_e->second) / Speed_;
//		else
//			this_e->second -= (0 + this_e->second) / Speed_;
//	}
//				break;
//
//	case INTERP: {
//		this_e->second += (Maximal_ - this_e->second) / Speed_;
//	}
//			   break;
//
//	case STATIC: {
//		if (if_) //do
//			this_e->second += Speed_;
//		else
//			this_e->second -= Speed_;
//	}
//			   break;
//	}
//
//	if (type != INTERP) {
//		if (this_e->second < 0.f)
//			this_e->second = 0.f;
//		else if (this_e->second > Maximal_)
//			this_e->second = Maximal_;
//	}
//
//	return this_e->second;
//
//}
//
//void DrawShapeAnimated(const char* label, ImVec2 min, ImVec2 max, ImVec2 pos, ImColor color) {
//
//	int hovered_delta = Animate(label, XorStr("hovered_rect"), ImGui::IsMouseHoveringRect(min, max, false), 255.f, 15.f, STATIC);
//
//	ImGui::SetCursorPos(pos - ImGui::GetWindowPos());
//
//	auto clicked = ImGui::GetIO().MouseClicked[0] && ImGui::IsMouseHoveringRect(min, max, false);
//	auto ID = ImGui::GetID(pChar(label, XorStr("b_animation")).c_str());
//
//	static std::map<ImGuiID, bool> pValue;
//	auto ItPLibrary = pValue.find(ID);
//	if (ItPLibrary == pValue.end()) {
//		pValue.insert({ ID, false });
//		ItPLibrary = pValue.find(ID);
//	}
//
//	float radius = Animate(label, XorStr("circle_radius"), ItPLibrary->second, 100.f, 10.f, DYNAMIC);
//
//	static std::map<ImGuiID, int> pValue2;
//
//	static POINT MousePoint;
//	GetCursorPos(&MousePoint);
//
//	static std::map<ImGuiID, ImVec2> CirclePosMap;
//	auto CirclePos = CirclePosMap.find(ID);
//	if (CirclePos == CirclePosMap.end()) {
//		CirclePosMap.insert({ ID, ImVec2(MousePoint.x, MousePoint.y) });
//		CirclePos = CirclePosMap.find(ID);
//	}
//
//	if (clicked) {
//		CirclePos->second = ImVec2(MousePoint.x, MousePoint.y);
//		ItPLibrary->second = true;
//	}
//
//	if (ItPLibrary->second) {
//
//		if (radius >= 95.f) {
//			ItPLibrary->second = false;
//			radius = 0.f;
//		}
//	}
//	ImRect bb = ImRect(min, max);
//	auto size = bb.GetSize();
//	//calc circle start radius and scale radius
//	float StartRad = sqrt((size.x + size.y) / 2);
//	//scale :::
//	float ScaleRad = ((StartRad + radius) + (sqrt(size.x * size.y) / ((size.x - size.y) / StartRad))) * 0.5;
//
//	ImGui::BeginChild(pChar(XorStr("drawlist_zone"), label).c_str(), size, false, ImGuiWindowFlags_NoBackground); {
//		if (ItPLibrary->second)
//			ImGui::GetWindowDrawList()->AddCircleFilled(CirclePos->second, StartRad + ScaleRad, ImColor(int(255.f * color.Value.x), int(255.f * color.Value.y), int(255.f * color.Value.z), 95 - (int)(radius)), (StartRad + ScaleRad) * 15.f);
//	}
//	ImGui::EndChild();
//}
//void create_spectators(const char* name, std::vector <std::string> vec) {
//
//	ImGui::SetNextWindowSize(ImVec2(170, 30));
//	if (!ImGui::IsMouseDragging()) {
//		ImGui::SetNextWindowPos(ImVec2(g_Vars.esp.spec_window_x, g_Vars.esp.spec_window_y), ImGuiCond_Always);
//	}
//	//draw
//
//	//flags - 
//	static const WORD dw_window_flags{
//		ImGuiWindowFlags_NoBackground
//		| ImGuiWindowFlags_NoTitleBar
//		| ImGuiWindowFlags_NoCollapse
//		| ImGuiWindowFlags_NoResize
//		| ImGuiWindowFlags_NoScrollbar
//		| ImGuiWindowFlags_NoScrollWithMouse
//		| ImGuiWindowFlags_NoSavedSettings
//	};
//
//	if (ImGui::Begin(name, nullptr, dw_window_flags))
//	{
//		//get vars :::
//		ImColor theme;
//		ImColor theme_zero;
//		ImColor circle_color;
//
//		if (!g_Vars.misc.custom_menu) {
//			theme = ImColor(255, 0, 0, 255);
//			theme_zero = ImColor(255, 0, 0, 0);
//			circle_color = ImColor(255, 0, 0, 255);
//		}
//		else {
//			theme = (ImColor)g_Vars.misc.accent_color;
//			theme_zero = ImColor(g_Vars.misc.accent_color.r, g_Vars.misc.accent_color.g, g_Vars.misc.accent_color.b, 0.f);
//			circle_color = (ImColor)g_Vars.misc.accent_color;
//		}
//
//		ImColor black = ImColor(0, 0, 0, 210);
//		ImColor black_zero = ImColor(0, 0, 0, 0);
//		ImColor black_half = ImColor(0, 0, 0, 60);
//		ImColor theme_background = ImColor(41, 32, 59, 210);
//
//		const auto draw = ImGui::GetBackgroundDrawList();
//		const auto p = ImGui::GetWindowPos();
//		const auto s = ImGui::GetWindowSize();
//
//		if (g_Vars.esp.spec_window_x != p.x) {
//			g_Vars.esp.spec_window_x = p.x;
//		}
//		
//		if (g_Vars.esp.spec_window_y != p.y) {
//			g_Vars.esp.spec_window_y = p.y;
//		}
//
//		//background
//		draw->AddRectFilledMultiColor(p + ImVec2(10, 0), p + ImVec2(s.x / 2, s.y), theme_background, theme_background, theme_background, theme_background);
//		draw->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, s.y) - ImVec2(10, 0), theme_background, theme_background, theme_background, theme_background);
//
//		//up
//		draw->AddRectFilledMultiColor(p + ImVec2(10, 0), p + ImVec2(s.x / 2, 2), theme, theme, theme, theme);
//		draw->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 2) - ImVec2(10, 0), theme, theme, theme, theme);
//
//		auto spec_sz = ImGui::CalcTextSize(name);
//		draw->AddText(p + ImVec2(s.x / 2 - spec_sz.x / 2, s.y / 2 - spec_sz.y / 2), ImColor(255, 255, 255), name);
//
//		ImVec2 shape_pos = p + ImVec2(10, 0);
//		ImVec2 shape_sz = s - ImVec2(20, -2);
//
//		//DrawShapeAnimated(name, shape_pos, shape_pos + shape_sz, shape_pos, theme);
//
//		draw->AddRectFilled(p + ImVec2(10, s.y), p + ImVec2(s.x - 10, 4 + s.y + 22 * vec.size()), ImColor(0, 0, 0, 100), 0, 15);
//
//		for (auto i = 0; i < vec.size(); i++) {
//			auto first_circle_pos = ImVec2(p.x + 20, p.y + s.y + 15 + 20 * i);
//			//draw->AddCircleFilled(first_circle_pos, 2.3f, circle_color, 5.f * 15.f);
//			//draw->AddCircleFilled(first_circle_pos, 2.f, circle_color, 5.f * 15.f);
//			draw->AddText(NULL, 12.f, first_circle_pos + ImVec2(0, -7), ImColor(255, 255, 255, 255), vec[i].c_str());
//		}
//
//	}
//	ImGui::End();
//
//
//}
//void create_keybinds(const char* name, std::vector <std::string> vec) {
//
//	ImGui::SetNextWindowSize(ImVec2(170, 10));
//	if (!ImGui::IsMouseDragging()) {
//		ImGui::SetNextWindowPos(ImVec2(g_Vars.esp.keybind_window_x, g_Vars.esp.keybind_window_y), ImGuiCond_Always);
//	}
//	//draw
//
//	//flags - 
//	static const WORD dw_window_flags{
//		ImGuiWindowFlags_NoBackground
//		| ImGuiWindowFlags_NoTitleBar
//		| ImGuiWindowFlags_NoCollapse
//		| ImGuiWindowFlags_NoResize
//		| ImGuiWindowFlags_NoScrollbar
//		| ImGuiWindowFlags_NoScrollWithMouse
//		| ImGuiWindowFlags_NoSavedSettings
//	};
//
//	if (ImGui::Begin(name, nullptr, dw_window_flags))
//	{
//		//get vars :::
//		ImColor theme;
//		ImColor theme_zero;
//		ImColor circle_color;
//
//		if (!g_Vars.misc.custom_menu) {
//			theme = ImColor(255, 0, 0, 255);
//			theme_zero = ImColor(255, 0, 0, 0);
//			circle_color = ImColor(255, 0, 0, 255);
//		}
//		else {
//			theme = (ImColor)g_Vars.misc.accent_color;
//			theme_zero = ImColor(g_Vars.misc.accent_color.r, g_Vars.misc.accent_color.g, g_Vars.misc.accent_color.b, 0.f);
//			circle_color = (ImColor)g_Vars.misc.accent_color;
//		}
//
//		ImColor black = ImColor(0, 0, 0, 210);
//		ImColor black_zero = ImColor(0, 0, 0, 0);
//		ImColor black_half = ImColor(0, 0, 0, 60);
//		ImColor theme_background = ImColor(41, 32, 59, 210);
//
//		const auto draw = ImGui::GetBackgroundDrawList();
//		const auto p = ImGui::GetWindowPos();
//		const auto s = ImGui::GetWindowSize();
//
//		if (g_Vars.esp.keybind_window_x != p.x) {
//			g_Vars.esp.keybind_window_x = p.x;
//		}
//
//		if (g_Vars.esp.keybind_window_y != p.y) {
//			g_Vars.esp.keybind_window_y = p.y;
//		}
//
//		//background
//		draw->AddRectFilledMultiColor(p + ImVec2(10, 0), p + ImVec2(s.x / 2, s.y), theme_background, theme_background, theme_background, theme_background);
//		draw->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, s.y) - ImVec2(10, 0), theme_background, theme_background, theme_background, theme_background);
//
//		//up
//		draw->AddRectFilledMultiColor(p + ImVec2(10, 0), p + ImVec2(s.x / 2, 2), theme, theme, theme, theme);
//		draw->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 2) - ImVec2(10, 0), theme, theme, theme, theme);
//
//		auto keybind_sz = ImGui::CalcTextSize(name);
//		draw->AddText(p + ImVec2(s.x / 2 - keybind_sz.x / 2, s.y / 2 - keybind_sz.y / 2), ImColor(255, 255, 255), name);
//
//		ImVec2 shape_pos = p + ImVec2(10, 0);
//		ImVec2 shape_sz = s - ImVec2(20, -2);
//
//		//DrawShapeAnimated(name, shape_pos, shape_pos + shape_sz, shape_pos, theme);
//
//		draw->AddRectFilled(p + ImVec2(10, s.y), p + ImVec2(s.x - 10, 4 + s.y + 22 * vec.size()), ImColor(0, 0, 0, 100), 0, 15);
//
//		for (auto i = 0; i < vec.size(); i++) {
//			auto first_circle_pos = ImVec2(p.x + 20, p.y + s.y + 15 + 20 * i);
//			//draw->AddCircleFilled(first_circle_pos, 2.3f, circle_color, 5.f * 15.f);
//			//draw->AddCircleFilled(first_circle_pos, 2.f, circle_color, 5.f * 15.f);
//			draw->AddText(NULL, 12.f, first_circle_pos + ImVec2(0, -7), ImColor(255, 255, 255, 255), vec[i].c_str());
//		}
//
//	}
//	ImGui::End();
//
//
//}
//void IMGUIMenu::Keybinds_Spectators() {
//
//	if (g_Vars.esp.keybind_window_enabled) {
//		std::vector <std::string> binds{ };
//
//		{ // SORRY FOR THE MESS! HAD NOWHERE TO PUT THIS JOHN!
//			if (g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled) {
//				binds.push_back(XorStr("Doubletap"));
//			}
//			if (g_Vars.rage.enabled) {
//				if (g_Vars.rage.key_dmg_override.enabled) {
//					binds.push_back(XorStr("Min dmg override"));
//				}
//				if (g_Vars.rage.prefer_body.enabled) {
//					binds.push_back(XorStr("Force body-aim"));
//				}
//				if (g_Vars.rage.override_key.enabled) {
//					binds.push_back(XorStr("Hitscan override"));
//				}
//				if (g_Vars.rage.override_reoslver.enabled) {
//					binds.push_back(XorStr("Resolver override"));
//				}
//			}
//			if (g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled) {
//				binds.push_back(XorStr("Fake-Duck"));
//			}
//			if (g_Vars.misc.move_exploit && g_Vars.misc.move_exploit_key.enabled && g_Vars.antiaim.enabled) {
//				binds.push_back(XorStr("Move Exploit"));
//			}
//			if (g_Vars.misc.autopeek && g_Vars.misc.autopeek_bind.enabled) {
//				binds.push_back(XorStr("Auto-Peek"));
//			}
//			if (g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled) {
//				if (!(g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
//					binds.push_back(XorStr("Fake-walk"));
//				}
//				else
//					binds.push_back(XorStr("Slow-walk"));
//			}
//			if (g_Vars.misc.extended_backtrack && g_Vars.misc.extended_backtrack_key.enabled) {
//				binds.push_back(XorStr("Ping-spike"));
//			}
//			if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
//				binds.push_back(XorStr("Jedi Mind-Trick"));
//			}
//		}
//
//		if (!binds.empty() || Opened) {
//			create_keybinds(XorStr("Keybinds"), binds);
//			g_Vars.globals.m_bKeyBindOpen = true;
//		}
//		else
//			g_Vars.globals.m_bKeyBindOpen = false;
//	}
//
//	if (g_Vars.esp.spec_window_enabled) {
//
//		std::vector< std::string > spectators{ };
//		C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();
//
//
//		if (Interfaces::m_pEngine->IsInGame() && pLocal) {
//			const auto local_observer = pLocal->m_hObserverTarget();
//			for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
//				C_CSPlayer* player = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);
//				if (!player)
//					continue;
//
//				if (!player->IsDead())
//					continue;
//
//				if (player->IsDormant())
//					continue;
//
//				if (player->EntIndex() == pLocal->EntIndex())
//					continue;
//
//				player_info_t info;
//				if (!Interfaces::m_pEngine->GetPlayerInfo(i, &info))
//					continue;
//
//				if (pLocal->IsDead()) {
//					auto observer = player->m_hObserverTarget();
//					if (local_observer.IsValid() && observer.IsValid()) {
//						const auto spec = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntityFromHandle(local_observer);
//						auto target = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntityFromHandle(observer));
//
//						if (target == spec && spec) {
//							spectators.push_back(std::string(info.szName).substr(0, 17));
//						}
//					}
//				}
//				else {
//					if (player->m_hObserverTarget() != pLocal)
//						continue;
//
//					spectators.push_back(std::string(info.szName).substr(0, 17));
//				}
//			}
//		}
//
//		if (!spectators.empty() || Opened) {
//			create_spectators(XorStr("Spectators"), spectators);
//			g_Vars.globals.m_bSpecListOpen = true;
//		}
//		else
//			g_Vars.globals.m_bSpecListOpen = false;
//	}
//
//}
//
//static bool m_bResetWatermarkPos;
//
//bool draw_water_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
//{
//	ImGuiStyle* style = &ImGui::GetStyle();
//	bool pressed = false;
//	if (ImGui::Button(label, ImVec2(12, 12)))
//	{
//		ImGui::OpenPopup(XorStr("additives_nigger"));
//	}
//
//	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
//	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
//	ImGui::SetNextWindowSize(ImVec2(175, 85), ImGuiCond_Always);
//	if (ImGui::BeginPopupContextItem(XorStr("additives_nigger")))
//	{
//		std::vector<MultiItem_t> additives = {
//			{ XorStr("Name"), &g_Vars.misc.watermarkadditives_WATERMARK_NAME },
//			{ XorStr("Server"), &g_Vars.misc.watermarkadditives_WATERMARK_SERVER },
//			{ XorStr("Ping"), &g_Vars.misc.watermarkadditives_WATERMARK_PING },
//			{ XorStr("Time"), &g_Vars.misc.watermarkadditives_WATERMARK_TIME },
//			{ XorStr("FPS"), &g_Vars.misc.watermarkadditives_WATERMARK_FPS },
//		};
//
//		InsertMultiCombo(std::string(XorStr("Watermark additives")).c_str(), additives);
//
//		InsertCheckbox(LockDefaultPos, XorStr("Lock Default Position"), &g_Vars.misc.watermark_lockdefaultposition);
//
//		if (ImGui::Button(XorStr("Reset Position"), ImVec2(100, 0)))
//		{
//			m_bResetWatermarkPos = true;
//		}
//
//		ImGui::EndPopup();
//	}
//	ImGui::PopStyleVar(2);
//	return pressed;
//}
//
//#define min(a, b) (((a) < (b)) ? (a) : (b))
//
//void create_watermark()
//{
//		if (!g_Vars.misc.watermark)
//			return;
//
//		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);
//		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 3));
//		ImVec2 p, s;
//
//		auto net_channel = Interfaces::m_pEngine->GetNetChannelInfo();
//
//		std::string watermark = XorStr("vader.tech");
//#ifdef DEV
//		watermark.append(XorStr(" [debug]")); // :)
//#endif
//#ifdef BETA_MODE
//		watermark.append(XorStr(" [beta]")); // :)
//#endif
//
//		const std::string user = g_Vars.globals.c_username;
//
//		if (user.empty())
//			LI_FN(exit)(0); // Random ass check lol
//
//		if (g_Vars.misc.watermarkadditives_WATERMARK_NAME)
//			watermark += XorStr(" | ") + user;
//
//		if (g_Vars.misc.watermarkadditives_WATERMARK_FPS) {
//			static auto framerate = 0.0f;
//			framerate = 0.9f * framerate + 0.1f * Interfaces::m_pGlobalVars->absoluteframetime;
//
//			if (framerate <= 0.0f)
//				framerate = 1.0f;
//
//			watermark += XorStr(" | ") + std::to_string((int)(1.0f / framerate)) + XorStr(" fps ");
//		}
//
//		if (Interfaces::m_pEngine->IsInGame())
//		{
//			auto nci = Interfaces::m_pEngine->GetNetChannelInfo();
//
//			if (nci)
//			{
//				auto server = nci->GetAddress();
//
//				if (!strcmp(server, XorStr("loopback")))
//					server = XorStr("local server");
//				else if (!strcmp(server, XorStr("162.248.92.227:27015")))
//					server = XorStr("emporium");
//				else if (!strcmp(server, XorStr("20.121.195.15:27015")))
//					server = XorStr("emporium 2");
//
//				auto tickrate = std::to_string((int)(1.0f / Interfaces::m_pGlobalVars->interval_per_tick));
//
//				if (g_Vars.misc.watermarkadditives_WATERMARK_SERVER)
//				{
//					watermark += XorStr(" | ");
//					watermark += server;
//				}
//
//				if (g_Vars.misc.watermarkadditives_WATERMARK_PING) {
//					auto outgoing = std::max(0, (int)std::round(nci->GetLatency(FLOW_OUTGOING) * 1000.f));
//
//					watermark += XorStr(" | ") + std::to_string(outgoing) + XorStr(" ms");
//				}
//			}
//
//		}
//		else
//		{
//			if (g_Vars.misc.watermarkadditives_WATERMARK_SERVER)
//				watermark += XorStr(" | no connection ");
//
//		}
//
//		if (g_Vars.misc.watermarkadditives_WATERMARK_TIME) {
//			time_t lt;
//			struct tm* t_m;
//
//			lt = time(nullptr);
//			t_m = localtime(&lt);
//
//			auto time_h = t_m->tm_hour;
//			auto time_m = t_m->tm_min;
//			auto time_s = t_m->tm_sec;
//
//			std::string time;
//
//			if (time_h < 10)
//				time += XorStr("0");
//
//			time += std::to_string(time_h) + XorStr(":");
//
//			if (time_m < 10)
//				time += XorStr("0");
//
//			time += std::to_string(time_m) + XorStr(":");
//
//			if (time_s < 10)
//				time += XorStr("0");
//
//			time += std::to_string(time_s);
//
//			watermark += XorStr(" | ") + time;
//		}
//
//		auto size_text = ImGui::CalcTextSize(watermark.c_str());
//		ImGui::SetNextWindowSize(ImVec2(size_text.x + (g_IMGUIMenu.Opened ? 24 : 14), 20));
//
//		const auto margin = 10; // Padding between screen edges and watermark
//		const auto padding = 4; // Padding between watermark elements
//		static bool m_bSetFirstPos = true;
//		if (m_bSetFirstPos) {
//			g_Vars.misc.watermark_window_x = Render::GetScreenSize().x - margin - padding - size_text.x - padding;
//			g_Vars.misc.watermark_window_y = margin;
//			m_bSetFirstPos = false;
//		}
//
//		if (m_bResetWatermarkPos) {
//			g_Vars.misc.watermark_window_x = Render::GetScreenSize().x - margin - padding - size_text.x - padding;
//			g_Vars.misc.watermark_window_y = margin;
//			m_bResetWatermarkPos = false;
//		}
//		
//		if (!g_Vars.misc.watermark_lockdefaultposition) {
//			if (!ImGui::IsMouseDragging()) {
//				ImGui::SetNextWindowPos(ImVec2(g_Vars.misc.watermark_window_x, g_Vars.misc.watermark_window_y), ImGuiCond_Always);
//			}
//		}
//		else {
//			ImGui::SetNextWindowPos(ImVec2(Render::GetScreenSize().x - margin - padding - size_text.x - padding, margin), ImGuiCond_Always);
//		}
//		ImGui::Begin(XorStr("watermark"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
//		{
//
//			auto d = ImGui::GetWindowDrawList();
//			p = ImGui::GetWindowPos();
//			s = ImGui::GetWindowSize();
//			ImGui::SetWindowSize(ImVec2(s.x, 21 + 18));
//
//			if (g_Vars.misc.watermark_window_x != p.x) {
//				g_Vars.misc.watermark_window_x = p.x;
//			}
//
//			if (g_Vars.misc.watermark_window_y != p.y) {
//				g_Vars.misc.watermark_window_y = p.y;
//			}
//
//			d->AddRectFilled(p, p + ImVec2(s.x, 21), ImColor(39, 39, 39, int(50 * 1)));
//			auto main_colf = ImColor(41, 32, 59, 210);
//			auto main_coll = ImColor(41, 32, 59, 210);
//			d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 20), main_coll, main_colf, main_colf, main_coll);
//			d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 20), main_colf, main_coll, main_coll, main_colf);
//			auto main_colf2 = ImColor(41, 32, 59, 210);
//			d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 20), main_coll, main_colf2, main_colf2, main_coll);
//			d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 20), main_colf2, main_coll, main_coll, main_colf2);
//
//			ImColor line_colf;
//			ImColor line_coll;
//
//			if (!g_Vars.misc.custom_menu) {
//				line_colf = ImColor(255, 0, 0, 255);
//				line_coll = ImColor(255, 0, 0, 255);
//			}
//			else {
//				line_colf = (ImColor)g_Vars.misc.accent_color;
//				line_coll = (ImColor)g_Vars.misc.accent_color;
//			}
//
//			d->AddRectFilledMultiColor(p, p + ImVec2(s.x / 2, 2), line_coll, line_colf, line_colf, line_coll);
//			d->AddRectFilledMultiColor(p + ImVec2(s.x / 2, 0), p + ImVec2(s.x, 2), line_colf, line_coll, line_coll, line_colf);
//			d->AddText(p + ImVec2((g_IMGUIMenu.Opened ? s.x - 10 : s.x) / 2 - size_text.x / 2, (20) / 2 - size_text.y / 2), ImColor(250, 250, 250, int(230 * min(1 * 3, 1.f))), watermark.c_str());
//			ImGui::SetCursorPos(ImVec2(s.x - 15, 5));
//			if (g_IMGUIMenu.Opened)
//				draw_water_button(XorStr(" "), XorStr("Swatermark_sett"), false, false, NULL, false);
//
//
//		}
//		ImGui::End();
//		ImGui::PopStyleVar(2);
//}
//
//void IMGUIMenu::Render()
//{
//
//	create_watermark();
//
//	if (!Opened) return;
//
//	//ImGui::GetIO().MouseDrawCursor = _visible;
//
//	ImGuiStyle* style = &ImGui::GetStyle();
//
//	ImGui::PushFont(gravityBold);
//
//	ImGui::SetNextWindowSize(ImVec2(MENU_WIDTH, MENU_HEIGHT));
//	ImGui::Begin(XorStr("##menu"), &_visible, windowFlags);
//
//	menu_position = ImGui::GetWindowPos();
//
//	style->WindowPadding = ImVec2(7.f, 7.f);
//
//	if (!g_Vars.misc.custom_menu) {
//		style->Colors[ImGuiCol_MenuAccent] = ImColor(255, 0, 0);
//		style->Colors[ImGuiCol_Logo] = ImColor(255, 0, 0);
//	}
//	else {
//		style->Colors[ImGuiCol_MenuAccent] = (ImColor)g_Vars.misc.accent_color;
//		style->Colors[ImGuiCol_Logo] = (ImColor)g_Vars.misc.logo_color;
//	}
//
//
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//	{
//		ImGui::BeginTitleBar(XorStr("Title Bar"), ImVec2(MENU_WIDTH, 55.f), false);
//
//		ImGui::PopFont();
//		ImGui::PushFont(StarWars);
//		ImGui::SameLine(8.f, -0.1f, 5.f);
//		ImGui::PushStyleColor(ImGuiCol_Text, ImGuiCol_Logo);
//		ImGui::Text(XorStr("VADER"));
//		ImGui::PopStyleColor();
//		ImGui::PopFont();
//		ImGui::PushFont(gravityBold);
//
//		ImGui::EndTitleBar();
//	}
//	ImGui::PopStyleVar();
//	style->Colors[ImGuiCol_ChildBg] = ImColor(41, 32, 59);
//
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
//	{
//		ImGui::BeginChild(XorStr("##tabs"), ImVec2(MENU_WIDTH, 35.f), false);
//
//		ImGui::SameLine(8.f);
//		ImGui::TrueTab(XorStr("  AIMBOT  "), tab, 0, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  HVH  "), tab, 1, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  VISUALS  "), tab, 2, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  MISC  "), tab, 3, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  SKINS  "), tab, 4, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  SCRIPTS  "), tab, 5, ImVec2(0.f, 35.f)); ImGui::SameLine();
//		ImGui::TrueTab(XorStr("  PLAYERLIST  "), tab, 6, ImVec2(0.f, 35.f)); ImGui::SameLine();
//
//		ImGui::EndChild();
//	}
//	ImGui::PopStyleVar();
//
//	style->Colors[ImGuiCol_ChildBg] = ImColor(31, 24, 46);
//
//	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); /* all SubTabs are handled here */
//	{
//		ImGui::BeginChild(XorStr("##subtabs"), ImVec2(MENU_WIDTH, 25.f), false);
//
//		ImGui::SameLine(8.f);
//
//
//		switch (tab)
//		{
//			case 0:
//			{
//				ImGui::TrueSubTab(XorStr("  Main  "), rageTab, -1, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Other  "), rageTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Pistols  "), rageTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Heavy Pistols  "), rageTab, 2, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Rifles  "), rageTab, 3, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  AWP  "), rageTab, 4, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Scout  "), rageTab, 5, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Auto  "), rageTab, 6, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  SMG  "), rageTab, 7, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Heavys  "), rageTab, 8, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Shotguns  "), rageTab, 9, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 1:
//			{
//				ImGui::TrueSubTab(XorStr("  Main  "), AAtab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Fakelag  "), AAtab, 1, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 2:
//			{
//				ImGui::TrueSubTab(XorStr("  Player  "), visualsSubTab, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  World  "), visualsSubTab, 1, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Misc  "), visualsSubTab, 2, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Chams  "), visualsSubTab, 3, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 3:
//			{
//				ImGui::TrueSubTab(XorStr("  Misc  "), miscSubtabs, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Configs  "), miscSubtabs, 1, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 4:
//			{
//				ImGui::TrueSubTab(XorStr("  Main  "), skinsSubtabs, 0, ImVec2(0.f, 25.f)); ImGui::SameLine();
//				ImGui::TrueSubTab(XorStr("  Weapons  "), skinsSubtabs, 1, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 5:
//			{
//				ImGui::TrueSubTab(XorStr("  Main  "), scriptsSubtabs, 0, ImVec2(0.f, 25.f));
//				break;
//			}
//			case 6:
//			{
//				ImGui::TrueSubTab(XorStr("  Main  "), PlayerListSubTabs, 0, ImVec2(0.f, 25.f));
//				break;
//			}
//		}
//
//		ImGui::EndChild();
//	}
//	ImGui::PopStyleVar();
//
//	style->Colors[ImGuiCol_ChildBg] = ImColor(25, 20, 27);
//	ImGui::BeginChild(XorStr("##main"), ImVec2(MENU_WIDTH, MENU_HEIGHT - 130.f), false);
//	ImGui::SameLine(7.f);
//	style->Colors[ImGuiCol_ChildBg] = ImColor(21, 17, 29);
//	switch (tab)
//	{
//		case 0:
//			Ragebot();
//			break;
//		case 1:
//			HvH();
//			break;
//		case 2:
//			Visuals();
//			break;
//		case 3:
//			Misc();
//			break;
//		case 4:
//			Skins();
//			break;
//		case 5:
//			Scripts();
//			break;
//		case 6:
//			PlayerList();
//			break;
//		default:
//			break;
//	}
//	style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
//
//	//ImGui::AddCircleImageFilled(
//	//	logo_nuts,
//	//	ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(45, 35),
//	//	30.f,
//	//	ImColor(255, 255, 255),
//	//	360
//	//);
//
//	auto window = ImGui::GetCurrentWindow();
//	if (window->SkipItems)
//		return;
//
//	ImGuiContext& g = *GImGui;
//	window->DrawList->PathClear();
//
//	//const std::string user = g_Vars.globals.user_info.username;
//	//char buff[24]; // max username is 12 
//	//sprintf_s(buff, XorStr("Hello, %s"), user.c_str());
//	//auto user_size = ImGui::CalcTextSize(user.c_str());
//
//	//window->DrawList->AddText(ImVec2{ ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(160 + (user_size.x / 2), 30) }, ImColor(255, 255, 255), buff);
//
//	ImGui::EndChild();
//	ImGui::End();
//	ImGui::PopFont();
//  
//}
//
//void IMGUIMenu::Toggle()
//{
//    _visible = !_visible;
//}
//
//void IMGUIMenu::CreateStyle()
//{
//	ImGuiStyle& style = ImGui::GetStyle();
//	ImGui::StyleColorsDark(&style);
//	style.WindowRounding = 0.0f;
//	style.WindowBorderSize = 1.0f;
//	style.ChildBorderSize = 1.0f;
//	style.GrabMinSize = 5.0f;
//	style.ScrollbarSize = 12.f;
//	style.ScrollbarRounding = 0.0f;
//	style.GrabRounding = 0.0f;
//	style.FrameRounding = 0.0f;
//	style.PopupRounding = 0.0f;
//	style.FrameBorderSize = 0.4f;
//	ImGuiIO& io = ImGui::GetIO();
//	io.IniFilename = nullptr;
//	io.LogFilename = nullptr;
//	//io.MouseDrawCursor = true;
//	char buffer[MAX_PATH];
//	//GetWindowsDirectoryA(buffer, MAX_PATH);
//
//	gravity = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravity_compressed_data, gravity_compressed_size, 13.f);
//	StarWars = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(starwars2_compressed_data, starwars2_compressed_size, 50.f);
//	watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 16.f);
//	gravityBold = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size, 13.f);
//	ImGui::SmallestPixel = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(smallestpixel_compressed_data, smallestpixel_compressed_size, 10, NULL, io.Fonts->GetGlyphRangesCyrillic());
//
//}
//
//void CustomCheckbox::Draw(std::string title, bool* value)
//{
//	using namespace std::chrono;
//	auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() / 10;
//
//	auto balls = *value;
//
//	if (!FirstDraw)
//	{
//		AnimStep = balls ? 200.f : 0.f;
//		LastStep = ms;
//		FirstDraw = true;
//	}
//
//	ImGui::CheckBoxAnimated((title).c_str(), value, AnimStep);
//
//
//	auto delta = ms - LastStep;
//	auto mod = 5 * delta; //9 / (1000 / 60) = 0.54
//	AnimStep += balls ? mod : -mod;
//	LastStep = ms;
//
//	if (AnimStep <= 0) {
//		AnimStep = 0;
//		LastStep = ms;
//	}
//	else if (AnimStep >= 200) {
//		AnimStep = 200;
//		LastStep = ms;
//	}
//}