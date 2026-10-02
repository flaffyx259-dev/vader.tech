#include "../../Utils/HttpRequest.hpp"
#include "lua_api.h"
#include "../../source.hpp"
#include "../../Features/Visuals/EventLogger.hpp"
#include "../../Utils/LogSystem.hpp"
#include "../../Features/Miscellaneous/Miscellaneous.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../Features/Rage/TickbaseShift.hpp"
#include "../../ShittierMenu/Menu.hpp"
#include "../../Utils/lazy_importer.hpp"
#include<fstream>
#include "../../Features/Rage/ShotInformation.hpp"
#include "../../ShittierMenu/MenuNEW.h"

#define engine_console(x) ILoggerEvent::Get()->PushEvent(x, FloatColor(1.f, 1.f, 1.f), true, "")
#define engine_console_error(x) ILoggerEvent::Get()->PushEvent(x, FloatColor(1.f, 0.f, 0.f), true, "")
void lua_panic(sol::optional<std::string> message) {

	if (message) {
		std::string m = message.value();
		MessageBoxA(0, m.c_str(), ("Lua: panic state"), MB_APPLMODAL | MB_OK);
	}
}

// ----- lua functions -----

int extract_owner(sol::this_state st) {
	sol::state_view lua_state(st);
	sol::table rs = lua_state["debug"]["getinfo"](2, "S");
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();
	return g_lua.get_script_id(filename);
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return g_lua.get_script_id(get_current_script(s));
}

namespace lua_events {

	void gameevent_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		g_luagameeventmanager.register_gameevent(eventname, g_lua.get_script_id(filename), func);
		engine_console(filename + XorStr(": registered to ") + eventname);
	}

}

std::vector <std::pair <std::string, menu_item>>::iterator find_item(std::vector <std::pair <std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}

namespace lua_ui {
	Vector2D keybinds_pos() {
		float x, y;
		x = g_Vars.esp.keybind_window_x;
		y = g_Vars.esp.keybind_window_y;

		return Vector2D(x, y);
	}

	Vector2D speclist_pos() {
		float x, y;
		x = g_Vars.esp.spec_window_x;
		y = g_Vars.esp.spec_window_y;

		return Vector2D(x, y);
	}

	bool keybinds_open() {
		return g_Vars.globals.m_bKeyBindOpen;
	}

	bool speclist_open() {
		return g_Vars.globals.m_bSpecListOpen;
	}

	bool menu_open() {
		return Menu::opened;
	}

	Vector2D mouse_pos() {
		return InputSys::Get()->GetMousePosition();
	}

	Vector2D menu_pos() {
		float x, y;
		x = Menu::menu_position.x;
		y = Menu::menu_position.y;

		return Vector2D(x, y);
	}

	void add_check_box(sol::this_state s, std::string key, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = g_lua.get_script_id(script);

		auto& items = g_lua.items.at(script_id);
		auto full_name = name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(key, false)));
	}

	void add_slider(sol::this_state s, std::string key, const std::string& name, float min, float max, const char* format)
	{
		auto script = get_current_script(s);
		auto script_id = g_lua.get_script_id(script);

		auto& items = g_lua.items.at(script_id);
		auto full_name = name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(key, min, max, min, format)));
	}

	void add_color_picker(sol::this_state s, std::string key, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = g_lua.get_script_id(script);

		auto& items = g_lua.items.at(script_id);
		auto full_name = name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(key, FloatColor(255, 255, 255, 255))));
	}

	void add_text(sol::this_state s, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = g_lua.get_script_id(script);

		auto& items = g_lua.items.at(script_id);
		auto full_name = name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, name));
	}

	void add_button(sol::this_state s, std::string key, const std::string& name)
	{
		auto script = get_current_script(s);
		auto script_id = g_lua.get_script_id(script);

		auto& items = g_lua.items.at(script_id);
		auto full_name = name;

		if (find_item(items, full_name) != items.end())
			return;

		items.emplace_back(std::make_pair(full_name, menu_item(key, false, false)));
	}
}

namespace lua_config {
	/*
	config.get(key)
	Returns value of given key or nil if key not found.
	*/
	std::tuple<sol::object, sol::object, sol::object, sol::object> get(sol::this_state s, std::string key) {
		std::tuple<sol::object, sol::object, sol::object, sol::object> retn = std::make_tuple(sol::nil, sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_BOOL)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_COLOR)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, (int)(kv.second[0] * 255)), sol::make_object(s, (int)(kv.second[1] * 255)), sol::make_object(s, (int)(kv.second[2] * 255)), sol::make_object(s, (int)(kv.second[3] * 255)));

		for (auto kv : LuaConfigSystem::C_FLOAT)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_INT)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		for (auto kv : LuaConfigSystem::C_MULTI)
			if (kv.first == key)
				retn = std::make_tuple(sol::make_object(s, kv.second), sol::nil, sol::nil, sol::nil);

		return retn;
	}

	void config_save() {
		LuaConfigSystem::Save();
	}

	void config_load() {
		LuaConfigSystem::Load();
	}

	/*
	config.set(key, value)
	Sets value for key
	*/
	void set_bool(std::string key, bool v) {
		LuaConfigSystem::C_BOOL[key] = v;
	}

	void set_float(std::string key, float v) {
		if (ceilf(v) != v)
			LuaConfigSystem::C_FLOAT[key] = v;
		else
			LuaConfigSystem::C_INT[key] = (int)v;
	}

	void set_color(std::string key, int r, int g, int b, int a) {
		LuaConfigSystem::C_COLOR[key][0] = r / 255.f;
		LuaConfigSystem::C_COLOR[key][1] = g / 255.f;
		LuaConfigSystem::C_COLOR[key][2] = b / 255.f;
		LuaConfigSystem::C_COLOR[key][3] = a / 255.f;
	}

	void set_multiselect(std::string key, int pos, bool e) {
		LuaConfigSystem::C_MULTI[key][pos] = e;
	}

	void set_int(std::string key, int value) {
		LuaConfigSystem::C_INT[key] = value;
	}

	bool watermark_enabled() {
		return g_Vars.misc.watermark;
	}

	bool autopeek_enabled() {
		return g_Vars.misc.autopeek_bind.enabled;
	}

	bool pingspike_enabled() {
		if (g_Vars.misc.extended_backtrack) {
			return g_Vars.misc.extended_backtrack_key.enabled;
		}
		else
			return false;
	}

	float pingspike_value() {
		return g_Vars.misc.extended_backtrack_time;
	}

	bool move_exploit_enabled() {
		return g_Vars.misc.move_exploit_key.enabled;
	}

	bool fakeduck_enabled() {
		return g_Vars.misc.fakeduck_bind.enabled;
	}

	bool doubletap_enabled() {
		return g_Vars.rage.key_dt.enabled;
	}

	int antiaim_manual_enabled() {
		if (!g_Vars.antiaim.manual)
			return -1;

		switch (g_Vars.globals.manual_aa) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 2;
			break;
		}
	}

	int antiaim_base_yaw() {
		return g_Vars.antiaim_stand.base_yaw;
	}

	void antiaim_base_yaw_set(int value) {
		g_Vars.antiaim_stand.base_yaw = value;
	}

	//int antiaim_jitter_value() {
	//	return g_Vars.antiaim.Jitter_range;
	//}

	//void antiaim_jitter_set(int value) {
	//	g_Vars.antiaim.Jitter_range = value;
	//}

	bool antiaim_fakewalk_enabled() {
		return g_Vars.misc.slow_walk_bind.enabled;
	}

	bool forcebaim_enabled() {
		if (g_Vars.rage.enabled) {
			return g_Vars.rage.prefer_body.enabled;
		}
		else
			return false;
	}

	bool slide_walk_set(bool value) {
		return g_Vars.misc.slide_walk = value;
	}

	bool mindtrick_enabled() {
		if (g_Vars.misc.mind_trick) {
			return g_Vars.misc.mind_trick_bind.enabled;
		}
		else
			return false;
	}

	bool fakewalk_enabled() {
		if (g_Vars.misc.slow_walk) {
			return g_Vars.misc.slow_walk_bind.enabled;
		}
		else
			return false;
	}

	int fakewalk_set_speed(int value) {
		return g_Vars.misc.slow_walk_speed = value;
	}

	bool dmgoverride_enabled() {
		if (g_Vars.rage.enabled && g_Vars.globals.OverridingMinDmg && g_Vars.rage.key_dmg_override.enabled) {
			return true;
		}
		else
			return false;
	}

	bool hitscanoverride_enabled() {
		if (g_Vars.rage.enabled && g_Vars.globals.OverridingHitscan && g_Vars.rage.override_key.enabled) {
			return true;
		}
		else
			return false;
	}

	float break_angle_set(float value) {
		return g_Vars.antiaim.break_lby = value;
	}

	float first_break_angle_set(float value) {
		return g_Vars.antiaim.break_lby_first = value;
	}
}

namespace lua_cheat {
	void set_event_callback(sol::this_state s, std::string eventname, sol::function func) {
		sol::state_view lua_state(s);
		sol::table rs = lua_state["debug"]["getinfo"](2, ("S"));
		std::string source = rs["source"];
		std::string filename = std::filesystem::path(source.substr(1)).filename().string();

		g_luahookmanager.register_hook(eventname, g_lua.get_script_id(filename), func);
		engine_console(filename + XorStr(": subscribed to event ") + eventname);
	}

	void run_script(std::string scriptname) {
		int scrid = g_lua.get_script_id(scriptname);
		if (scrid < 0)
			return;

		g_lua.load_script(scrid);
	}

	void reload_active_scripts() {
		g_lua.reload_all_scripts();
	}

	void log(const std::string& text) {
		engine_console(text);
	}

	void set_button(int button, int mode)
	{
		if (mode <= 0) g_Vars.globals.m_pCmd->buttons |= (1 << button);
		if (mode >= 1) g_Vars.globals.m_pCmd->buttons &= ~(1 << button);
	}

	bool get_button(int button)
	{
		if (g_Vars.globals.m_pCmd->buttons & (1 << button)) return true;

		return false;
	}

	std::string username()
	{
		return g_Vars.globals.c_username;
	}

	bool dt_charged() {
		if (g_TickbaseController.s_nExtraProcessingTicks > 0 && g_Vars.rage.key_dt.enabled)
			return true;

		return false;
	}

	void* create_interface(const std::string& image_name, const std::string& name) {
		return Interfaces::CreateInterface(image_name, name);
	}

}

namespace lua_math {
	QAngle calc_angle(Vector& src, Vector& dst)
	{
		return Math::CalcAngle(src, dst);
	}

	float get_fov(Vector& src, Vector& dst)
	{
		return Math::VecGetFov(src, dst);
	}
}

namespace lua_modelinfo {
	studiohdr_t* get_studio_model(model_t* mdl) {
		return Interfaces::m_pModelInfo->GetStudiomodel(mdl);
	}

	int get_model_index(std::string name) {
		return Interfaces::m_pModelInfo->GetModelIndex(name.c_str());
	}
}
namespace lua_debugoverlay {
	void add_box_overlay(Vector pos, Vector mins, Vector maxs, QAngle orientation, int r, int g, int b, int a, float duration) {
		Interfaces::m_pDebugOverlay->AddBoxOverlay(pos, mins, maxs, orientation, r, g, b, a, duration);
	}

	void add_line_overlay(Vector src, Vector dst, Color clr, bool nodepthtest, float duration) {
		Interfaces::m_pDebugOverlay->AddLineOverlayAlpha(src, dst, clr.r(), clr.g(), clr.b(), clr.a(), nodepthtest, duration);
	}

	void add_text_overlay(Vector pos, int line_offset, float duration, sol::variadic_args txt) {
		std::string text = "";
		for (auto v : txt)
			text += v;

		Interfaces::m_pDebugOverlay->AddTextOverlay(pos, duration, text.c_str());
	}
}
namespace lua_engine {
	Vector2D get_screen_size() {
		int w, h;
		Interfaces::m_pEngine->GetScreenSize(w, h);
		return Vector2D(w, h);
	}

	int get_player_for_user_id(int userid) {
		return Interfaces::m_pEngine->GetPlayerForUserID(userid);
	}

	int get_local_player_index() {
		return Interfaces::m_pEngine->GetLocalPlayer();
	}

	QAngle get_view_angles() {
		QAngle vaa;
		Interfaces::m_pEngine->GetViewAngles(vaa);
		return vaa;
	}

	void set_view_angles(QAngle va) {
		Interfaces::m_pEngine->SetViewAngles(va);
	}

	int get_max_clients() {
		return Interfaces::m_pEngine->GetMaxClients();
	}

	bool is_in_game() {
		return Interfaces::m_pEngine->IsInGame();
	}

	bool is_connected() {
		return Interfaces::m_pEngine->IsConnected();
	}

	INetChannel* get_net_channel_info() {
		return (INetChannel* )Interfaces::m_pEngine->GetNetChannelInfo();
	}

	void execute_client_cmd(std::string cmd) {
		Interfaces::m_pEngine->ClientCmd_Unrestricted(cmd.c_str());
	}

	player_info_t get_player_info(int i)
	{
		player_info_t player_info;
		Interfaces::m_pEngine->GetPlayerInfo(i, &player_info);

		return player_info;
	}
}
namespace lua_entitylist {
	C_CSPlayer* get_client_entity(int idx) {
		return (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(idx);
	}

	int get_highest_entity_index() {
		return Interfaces::m_pEntList->GetHighestEntityIndex();
	}

	C_CSPlayer* get_local_player() {
		return C_CSPlayer::GetLocalPlayer();
	}
}

namespace lua_utils {
	sol::table get_player_data(player_info_t& p) {
		sol::table t = g_lua.lua.create_table();
		t[XorStr("name")] = std::string(p.szName);
		t[XorStr("steamid")] = std::string(p.szSteamID);
		t[XorStr("userid")] = p.userId;
		return t;
	}

	double clamp(double v, double mi, double ma) {
		return std::clamp(v, mi, ma);
	}

	uint64_t find_signature(const std::string& szModule, const std::string& szSignature)
	{
		return Memory::Scan(szModule.c_str(), szSignature.c_str());
	}

	void apply_clan_tag_func(const char* name) {
		static auto apply_clan_tag = (int(__fastcall*)(const char*, const char*)) Engine::Displacement.Function.m_uClanTagChange;
		apply_clan_tag(name, name);
	}

	void apply_clan_tag(std::string clan) {
		apply_clan_tag_func(clan.c_str());
	}

	Color create_color(int r, int g, int b, int a) {
		return Color(r, g, b, a);
	}

	FloatColor create_floatcolor(float r, float g, float b, float a) {
		return FloatColor(r, g, b, a);
	}

	Color create_color_rainbow(int alpha) {
		static float rainbow;
		rainbow += 0.001f;
		if (rainbow > 1.f)
			rainbow = 0.f;



		return Color::HSBtoRGB(rainbow, 1.0f, 1.0f, alpha);
	}

	float lowerbody_yaw() {

		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local->IsAlive())
			return 0.f;

		if (local->m_vecVelocity().Length2D() > 30.f && !g_Vars.misc.slow_walk_bind.enabled) // bad fix for localplayer walking
			return 0.f;

		return std::abs(Math::NormalizedAngle(g_Vars.globals.m_flBody - g_Vars.globals.RegularAngles.y));
	}

	bool local_alive() {
		auto local = C_CSPlayer::GetLocalPlayer();

		if (!local || !local->IsAlive())
			return false;


		return true;
	}

	bool key_down(int key) {
		return InputHelper::Down(key);
	}

}

namespace lua_globals {
	float realtime() {
		return Interfaces::m_pGlobalVars->realtime;
	}

	int framecount() {
		return Interfaces::m_pGlobalVars->framecount;
	}

	float absoluteframetime() {
		return Interfaces::m_pGlobalVars->frametime;
	}

	float curtime() {
		return Interfaces::m_pGlobalVars->curtime;
	}

	float frametime() {
		return Interfaces::m_pGlobalVars->frametime;
	}

	int maxclients() {
		return Interfaces::m_pGlobalVars->maxClients;
	}

	int tickcount() {
		return Interfaces::m_pGlobalVars->tickcount;
	}

	float tickinterval() {
		return Interfaces::m_pGlobalVars->interval_per_tick;
	}

	std::string get_server_address()
	{
		if (!Interfaces::m_pEngine->IsInGame())
			return XorStr("Unknown");

		auto nci = Interfaces::m_pEngine->GetNetChannelInfo();

		if (!nci)
			return XorStr("Unknown");

		auto server = nci->GetAddress();

		if (!strcmp(server, XorStr("loopback")))
			server = XorStr("Local server");

		return server;
	}
}

namespace lua_cvar {

	ConVar* find_var(std::string name) {
		return Interfaces::m_pCvar->FindVar(name.c_str());
	}

	void console_color_print(Color col, sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(col, txt.c_str());
	}

	void console_print(sol::variadic_args args) {
		std::string txt = "";
		for (auto v : args)
			txt += v;

		Interfaces::m_pCvar->ConsoleColorPrintf(Color(255, 255, 255, 255), txt.c_str());
	}

	void unlock_cvar(ConVar* var)
	{
		if (var)
		{
			var->nFlags &= ~FCVAR_DEVELOPMENTONLY;
			var->nFlags &= ~FCVAR_HIDDEN;
		}
	}

	float get_float(ConVar* var) {
		return var ? var->GetFloat() : 0.f;
	}

	int get_int(ConVar* var) {
		return var ? var->GetInt() : 0;
	}

	const char* get_string(ConVar* var) {
		return var ? var->GetString() : "";
	}

	void set_float(ConVar* var, float f) {
		if (var) var->SetValue(std::to_string(f).c_str());
	}

	void set_int(ConVar* var, int i) {
		if (var) var->SetValue(std::to_string(i).c_str());
	}

	void set_string(ConVar* var, const char* v) {
		if (var)
			var->SetValue(v);
	}
}

namespace lua_surface {
	void set_texture(int id) {
		Interfaces::m_pSurface->DrawSetTexture(id);
	}

	void set_texture_rgba(int id, const unsigned char* rgba, int w, int h) {
		Interfaces::m_pSurface->DrawSetTextureRGBA(id, rgba, w, h);
	}

	void set_color(Color col) {
		Interfaces::m_pSurface->DrawSetColor(col.r(), col.g(), col.b(), col.a());
	}

	int create_texture() {
		return Interfaces::m_pSurface->CreateNewTextureID();
	}

	void draw_filled_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawFilledRect(x, y, x2, y2);
	}

	void draw_outlined_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawOutlinedRect(x, y, x2, y2);
	}

	Vector2D get_text_size(int font, std::wstring text) {
		int w, h;
		Interfaces::m_pSurface->GetTextSize(font, text.c_str(), w, h);
		return Vector2D(w, h);
	}

	void draw_line(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawLine(x, y, x2, y2);
	}

	void draw_outlined_circle(int x, int y, int radius, int segments) {
		Interfaces::m_pSurface->DrawOutlinedCircle(x, y, radius, segments);
	}

	int create_font(std::string fontname, int w, int h, int blur, int flags) {
		auto f = Interfaces::m_pSurface->CreateFont_();
		Interfaces::m_pSurface->SetFontGlyphSet(f, fontname.c_str(), w, h, blur, 0, flags);
		return f;
	}

	void set_text_font(int font) {
		Interfaces::m_pSurface->DrawSetTextFont(font);
	}

	void set_text_color(Color col) {
		Interfaces::m_pSurface->DrawSetTextColor(col.r(), col.g(), col.b(), col.a());
	}

	void set_text_pos(int x, int y) {
		Interfaces::m_pSurface->DrawSetTextPos(x, y);
	}

	void draw_text(std::wstring str) {
		Interfaces::m_pSurface->DrawPrintText(str.c_str(), str.length());
	}

	void draw_textured_rect(int x, int y, int x2, int y2) {
		Interfaces::m_pSurface->DrawTexturedRect(x, y, x2, y2);
	}
}

namespace lua_render {

	void draw_text(int x, int y, unsigned long font, std::string string, bool text_centered, Color color)
	{
		Render::Engine::text((int)x, (int)y, font, string, text_centered, color);
	}

	Vector2D text_size(unsigned long font, std::string text) {
		return Render::Engine::GetTextSize(font, text);
	}

	void draw_line(float x, float y, float x2, float y2, Color col)
	{
		Render::Engine::Line((int)x, (int)y, (int)x2, (int)y2, col);
	}

	void draw_rect(float x, float y, float w, float h, Color col)
	{
		Render::Engine::Rect((int)x, (int)y, (int)w, (int)h, col);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color col)
	{
		Render::Engine::RectFilled((int)x, (int)y, (int)w, (int)h, col);
	}

	void draw_rect_outlined(int x, int y, int w, int h, Color color, Color color2)
	{
		Render::Engine::RectOutlined((int)x, (int)y, (int)w, (int)h, color, color2);
	}

	void draw_circle(int x, int y, int radius, int segments, Color color)
	{
		Render::Engine::CircleFilled((int)x, (int)y, (int)radius, (int)segments, color);
	}

	void draw_gradient(int x, int y, int w, int h, Color color, Color color2, bool horizontal)
	{
		Render::Engine::Gradient((int)x, (int)y, (int)w, (int)h, color, color2, horizontal);
	}

	void draw_world_circle(Vector origin, float radius, Color color, Color colorFill)
	{
		Render::Engine::WorldCircle(origin, radius, color, colorFill);
	}

	void draw_arc(int x, int y, int radius, int start_angle, int percent, int thickness, Color color)
	{
		Render::Engine::draw_arc(x, y, radius, start_angle, percent, thickness, color);
	}

	void draw_triangle(Vector2D pos1, Vector2D pos2, Vector2D pos3, Color col)
	{
		Render::Engine::FilledTriangle(pos1, pos2, pos3, col);
	}

	Vector world_to_screen(Vector pos) {
		Vector2D scr;
		Render::Engine::WorldToScreen(pos, scr);
		return Vector(scr.x, scr.y, 0);
	}

}

namespace lua_clientstate
{
	int chokedcommands() {
		return Interfaces::m_pClientState->m_nChokedCommands();
	}

}

namespace lua_effects
{
	void sparks(const Vector& position, int nMagnitude = 1, int nTrailLength = 1) {
		Interfaces::g_IEffects->Sparks(position, nMagnitude, nTrailLength);
	}

	void dust(const Vector& pos, const Vector& dir, float size, float speed) {
		Interfaces::g_IEffects->Dust(pos, dir, size, speed);
	}

	void EnergySplash(const Vector& position, const Vector& direction, bool bExplosive = false) {
		Interfaces::g_IEffects->EnergySplash(position, direction, bExplosive);
	}
}

namespace lua_http
{
	std::string get(sol::this_state s, std::string& link)
	{
		if (!g_Vars.misc.lua_allow_http_requests)
		{
			engine_console_error(XorStr("Please Allow HTTP Requests"), false);
			return "";
		}
		try
		{

			http::Request request(link);

			// send a get request
			const http::Response response = request.send(XorStr("GET"));
			return std::string(response.body.begin(), response.body.end());
		}
		catch (const std::exception& e)
		{
			engine_console_error(XorStr("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}
	std::string post(sol::this_state s, std::string& link, std::string& params)
	{
		if (!g_Vars.misc.lua_allow_http_requests)
		{
			engine_console_error(XorStr("Please Allow HTTP Requests"));
			return "";
		}
		try
		{
			http::Request request(link);
			// send a post request
			const http::Response response = request.send(XorStr("POST"), params, {
				XorStr("Content-Type: application/x-www-form-urlencoded")
				});
			return std::string(response.body.begin(), response.body.end());
		}
		catch (const std::exception& e)
		{
			engine_console_error(XorStr("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}
}

namespace lua_file
{
	void append(sol::this_state s, std::string& path, std::string& data)
	{
		std::ofstream out(path, std::ios::app | std::ios::binary);

		if (out.is_open())
			out << data;
		else
			engine_console_error(XorStr("Can't append to file: ") + path);

		out.close();
	}
	void write(sol::this_state s, std::string& path, std::string& data)
	{
		std::ofstream out(path, std::ios::binary);

		if (out.is_open())
			out << data;
		else
			engine_console_error(XorStr("Can't write to file: ") + path);

		out.close();
	}
	std::string read(sol::this_state s, std::string& path)
	{

		std::ifstream file(path, std::ios::binary);

		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return content;
		}
		else
		{
			engine_console_error(XorStr("Can't read file: ") + path);
			file.close();
			return "";
		}
	}
}

namespace lua_exploits {
	void force_charge()
	{
		g_TickbaseController.s_bBuilding = true;
	}

	void force_uncharge()
	{
		g_TickbaseController.s_bBuilding = false;
	}

	void force_speed(int speed)
	{
		g_TickbaseController.s_nSpeed = speed;
	}

	void force_chargedelay(float time)
	{
		g_TickbaseController.s_flTimeRequired = time;
	}

	void force_unchargestate(bool state)
	{
		g_TickbaseController.m_bForceUnChargeState = state;
	}
}

// ----- lua functions -----

c_lua g_lua;
bool c_lua::initialize() {
	this->lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	this->lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::os, sol::lib::io, sol::lib::bit32, sol::lib::ffi, sol::lib::jit);

	this->lua[XorStr("collectgarbage")] = sol::nil;
	this->lua[XorStr("__nil_callback")] = [](){};

	this->lua["print"] = [](std::string s) { engine_console(s); };
	this->lua["error"] = [](std::string s) { engine_console_error(s); };

	this->lua.new_usertype<Color>(XorStr("color"), sol::constructors <Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)XorStr("r"), &Color::r,
		(std::string)XorStr("g"), &Color::g,
		(std::string)XorStr("b"), &Color::b,
		(std::string)XorStr("a"), &Color::a
		);

	this->lua.new_usertype<CUserCmd>(XorStr("cmd"),
		XorStr("command_number"), sol::readonly(&CUserCmd::command_number),
		XorStr("tick_count"), sol::readonly(&CUserCmd::tick_count),
		XorStr("viewangles"), &CUserCmd::viewangles,
		XorStr("aimdirection"), &CUserCmd::aimdirection,
		XorStr("forwardmove"), &CUserCmd::forwardmove,
		XorStr("sidemove"), &CUserCmd::sidemove,
		XorStr("upmove"), &CUserCmd::upmove,
		XorStr("buttons"), &CUserCmd::buttons,
		XorStr("impulse"), sol::readonly(&CUserCmd::impulse),
		XorStr("weaponselect"), &CUserCmd::weaponselect,
		XorStr("weaponsubtype"), sol::readonly(&CUserCmd::weaponsubtype),
		XorStr("random_seed"), sol::readonly(&CUserCmd::random_seed),
		XorStr("mousedx"), &CUserCmd::mousedx,
		XorStr("mousedy"), &CUserCmd::mousedy,
		XorStr("hasbeenpredicted"), sol::readonly(&CUserCmd::hasbeenpredicted)
		);
	this->lua.new_usertype<IGameEvent>(XorStr("game_event"),
		XorStr("get_name"), &IGameEvent::GetName,
		XorStr("is_reliable"), &IGameEvent::IsReliable,
		XorStr("is_local"), &IGameEvent::IsLocal,
		XorStr("is_empty"), &IGameEvent::IsEmpty,
		XorStr("get_bool"), &IGameEvent::GetBool,
		XorStr("get_int"), &IGameEvent::GetInt,
		XorStr("get_uint64"), &IGameEvent::GetUint64,
		XorStr("get_float"), &IGameEvent::GetFloat,
		XorStr("get_string"), &IGameEvent::GetString
		);
	this->lua.new_enum(XorStr("hitboxes"),
		XorStr("HEAD"), Hitboxes::HITBOX_HEAD,
		XorStr("NECK"), Hitboxes::HITBOX_NECK,
		XorStr("LOWER_NECK"), Hitboxes::HITBOX_LOWER_NECK,
		XorStr("PELVIS"), Hitboxes::HITBOX_PELVIS,
		XorStr("STOMACH"), Hitboxes::HITBOX_STOMACH,
		XorStr("UPPER_CHEST"), Hitboxes::HITBOX_UPPER_CHEST,
		XorStr("CHEST"), Hitboxes::HITBOX_CHEST,
		XorStr("LOWER_CHEST"), Hitboxes::HITBOX_LOWER_CHEST,
		XorStr("RIGHT_THIGH"), Hitboxes::HITBOX_RIGHT_THIGH,
		XorStr("LEFT_THIGH"), Hitboxes::HITBOX_LEFT_THIGH,
		XorStr("RIGHT_CALF"), Hitboxes::HITBOX_RIGHT_CALF,
		XorStr("LEFT_CALF"), Hitboxes::HITBOX_LEFT_CALF,
		XorStr("RIGHT_FOOT"), Hitboxes::HITBOX_RIGHT_FOOT,
		XorStr("LEFT_FOOT"), Hitboxes::HITBOX_LEFT_FOOT,
		XorStr("RIGHT_HAND"), Hitboxes::HITBOX_RIGHT_HAND,
		XorStr("LEFT_HAND"), Hitboxes::HITBOX_LEFT_HAND,
		XorStr("RIGHT_UPPER_ARM"), Hitboxes::HITBOX_RIGHT_UPPER_ARM,
		XorStr("RIGHT_FOREARM"), Hitboxes::HITBOX_RIGHT_FOREARM,
		XorStr("LEFT_UPPER_ARM"), Hitboxes::HITBOX_LEFT_UPPER_ARM,
		XorStr("LEFT_FOREARM"), Hitboxes::HITBOX_LEFT_FOREARM,
		XorStr("MAX"), Hitboxes::HITBOX_MAX
	);
	this->lua.new_enum(XorStr("fontflags"),
		XorStr("italic"), FontFlags_t::FONTFLAG_ITALIC,
		XorStr("underline"), FontFlags_t::FONTFLAG_UNDERLINE,
		XorStr("strikeout"), FontFlags_t::FONTFLAG_STRIKEOUT,
		XorStr("symbol"), FontFlags_t::FONTFLAG_SYMBOL,
		XorStr("antialias"), FontFlags_t::FONTFLAG_ANTIALIAS,
		XorStr("gaussianblur"), FontFlags_t::FONTFLAG_GAUSSIANBLUR,
		XorStr("rotary"), FontFlags_t::FONTFLAG_ROTARY,
		XorStr("dropshadow"), FontFlags_t::FONTFLAG_DROPSHADOW,
		XorStr("additive"), FontFlags_t::FONTFLAG_ADDITIVE,
		XorStr("outline"), FontFlags_t::FONTFLAG_OUTLINE
	);
	this->lua.new_usertype<INetChannel>(XorStr("inetchannel"),
		XorStr("out_sequence_nr"), sol::readonly(&INetChannel::m_nOutSequenceNr),
		XorStr("in_sequence_nr"), sol::readonly(&INetChannel::m_nInSequenceNr),
		XorStr("out_sequence_nr_ack"), sol::readonly(&INetChannel::m_nOutSequenceNrAck),
		XorStr("out_reliable_state"), sol::readonly(&INetChannel::m_nOutReliableState),
		XorStr("in_reliable_state"), sol::readonly(&INetChannel::m_nInReliableState),
		XorStr("choked_packets"), sol::readonly(&INetChannel::m_nChokedPackets),
		XorStr("get_latency"), &INetChannel::GetLatency,
		XorStr("get_avg_latency"), &INetChannel::GetAvgLatency
		);
	this->lua.new_usertype<Vector2D>(XorStr("vector2d"),
		sol::constructors<Vector2D(), Vector2D(float, float), Vector2D(Vector2D)>(),
		XorStr("x"), &Vector2D::x,
		XorStr("y"), &Vector2D::y,
		XorStr("length"), &Vector2D::Length
		);
	this->lua.new_usertype<Vertex_t>(XorStr("c_vertex"),
		sol::constructors<Vertex_t(), Vertex_t(Vector2D), Vertex_t(Vector2D, Vector2D)>(),
		XorStr("init"), &Vertex_t::Init,
		XorStr("position"), &Vertex_t::m_Position,
		XorStr("tex_coord"), &Vertex_t::m_TexCoord
		);
	this->lua.new_usertype<Vector>(XorStr("vector"),
		sol::constructors<Vector(), Vector(float, float, float)>(),
		XorStr("x"), &Vector::x,
		XorStr("y"), &Vector::y,
		XorStr("z"), &Vector::z,
		XorStr("length"), &Vector::Length,
		XorStr("length_sqr"), &Vector::LengthSquared,
		XorStr("length2d"), &Vector::Length2D,
		XorStr("length2dsqr"), &Vector::Length2DSquared,
		XorStr("dist_to"), &Vector::Distance,
		XorStr("cross_product"), &Vector::Cross,
		XorStr("normalize"), &Vector::Normalize
		);
	this->lua.new_usertype<QAngle>(XorStr("qangle"),
		sol::constructors<QAngle(), QAngle(float, float, float)>(),
		XorStr("x"), &QAngle::x,
		XorStr("y"), &QAngle::y,
		XorStr("z"), &QAngle::z,
		XorStr("normalize"), &QAngle::Normalize
		);
	this->lua.new_usertype<C_CSPlayer>(XorStr("player"),
		XorStr("is_player"), &C_CSPlayer::IsPlayer,
		XorStr("is_alive"), &C_CSPlayer::IsAlive,
		XorStr("is_dead"), &C_CSPlayer::IsDead,
		XorStr("is_dormant"), &C_CSPlayer::IsDormant,
		XorStr("get_vec_origin"), &C_CSPlayer::m_vecOrigin,
		XorStr("get_abs_origin"), &C_CSPlayer::GetAbsOrigin,
		XorStr("get_eye_position"), &C_CSPlayer::GetEyePosition,
		XorStr("get_shoot_position"), &C_CSPlayer::GetShootPosition,
		XorStr("get_hitbox_position"), &C_CSPlayer::GetHitboxPosition,
		XorStr("get_index"), &C_CSPlayer::EntIndex,
		XorStr("get_team"), &C_CSPlayer::m_iTeamNum,
		XorStr("set_clientside_angle"), &C_CSPlayer::ForceAngleTo,
		XorStr("get_velocity"), &C_CSPlayer::m_vecVelocity,
		XorStr("get_fflags"), &C_CSPlayer::m_fFlags,
		XorStr("is_scoped"), &C_CSPlayer::m_bIsScoped,
		XorStr("get_velmodifer"), &C_CSPlayer::m_flVelocityModifier,
		XorStr("get_health"), &C_CSPlayer::m_iHealth,
		XorStr("get_prop_int"), &C_CSPlayer::GetPropInt,
		XorStr("get_prop_float"), &C_CSPlayer::GetPropFloat,
		XorStr("get_prop_bool"), &C_CSPlayer::GetPropBool,
		XorStr("get_prop_string"), &C_CSPlayer::GetPropString,
		XorStr("set_prop_int"), &C_CSPlayer::SetPropInt,
		XorStr("set_prop_float"), &C_CSPlayer::SetPropFloat,
		XorStr("set_prop_bool"), &C_CSPlayer::SetPropBool,
		XorStr("set_prop_string"), &C_CSPlayer::SetPropString
		);
	this->lua.new_usertype<Engine::ShotSnapshot>(XorStr("shot_info"), sol::constructors <>(),
		XorStr("result"), &Engine::ShotSnapshot::result,
		XorStr("client_damage"), &Engine::ShotSnapshot::client_damage,
		XorStr("server_damage"), &Engine::ShotSnapshot::server_damage,
		XorStr("client_hitbox"), &Engine::ShotSnapshot::client_hitbox,
		XorStr("server_hitbox"), &Engine::ShotSnapshot::server_hitbox,
		XorStr("backtrack_ticks"), &Engine::ShotSnapshot::backtrack_ticks,
		XorStr("hitchance"), &Engine::ShotSnapshot::hitchance,
		XorStr("player"), &Engine::ShotSnapshot::player
		);

	auto events = this->lua.create_table();
	events[XorStr("register_event")] = lua_events::gameevent_callback;

	auto config = this->lua.create_table();
	config[XorStr("get")] = lua_config::get;
	config[XorStr("set")] = sol::overload(lua_config::set_bool, lua_config::set_color, lua_config::set_float, lua_config::set_multiselect, lua_config::set_int);
	config[XorStr("save")] = lua_config::config_save;
	config[XorStr("load")] = lua_config::config_load;
	config[XorStr("watermark_enabled")] = lua_config::watermark_enabled;
	config[XorStr("autopeek_enabled")] = lua_config::autopeek_enabled;
	config[XorStr("pingspike_enabled")] = lua_config::pingspike_enabled;
	config[XorStr("pingspike_value")] = lua_config::pingspike_value;
	config[XorStr("move_exploit_enabled")] = lua_config::move_exploit_enabled;
	config[XorStr("fakeduck_enabled")] = lua_config::fakeduck_enabled;
	config[XorStr("doubletap_enabled")] = lua_config::doubletap_enabled;
	config[XorStr("antiaim_manual_enabled")] = lua_config::antiaim_manual_enabled;
	config[XorStr("antiaim_base_yaw")] = lua_config::antiaim_base_yaw;
	config[XorStr("antiaim_base_yaw_set")] = lua_config::antiaim_base_yaw_set;
	//config[XorStr("antiaim_jitter_value")] = lua_config::antiaim_jitter_value;
	//config[XorStr("antiaim_jitter_set")] = lua_config::antiaim_jitter_set;
	config[XorStr("antiaim_fakewalk_enabled")] = lua_config::antiaim_fakewalk_enabled;
	config[XorStr("forcebaim_enabled")] = lua_config::forcebaim_enabled;
	config[XorStr("slide_walk_set")] = lua_config::slide_walk_set;
	config[XorStr("mindtrick_enabled")] = lua_config::mindtrick_enabled;
	config[XorStr("fakewalk_enabled")] = lua_config::fakewalk_enabled;
	config[XorStr("hitscanoverride_enabled")] = lua_config::hitscanoverride_enabled;
	config[XorStr("dmgoverride_enabled")] = lua_config::dmgoverride_enabled;
	config[XorStr("first_break_angle_set")] = lua_config::first_break_angle_set;
	config[XorStr("break_angle_set")] = lua_config::break_angle_set;
	config[XorStr("fakewalk_set_speed")] = lua_config::fakewalk_set_speed;

	auto cheat = this->lua.create_table();
	cheat[XorStr("set_event_callback")] = lua_cheat::set_event_callback;
	cheat[XorStr("run_script")] = lua_cheat::run_script;
	cheat[XorStr("reload_active_scripts")] = lua_cheat::reload_active_scripts;
	cheat[XorStr("get_button")] = lua_cheat::get_button;
	cheat[XorStr("set_button")] = lua_cheat::set_button;
	cheat[XorStr("username")] = lua_cheat::username;
	cheat[XorStr("dt_charged")] = lua_cheat::dt_charged;
	cheat[XorStr("create_interface")] = lua_cheat::create_interface;

	auto math = this->lua.create_table();
	math[XorStr("calc_angle")] = lua_math::calc_angle;
	math[XorStr("get_fov")] = lua_math::get_fov;

	auto surface = this->lua.create_table();
	surface[XorStr("create_font")] = lua_surface::create_font;
	surface[XorStr("draw_line")] = lua_surface::draw_line;
	surface[XorStr("draw_circle")] = lua_surface::draw_outlined_circle;
	surface[XorStr("draw_rect")] = lua_surface::draw_outlined_rect;
	surface[XorStr("draw_filled_rect")] = lua_surface::draw_filled_rect;
	surface[XorStr("draw_text")] = lua_surface::draw_text;
	surface[XorStr("get_text_size")] = lua_surface::get_text_size;
	surface[XorStr("set_color")] = lua_surface::set_color;
	surface[XorStr("set_text_color")] = lua_surface::set_text_color;
	surface[XorStr("set_text_font")] = lua_surface::set_text_font;
	surface[XorStr("set_text_pos")] = lua_surface::set_text_pos;

	auto modelinfo = this->lua.create_table();
	modelinfo[XorStr("get_model_index")] = lua_modelinfo::get_model_index;
	//modelinfo[XorStr("get_studio_model")] = lua_modelinfo::get_studio_model;

	auto debugoverlay = this->lua.create_table();
	debugoverlay[XorStr("add_box_overlay")] = lua_debugoverlay::add_box_overlay;
	debugoverlay[XorStr("add_line_overlay")] = lua_debugoverlay::add_line_overlay;
	debugoverlay[XorStr("add_text_overlay")] = lua_debugoverlay::add_text_overlay;

	auto engine = this->lua.create_table();
	engine[XorStr("execute_client_cmd")] = lua_engine::execute_client_cmd;
	engine[XorStr("get_player_for_user_id")] = lua_engine::get_player_for_user_id;
	engine[XorStr("get_local_player_index")] = lua_engine::get_local_player_index;
	engine[XorStr("get_max_clients")] = lua_engine::get_max_clients;
	engine[XorStr("get_net_channel_info")] = lua_engine::get_net_channel_info;
	engine[XorStr("get_screen_size")] = lua_engine::get_screen_size;
	engine[XorStr("get_view_angles")] = lua_engine::get_view_angles;
	engine[XorStr("set_view_angles")] = lua_engine::set_view_angles;
	engine[XorStr("is_connected")] = lua_engine::is_connected;
	engine[XorStr("is_in_game")] = lua_engine::is_in_game;
	engine[XorStr("get_player_info")] = lua_engine::get_player_info;

	auto entity_list = this->lua.create_table();
	entity_list[XorStr("get_client_entity")] = lua_entitylist::get_client_entity;
	entity_list[XorStr("get_highest_entity_index")] = lua_entitylist::get_highest_entity_index;
	entity_list[XorStr("get_local_player")] = lua_entitylist::get_local_player;

	auto utils = this->lua.create_table();
	utils[XorStr("get_player_data")] = lua_utils::get_player_data;
	utils[XorStr("clamp")] = lua_utils::clamp;
	utils[XorStr("find_signature")] = lua_utils::find_signature;
	utils[XorStr("apply_clan_tag")] = lua_utils::apply_clan_tag;
	//utils[XorStr("color")] = lua_utils::create_color; // not needed.......
	//utils[XorStr("floatcolor")] = lua_utils::create_floatcolor; // not needed.......
	utils[XorStr("color_rainbow")] = lua_utils::create_color_rainbow;
	utils[XorStr("lowerbody_yaw")] = lua_utils::lowerbody_yaw;
	//utils[XorStr("local_alive")] = lua_utils::local_alive;
	utils[XorStr("key_down")] = lua_utils::key_down;

	auto globals = this->lua.create_table();
	globals[XorStr("realtime")] = lua_globals::realtime;
	globals[XorStr("framecount")] = lua_globals::framecount;
	globals[XorStr("absoluteframetime")] = lua_globals::absoluteframetime;
	globals[XorStr("curtime")] = lua_globals::curtime;
	globals[XorStr("frametime")] = lua_globals::frametime;
	globals[XorStr("maxclients")] = lua_globals::maxclients;
	globals[XorStr("tickcount")] = lua_globals::tickcount;
	globals[XorStr("tickinterval")] = lua_globals::tickinterval;
	globals[XorStr("get_server_address")] = lua_globals::get_server_address;

	auto cvar = this->lua.create_table();
	cvar[XorStr("console_print")] = lua_cvar::console_print;
	cvar[XorStr("find_var")] = lua_cvar::find_var;
	cvar[XorStr("get_float")] = lua_cvar::get_float;
	cvar[XorStr("get_int")] = lua_cvar::get_int;
	cvar[XorStr("set_float")] = lua_cvar::set_float;
	cvar[XorStr("set_int")] = lua_cvar::set_int;
	cvar[XorStr("get_string")] = lua_cvar::get_string;
	cvar[XorStr("set_string")] = lua_cvar::set_string;
	cvar[XorStr("unlock_cvar")] = lua_cvar::unlock_cvar;
	
	auto render = this->lua.create_table();
	render[XorStr("draw_text")] = lua_render::draw_text;
	render[XorStr("text_size")] = lua_render::text_size;
	render[XorStr("draw_line")] = lua_render::draw_line;
	render[XorStr("draw_rect")] = lua_render::draw_rect;
	render[XorStr("draw_rect_filled")] = lua_render::draw_rect_filled;
	render[XorStr("draw_circle")] = lua_render::draw_circle;
	render[XorStr("draw_rect_outlined")] = lua_render::draw_rect_outlined;
	render[XorStr("draw_gradient")] = lua_render::draw_gradient;
	render[XorStr("draw_world_circle")] = lua_render::draw_world_circle;
	render[XorStr("draw_arc")] = lua_render::draw_arc;
	render[XorStr("draw_triangle")] = lua_render::draw_triangle;
	render[XorStr("world_to_screen")] = lua_render::world_to_screen;

	auto ui = this->lua.create_table();
	ui[XorStr("keybinds_pos")] = lua_ui::keybinds_pos;
	ui[XorStr("speclist_pos")] = lua_ui::speclist_pos;
	ui[XorStr("keybinds_open")] = lua_ui::keybinds_open;
	ui[XorStr("speclist_open")] = lua_ui::speclist_open;
	ui[XorStr("menu_open")] = lua_ui::menu_open;
	ui[XorStr("menu_pos")] = lua_ui::menu_pos;
	ui[XorStr("mouse_pos")] = lua_ui::mouse_pos;
	ui[XorStr("new_checkbox")] = lua_ui::add_check_box;
	ui[XorStr("new_colorpicker")] = lua_ui::add_color_picker;
	ui[XorStr("new_slider")] = lua_ui::add_slider;
	ui[XorStr("new_text")] = lua_ui::add_text;
	ui[XorStr("new_button")] = lua_ui::add_button;

	auto clientstate = this->lua.create_table();
	clientstate[XorStr("chokedcommands")] = lua_clientstate::chokedcommands;

	auto effects = this->lua.create_table();
	effects[XorStr("sparks")] = lua_effects::sparks;
	effects[XorStr("dust")] = lua_effects::dust;
	effects[XorStr("energysplash")] = lua_effects::EnergySplash;

	auto http = this->lua.create_table();
	http[XorStr("get")] = lua_http::get;
	http[XorStr("post")] = lua_http::post;

	auto file = this->lua.create_table();
	file[XorStr("append")] = lua_file::append;
	file[XorStr("write")] = lua_file::write;
	file[XorStr("read")] = lua_file::read;

	auto exploits = this->lua.create_table();
	exploits[XorStr("force_charge")] = lua_exploits::force_charge;
	exploits[XorStr("force_uncharge")] = lua_exploits::force_uncharge;
	exploits[XorStr("force_speed")] = lua_exploits::force_speed;
	exploits[XorStr("force_chargedelay")] = lua_exploits::force_chargedelay;
	exploits[XorStr("force_unchargestate")] = lua_exploits::force_unchargestate;

	this->lua[XorStr("event")] = events;
	this->lua[XorStr("config")] = config;
	this->lua[XorStr("cheat")] = cheat;
	this->lua[XorStr("math2")] = math;
	this->lua[XorStr("surface")] = surface;
	this->lua[XorStr("modelinfo")] = modelinfo;
	this->lua[XorStr("debugoverlay")] = debugoverlay;
	this->lua[XorStr("engine")] = engine;
	this->lua[XorStr("entity_list")] = entity_list;
	this->lua[XorStr("utils")] = utils;
	this->lua[XorStr("globals")] = globals;
	this->lua[XorStr("cvar")] = cvar;
	this->lua[XorStr("render")] = render;
	this->lua[XorStr("ui")] = ui;
	this->lua[XorStr("clientstate")] = clientstate;
	this->lua[XorStr("effects")] = effects;
	this->lua[XorStr("http")] = http;
	this->lua[XorStr("file")] = file;
	this->lua[XorStr("exploits")] = exploits;

	this->refresh_scripts();
	this->load_script(this->get_script_id("autorun.lua"));

	if (g_Vars.globals.c_username.empty()) { // another random ass check lol
		LI_FN(exit)(0);
		return false;
	}

	return true;
}

void c_lua::load_script(int id) {
	if (id == -1)
		return;

	if (this->loaded.at(id))
		return;

	auto path = this->get_script_path(id);
	if (path == XorStr(""))
		return;

	auto error_load = false;
	lua.script_file(path,
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = XorStr("Lua error: ") + (std::string)error.what();
				Interfaces::m_pSurface->PlaySound_(XorStr("UI/weapon_cant_buy.wav"));
				engine_console_error(log);
				error_load = true;

			}

			return result;
		}
	);

	this->loaded.at(id) = true;

	if (error_load)
	{
		this->unload_script(id);
		return;
	}
}

void c_lua::unload_script(int id) {
	if (id == -1)
		return;

	if (!this->loaded.at(id))
		return;

	items.at(id).clear();

	g_luahookmanager.unregister_hooks(id);
	g_luagameeventmanager.unregister_gameevents(id);
	this->loaded.at(id) = false;
}

void c_lua::reload_all_scripts() {
	for (auto s : this->scripts) {
		if (this->loaded.at(this->get_script_id(s))) {
			this->unload_script(this->get_script_id(s));
			this->load_script(this->get_script_id(s));
		}
	}
}

void c_lua::unload_all_scripts() {
	for (auto s : this->scripts)
		if (this->loaded.at(this->get_script_id(s)))
			this->unload_script(this->get_script_id(s));
}

void c_lua::refresh_scripts() {
	auto oldLoaded = this->loaded;
	auto oldScripts = this->scripts;

	this->loaded.clear();
	this->pathes.clear();
	this->scripts.clear();

	std::filesystem::path full_path(std::filesystem::current_path());
	std::wstring str = full_path.wstring() + XorStr(L"\\vader.tech\\scripts");
	std::wstring str2 = full_path.wstring() + XorStr(L"\\vader.tech\\scripts\\autorun.lua");

	CreateDirectoryW(str.c_str(), nullptr);
	CreateFileW(str2.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

	for (auto& entry : std::filesystem::directory_iterator(str)) {
		if (entry.path().extension() == XorStr(".lua") || entry.path().extension() == XorStr(".luac") || entry.path().extension() == XorStr(".ljbc")) {

			//if (entry.path().stem().string().find('.') != std::string::npos) {
			//	engine_console_error(XorStr("Invalid lua found! make sure your lua does not contain '.' in its name."));
			//	engine_console_error(entry.path().stem().string());
			//	continue;
			//}

			auto path = entry.path();
			auto filename = path.filename().string();

			bool didPut = false;
			for (size_t i = 0; i < oldScripts.size(); i++) {
				if (filename == oldScripts.at(i)) {
					this->loaded.push_back(oldLoaded.at(i));
					didPut = true;
				}
			}

			if (!didPut)
				this->loaded.push_back(false);

			this->pathes.push_back(path);
			this->scripts.push_back(filename);

			this->items.emplace_back(std::vector <std::pair <std::string, menu_item>>());
		}
	}
}

int c_lua::get_script_id(std::string name) {
	for (size_t i = 0; i < this->scripts.size(); i++) {
		if (this->scripts.at(i) == name)
			return i;
	}

	return -1;
}

int c_lua::get_script_id_by_path(std::string path) {
	for (size_t i = 0; i < this->pathes.size(); i++) {
		if (this->pathes.at(i).string() == path)
			return i;
	}

	return -1;
}

std::string c_lua::get_script_path(std::string name) {
	return this->get_script_path(this->get_script_id(name));
}

std::string c_lua::get_script_path(int id) {
	if (id == -1)
		return  "";

	return this->pathes.at(id).string();
}
