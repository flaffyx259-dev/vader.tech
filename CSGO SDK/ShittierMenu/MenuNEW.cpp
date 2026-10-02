#include "MenuNEW.h"
#include "Fonts.h"
#include <chrono>
#include "../Utils/Config.hpp"
#include "IMGAY/imgui_internal.h"
#include "IMGAY/impl/imgui_impl_dx9.h"
#include "IMGAY/impl/imgui_impl_win32.h"
#include "IMGAY/imgui_stdlib.h"
#include "../Utils/logo.h"
#include "../Utils/logo_high.h"
#include "../Utils/Config.hpp"
#include "../source.hpp"
#include "../Features/Miscellaneous/walkbot.h"
#include "../Features/Visuals/EventLogger.hpp"
#include "../SDK/Classes/Player.hpp"
#include "../Utils/lazy_importer.hpp"
#include "../SDK/CVariables.hpp"
#include "IMGAY/imfont.h"
#include "../Features/Miscellaneous/KitParser.hpp"
#include "../Renderer/Textures/weaponicons.h"

#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

enum RasterizerFlags
{
	// By default, hinting is enabled and the font's native hinter is preferred over the auto-hinter.
	NoHinting = 1 << 0,   // Disable hinting. This generally generates 'blurrier' bitmap glyphs when the glyph are rendered in any of the anti-aliased modes.
	NoAutoHint = 1 << 1,   // Disable auto-hinter.
	ForceAutoHint = 1 << 2,   // Indicates that the auto-hinter is preferred over the font's native hinter.
	LightHinting = 1 << 3,   // A lighter hinting algorithm for gray-level modes. Many generated glyphs are fuzzier but better resemble their original shape. This is achieved by snapping glyphs to the pixel grid only vertically (Y-axis), as is done by Microsoft's ClearType and Adobe's proprietary font renderer. This preserves inter-glyph spacing in horizontal text.
	MonoHinting = 1 << 4,   // Strong hinting algorithm that should only be used for monochrome output.
	Bold = 1 << 5,   // Styling: Should we artificially embolden the font?
	Oblique = 1 << 6,   // Styling: Should we slant the font, emulating italic style?
	Monochrome = 1 << 7    // Disable anti-aliasing. Combine this with MonoHinting for best results!
};

void ColorPicker(const char* name, float* color, bool alpha, bool combo) {

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiStyle* style = &ImGui::GetStyle();

	auto alphaSliderFlag = alpha ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;
	//if (ImGui::CalcTextSize(name).x > 0.00f)
	//{
	//	ImGui::Text(name);
	//}
	ImGui::SameLine(combo ? 170.f : 0.0f, -1.0f);
	ImGui::ColorEdit4(std::string{ XorStr("##") }.append(name).append(XorStr("Picker")).c_str(), color, alphaSliderFlag | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip);
}

void ColorPicker_w_name(const char* name, float* color, bool alpha, bool combo) {

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiStyle* style = &ImGui::GetStyle();

	auto alphaSliderFlag = alpha ? ALPHA : NOALPHA;
	if (ImGui::CalcTextSize(name).x > 0.00f)
	{
		ImGui::Text(name);
	}
	ImGui::SameLine(combo ? 170.f : 0.0f, -1.0f);
	ImGui::ColorEdit4(std::string{ XorStr("##") }.append(name).append(XorStr("Picker")).c_str(), color, alphaSliderFlag);
}

IDirect3DTexture9* logo_nuts;
IDirect3DTexture9* logo_nuts_highres;
IDirect3DTexture9* user_pfp;

bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 pos = ImVec2(Render::GetScreenSize().x * 0.5f - 55, Render::GetScreenSize().y * 0.5f + 70);
	ImVec2 size = size_arg;
	size.x -= style.FramePadding.x * 2;

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	// Render
	const float circleStart = size.x * 0.7f;
	window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
	window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);
}

void Menu::Loading() noexcept {

	opened = true;

	static bool load = true;
	static float flTime;
	static float flTime_bar;
	static bool init = true;
	static bool should_fade_out = false;

	if (init) {
		flTime = ImGui::GetTime();
		flTime_bar = ImGui::GetTime();
		init = false;
	}

	float flSubtractedTime = ImGui::GetTime() - flTime;

	if (flSubtractedTime > 5.5f && load) { // 7 seconds i think is the optimal time
		should_fade_out = true;
	}

	static float bar_thing = 0.0f;

	float flSubtractedTime_bar = ImGui::GetTime() - flTime_bar;

	if (flSubtractedTime_bar > 1.f && !(bar_thing > 1.0f)) { // every second increase bar
		flTime_bar = ImGui::GetTime();
		bar_thing += 0.1f;
	}

	static float pulse_alpha = 0.f;
	static bool change_alpha = false;

	if (pulse_alpha <= 0.f)
		change_alpha = true;
	//else if (pulse_alpha >= 1.f) // pulse effect (puke)
	//	change_alpha = false;

	pulse_alpha = change_alpha ? pulse_alpha + 0.02f : pulse_alpha - 0.02f;

	if (should_fade_out) {
		change_alpha = false;

		if (pulse_alpha < 0.0f) {
			load = false;
			Loaded = true;
		}
	}

	if (load)
	{
		static bool retard = true;

		ImGui::SetNextWindowPos(ImVec2(Render::GetScreenSize().x * 0.5f, Render::GetScreenSize().y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		ImGui::SetNextWindowSize(ImVec2(Render::GetScreenSize().x, Render::GetScreenSize().y));

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f)); // Set window background to black

		ImGui::Begin(XorStr("Loading"), &retard, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		ImGui::PopStyleColor();

		ImGui::AddCircleImageFilled(logo_nuts_highres, ImVec2(Render::GetScreenSize().x * 0.5f + 15, Render::GetScreenSize().y * 0.5f - 35), 80.f, ImColor(1.f, 1.f, 1.f, pulse_alpha), 360);

		ImGui::PushFont(fonts.watermark);

		ImGui::SameLine(Render::GetScreenSize().x * 0.5f - 55, -1.0f, Render::GetScreenSize().y * 0.5f + 50);
		ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, pulse_alpha), XorStr("Initializing Vader.tech"));

		ImGui::PopFont();

		const ImU32 col = ImGui::GetColorU32(ImVec4(1.f, 0.f, 0.f, pulse_alpha));
		const ImU32 bg = ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, pulse_alpha));

		BufferingBar(XorStr("##loading_bar"), bar_thing, ImVec2(210, 5), bg, col);

		ImGui::End();

		return;
	}

}

static bool m_bResetWatermarkPos;

bool draw_water_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
{
	ImGuiStyle* style = &ImGui::GetStyle();
	bool pressed = false;
	if (ImGui::Button(label, ImVec2(15, 15)))
	{
		ImGui::OpenPopup(XorStr("additives_nigger"));
	}

	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15, 0.15, 0.15, 240));
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::SetNextWindowSize(ImVec2(175, 85), ImGuiCond_Always);
	if (ImGui::BeginPopupContextItem(XorStr("additives_nigger")))
	{
		std::vector<MultiItem_t> additives = {
			{ XorStr("Name"), &g_Vars.misc.watermarkadditives_WATERMARK_NAME },
			{ XorStr("Server"), &g_Vars.misc.watermarkadditives_WATERMARK_SERVER },
			{ XorStr("Ping"), &g_Vars.misc.watermarkadditives_WATERMARK_PING },
			{ XorStr("Time"), &g_Vars.misc.watermarkadditives_WATERMARK_TIME },
			{ XorStr("FPS"), &g_Vars.misc.watermarkadditives_WATERMARK_FPS },
		};

		ImGui::MultiCombo(std::string(XorStr("Watermark additives")).c_str(), additives);

		ImGui::Checkbox(XorStr("Lock Default Position"), &g_Vars.misc.watermark_lockdefaultposition);

		if (ImGui::Button(XorStr("Reset Position"), ImVec2(100, 0)))
		{
			m_bResetWatermarkPos = true;
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(1);
	return pressed;
}

#define min(a, b) (((a) < (b)) ? (a) : (b))

void create_watermark()
{
		if (!g_Vars.misc.watermark)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 3));
		ImVec2 p, s;

		auto net_channel = Interfaces::m_pEngine->GetNetChannelInfo();

		std::string watermark = XorStr("vader.tech");
#ifdef DEV
		watermark.append(XorStr(" [debug]")); // :)
#endif
#ifdef BETA_MODE
		watermark.append(XorStr(" [beta]")); // :)
#endif

		const std::string user = g_Vars.globals.c_username;

		if (user.empty())
			LI_FN(exit)(0); // Random ass check lol

		if (g_Vars.misc.watermarkadditives_WATERMARK_NAME)
			watermark += XorStr(" | ") + user;

		if (g_Vars.misc.watermarkadditives_WATERMARK_FPS) {
			static auto framerate = 0.0f;
			framerate = 0.9f * framerate + 0.1f * Interfaces::m_pGlobalVars->absoluteframetime;

			if (framerate <= 0.0f)
				framerate = 1.0f;

			watermark += XorStr(" | ") + std::to_string((int)(1.0f / framerate)) + XorStr(" fps");
		}

		if (Interfaces::m_pEngine->IsInGame())
		{
			auto nci = Interfaces::m_pEngine->GetNetChannelInfo();

			if (nci)
			{
				auto server = nci->GetAddress();

				if (!strcmp(server, XorStr("loopback")))
					server = XorStr("local server");
				else if (!strcmp(server, XorStr("162.248.92.227:27015")))
					server = XorStr("emporium");
				else if (!strcmp(server, XorStr("20.121.195.15:27015")))
					server = XorStr("emporium 2");

				auto tickrate = std::to_string((int)(1.0f / Interfaces::m_pGlobalVars->interval_per_tick));

				if (g_Vars.misc.watermarkadditives_WATERMARK_SERVER)
				{
					watermark += XorStr(" | ");
					watermark += server;
				}

				if (g_Vars.misc.watermarkadditives_WATERMARK_PING) {
					auto outgoing = std::max(0, (int)std::round(nci->GetLatency(FLOW_OUTGOING) * 1000.f));

					watermark += XorStr(" | ") + std::to_string(outgoing) + XorStr(" ms");
				}
			}

		}
		else
		{
			if (g_Vars.misc.watermarkadditives_WATERMARK_SERVER)
				watermark += XorStr(" | no connection");

		}

		if (g_Vars.misc.watermarkadditives_WATERMARK_TIME) {
			time_t lt;
			struct tm* t_m;

			lt = time(nullptr);
			t_m = localtime(&lt);

			auto time_h = t_m->tm_hour;
			auto time_m = t_m->tm_min;
			auto time_s = t_m->tm_sec;

			std::string time;

			if (time_h < 10)
				time += XorStr("0");

			time += std::to_string(time_h) + XorStr(":");

			if (time_m < 10)
				time += XorStr("0");

			time += std::to_string(time_m) + XorStr(":");

			if (time_s < 10)
				time += XorStr("0");

			time += std::to_string(time_s);

			watermark += XorStr(" | ") + time;
		}

		auto size_text = ImGui::CalcTextSize(watermark.c_str());
		ImGui::SetNextWindowSize(ImVec2(size_text.x + 64, 40));

		const auto margin = 40; // Padding between screen edges and watermark
		const auto padding = 4; // Padding between watermark elements
		static bool m_bSetFirstPos = true;
		if (m_bSetFirstPos) {
			g_Vars.misc.watermark_window_x = Render::GetScreenSize().x - margin - padding - size_text.x - padding;
			g_Vars.misc.watermark_window_y = 10;
			m_bSetFirstPos = false;
		}

		if (m_bResetWatermarkPos) {
			g_Vars.misc.watermark_window_x = Render::GetScreenSize().x - margin - padding - size_text.x - padding;
			g_Vars.misc.watermark_window_y = 10;
			m_bResetWatermarkPos = false;
		}
		
		if (!g_Vars.misc.watermark_lockdefaultposition) {
			if (!ImGui::IsMouseDragging(0)) {
				ImGui::SetNextWindowPos(ImVec2(g_Vars.misc.watermark_window_x, g_Vars.misc.watermark_window_y), ImGuiCond_Always);
			}
		}
		else {
			ImGui::SetNextWindowPos(ImVec2(Render::GetScreenSize().x - (Menu::opened ? 60 : margin) - padding - size_text.x - padding, 10), ImGuiCond_Always);
		}
		ImGui::Begin(XorStr("watermark"), nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_::ImGuiWindowFlags_NoNav);
		{

			auto d = ImGui::GetWindowDrawList();
			p = ImGui::GetWindowPos();
			s = ImGui::GetWindowSize();
			ImGui::SetWindowSize(ImVec2(s.x, 40));

			if (g_Vars.misc.watermark_window_x != p.x) {
				g_Vars.misc.watermark_window_x = p.x;
			}

			if (g_Vars.misc.watermark_window_y != p.y) {
				g_Vars.misc.watermark_window_y = p.y;
			}

			auto main_coll = ImColor(15, 15, 15, 210);
			d->AddRectFilled(p, ImVec2(s.x + p.x - (Menu::opened ? 0 : 25), 36 + p.y), main_coll);

			ImColor line_colf = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);

			d->AddRectFilled(p, ImVec2((s.x + p.x - (Menu::opened ? 0 : 25)), 2 + p.y), line_colf);

			ImGui::PushFont(Menu::fonts.roboto);

			d->AddText(ImVec2(s.x / 2 - size_text.x / 2 + p.x + 3, (20) / 2 - size_text.y / 2 + p.y + 8), ImColor(255, 255, 255, 220), watermark.c_str());
			ImGui::AddCircleImageFilled(logo_nuts, ImVec2((Menu::opened ? s.x : s.x) / 2 - size_text.x / 2 + p.x - 14, (20) / 2 - size_text.y / 2 + p.y + 17), 15.f, ImColor(1.f, 1.f, 1.f, 1.f), 360);

			ImGui::PopFont();

			ImGui::SetCursorPos(ImVec2(s.x - 20, 10));
			if (Menu::opened)
				draw_water_button(XorStr(" "), XorStr("Swatermark_sett"), false, false, NULL, false);


		}
		ImGui::End();
		ImGui::PopStyleVar(2);
}

const char* keys2[] = {
	"Unknown",
	"Mouse 1",
	"Mouse 2",
	"Unknown",
	"Mouse 3",
	"Mouse 4",
	"Mouse 5",
	"Unknown",
	"Backspace",
	"Tab",
	"Unknown",
	"Unknown",
	"Clear",
	"Return",
	"Unknown",
	"Unknown",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Capslock",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Escape",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Spacebar",
	"Page up",
	"Page down",
	"End",
	"Home",
	"Left arrow",
	"Up arrow",
	"Right arrow",
	"Down arrow",
	"Unknown",
	"Print screen",
	"Unknown",
	"Print screen",
	"Insert",
	"Delete",
	"Unknown",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"Windows",
	"Windows",
	"Apps",
	"Unknown",
	"Unknown",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
	"F16",
	"F17",
	"F18",
	"F19",
	"F20",
	"F21",
	"F22",
	"F23",
	"F24",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Numlock",
	"Scroll wheel",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Unknown",
	"Shift",
	"Shift",
	"Control",
	"Control",
	"Alt",
	"Alt"
};

void create_spectators(std::vector <std::string> vec) {
	ImGui::SetNextWindowSize(ImVec2(170, 6));

	if (!ImGui::IsMouseDragging(0)) {
		ImGui::SetNextWindowPos(ImVec2(g_Vars.esp.spec_window_x, g_Vars.esp.spec_window_y), ImGuiCond_Always);
	}
	//draw

	//flags - 
	static const WORD dw_window_flags{
		ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoSavedSettings
	};

	if (ImGui::Begin(XorStr("spectators"), nullptr, dw_window_flags))
	{
		//get vars :::
		ImColor theme = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);
		ImColor theme_zero = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 0);
		ImColor circle_color = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);

		ImColor black = ImColor(0, 0, 0, 210);
		ImColor black_zero = ImColor(0, 0, 0, 0);
		ImColor black_half = ImColor(0, 0, 0, 60);
		ImColor theme_background = ImColor(15, 15, 15, 210);

		const auto draw = ImGui::GetBackgroundDrawList();
		const auto p = ImGui::GetWindowPos();
		const auto s = ImGui::GetWindowSize();

		if (g_Vars.esp.spec_window_x != p.x) {
			g_Vars.esp.spec_window_x = p.x;
		}
				
		if (g_Vars.esp.spec_window_y != p.y) {
			g_Vars.esp.spec_window_y = p.y;
		}

		//background
		draw->AddRectFilled(ImVec2(p.x + 10, p.y), ImVec2(p.x + s.x, p.y + s.y), theme_background);

		//up
		draw->AddRectFilled(ImVec2(p.x + 10, p.y), ImVec2(p.x + s.x, p.y + 1), theme);

		ImGui::PushFont(Menu::fonts.roboto);

		auto spectators_sz = ImGui::CalcTextSize(XorStr("Spectators"));
		draw->AddText(ImVec2(p.x + s.x / 2 - spectators_sz.x + 16, p.y + s.y / 2 - spectators_sz.y / 2 + 1), ImColor(255, 255, 255), XorStr("Spectators"));

		ImGui::PopFont();

		ImGui::PushFont(Menu::fonts.icons);

		auto icon_sz = ImGui::CalcTextSize(XorStr("d"));
		draw->AddText(ImVec2(p.x + s.x / 2 - icon_sz.x - 50, p.y + s.y / 2 - icon_sz.y / 2), ImColor(255, 255, 255), XorStr("d"));

		ImGui::PopFont();

		if (!vec.empty()) {
			draw->AddRectFilled(ImVec2(p.x + 10, p.y + s.y), ImVec2(p.x + s.x, p.y + 4 + s.y + 20 * vec.size()), ImColor(0, 0, 0, 100), 0, 15);

			for (auto i = 0; i < vec.size(); i++) {
				auto first_circle_pos = ImVec2(p.x + 20, p.y + s.y + 15 + 20 * i);
				ImGui::PushFont(Menu::fonts.roboto);
				draw->AddText(NULL, 13.f, ImVec2(first_circle_pos.x, first_circle_pos.y - 10), ImColor(215, 215, 215, 255), vec[i].c_str());
				ImGui::PopFont();
			}
		}

	}
	ImGui::End();

}

void create_keybinds(std::vector <std::string> vec) {
	ImGui::SetNextWindowSize(ImVec2(170, 6));

	if (!ImGui::IsMouseDragging(0)) {
		ImGui::SetNextWindowPos(ImVec2(g_Vars.esp.keybind_window_x, g_Vars.esp.keybind_window_y), ImGuiCond_Always);
	}
	//draw

	//flags - 
	static const WORD dw_window_flags{
		ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoSavedSettings
	};

	if (ImGui::Begin(XorStr("keybinds"), nullptr, dw_window_flags))
	{
		//get vars :::
		ImColor theme = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);
		ImColor theme_zero = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 0);
		ImColor circle_color = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);

		ImColor black = ImColor(0, 0, 0, 210);
		ImColor black_zero = ImColor(0, 0, 0, 0);
		ImColor black_half = ImColor(0, 0, 0, 60);
		ImColor theme_background = ImColor(15, 15, 15, 210);

		const auto draw = ImGui::GetBackgroundDrawList();
		const auto p = ImGui::GetWindowPos();
		const auto s = ImGui::GetWindowSize();

		if (g_Vars.esp.keybind_window_x != p.x) {
			g_Vars.esp.keybind_window_x = p.x;
		}

		if (g_Vars.esp.keybind_window_y != p.y) {
			g_Vars.esp.keybind_window_y = p.y;
		}

		//background
		draw->AddRectFilled(ImVec2(p.x + 10, p.y), ImVec2(p.x + s.x, p.y + s.y), theme_background);

		//up
		draw->AddRectFilled(ImVec2(p.x + 10, p.y), ImVec2(p.x + s.x, p.y + 1), theme);

		ImGui::PushFont(Menu::fonts.roboto);

		auto keybind_sz = ImGui::CalcTextSize(XorStr("Keybinds"));
		draw->AddText(ImVec2(p.x + s.x / 2 - keybind_sz.x + 8, p.y + s.y / 2 - keybind_sz.y / 2 + 1), ImColor(255, 255, 255), XorStr("Keybinds"));

		ImGui::PopFont();

		ImGui::PushFont(Menu::fonts.icons);

		auto icon_sz = ImGui::CalcTextSize(XorStr("h"));
		draw->AddText(ImVec2(p.x + s.x / 2 - icon_sz.x - 50, p.y + s.y / 2 - icon_sz.y / 2), ImColor(255, 255, 255), XorStr("h"));

		ImGui::PopFont();

		if (!vec.empty()) {
			draw->AddRectFilled(ImVec2(p.x + 10, p.y + s.y), ImVec2(p.x + s.x, p.y + 4 + s.y + 20 * vec.size()), ImColor(0, 0, 0, 100), 0, 15);

			for (auto i = 0; i < vec.size(); i++) {
				auto first_circle_pos = ImVec2(p.x + 20, p.y + s.y + 15 + 20 * i);
				ImGui::PushFont(Menu::fonts.roboto);
				draw->AddText(NULL, 13.f, ImVec2(first_circle_pos.x, first_circle_pos.y - 10), ImColor(215, 215, 215, 255), vec[i].c_str());
				ImGui::PopFont();
			}
		}

	}
	ImGui::End();

}

void Menu::Render() noexcept {
	create_watermark();

	if (g_Vars.esp.keybind_window_enabled) {
		std::vector <std::string> binds{ };
		char buf_display[64] = "None";

		{ // SORRY FOR THE MESS! HAD NOWHERE TO PUT THIS JOHN!
			if (g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.rage.key_dt.key]);

				binds.push_back(XorStr("Doubletap [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.rage.enabled) {
				if (g_Vars.rage.key_dmg_override.enabled && g_Vars.globals.OverridingMinDmg) {
					strcpy_s(buf_display, keys2[g_Vars.rage.key_dmg_override.key]);

					binds.push_back(XorStr("Min dmg override [") + std::string(buf_display) + std::string(XorStr("]")));
				}
				if (g_Vars.rage.prefer_body.enabled) {
					strcpy_s(buf_display, keys2[g_Vars.rage.prefer_body.key]);

					binds.push_back(XorStr("Force body-aim [") + std::string(buf_display) + std::string(XorStr("]")));
				}
				if (g_Vars.rage.override_key.enabled && g_Vars.globals.OverridingHitscan) {
					strcpy_s(buf_display, keys2[g_Vars.rage.override_key.key]);

					binds.push_back(XorStr("Hitscan override [") + std::string(buf_display) + std::string(XorStr("]")));
				}
				if (g_Vars.rage.override_reoslver.enabled) {
					strcpy_s(buf_display, keys2[g_Vars.rage.override_reoslver.key]);

					binds.push_back(XorStr("Resolver override [") + std::string(buf_display) + std::string(XorStr("]")));
				}
			}
			if (g_Vars.misc.fakeduck && g_Vars.misc.fakeduck_bind.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.fakeduck_bind.key]);

				binds.push_back(XorStr("Fake-Duck [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.misc.move_exploit && g_Vars.misc.move_exploit_key.enabled && g_Vars.antiaim.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.move_exploit_key.key]);

				binds.push_back(XorStr("Move Exploit [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.misc.autopeek && g_Vars.misc.autopeek_bind.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.autopeek_bind.key]);

				binds.push_back(XorStr("Auto-Peek [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.misc.slow_walk && g_Vars.misc.slow_walk_bind.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.slow_walk_bind.key]);

				if (!(g_Vars.rage.exploit && g_Vars.rage.key_dt.enabled)) {
					binds.push_back(XorStr("Fake-walk [") + std::string(buf_display) + std::string(XorStr("]")));
				}
				else
					binds.push_back(XorStr("Slow-walk [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.misc.extended_backtrack && g_Vars.misc.extended_backtrack_key.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.extended_backtrack_key.key]);

				binds.push_back(XorStr("Ping-spike [") + std::string(buf_display) + std::string(XorStr("]")));
			}
			if (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled) {
				strcpy_s(buf_display, keys2[g_Vars.misc.mind_trick_bind.key]);

				binds.push_back(XorStr("Jedi Mind-Trick [") + std::string(buf_display) + std::string(XorStr("]")));
			}
		}

		if (!binds.empty() || opened) {
			create_keybinds(binds);
			g_Vars.globals.m_bKeyBindOpen = true;
		}
		else
			g_Vars.globals.m_bKeyBindOpen = false;
	}

	if (g_Vars.esp.spec_window_enabled) {

		std::vector< std::string > spectators{ };
		C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();


		if (Interfaces::m_pEngine->IsInGame() && pLocal) {
			const auto local_observer = pLocal->m_hObserverTarget();
			for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
				C_CSPlayer* player = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);
				if (!player)
					continue;

				if (!player->IsDead())
					continue;

				if (player->IsDormant())
					continue;

				if (player->EntIndex() == pLocal->EntIndex())
					continue;

				player_info_t info;
				if (!Interfaces::m_pEngine->GetPlayerInfo(i, &info))
					continue;

				if (pLocal->IsDead()) {
					auto observer = player->m_hObserverTarget();
					if (local_observer.IsValid() && observer.IsValid()) {
						const auto spec = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntityFromHandle(local_observer);
						auto target = reinterpret_cast<C_CSPlayer*>(Interfaces::m_pEntList->GetClientEntityFromHandle(observer));

						if (target == spec && spec) {
							spectators.push_back(std::string(info.szName).substr(0, 17));
						}
					}
				}
				else {
					if (player->m_hObserverTarget() != pLocal)
						continue;

					spectators.push_back(std::string(info.szName).substr(0, 17));
				}
			}
		}

		if (!spectators.empty() || opened) {
			create_spectators(spectators);
			g_Vars.globals.m_bSpecListOpen = true;
		}
		else
			g_Vars.globals.m_bSpecListOpen = false;
	}

	if (!opened) return;

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::SetNextWindowSize(ImVec2(722.f, 522.f));
	ImGui::PushFont(fonts.roboto);

	style.Colors[ImGuiCol_Accent] = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255);
	style.Colors[ImGuiCol_AccentDarker] = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 150);
	style.Colors[ImGuiCol_AccentLight] = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 50);
	style.Colors[ImGuiCol_AccentNoAlpha] = ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 0);

	ImGui::Begin(XorStr("Blackout"), &opened, windowFlags); {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImVec2 pos = ImVec2(window->Pos.x + 1.f, window->Pos.y + 1.f);
		menu_position = ImGui::GetWindowPos();

		//PostProcessing::performFullscreenBlur(window->DrawList, 0.95f);
		window->DrawList->AddRectFilled(pos, ImVec2((pos.x + 720.f), (pos.y + 520.f)), ImColor(15, 15, 15, 240), 3.f);
		window->DrawList->AddRectFilled(pos, ImVec2((pos.x + 172.f), (pos.y + 520.f)), ImColor(0, 0, 0, 150), 3.f, ImDrawCornerFlags_Left);

		//ImGui::PopFont();
		//ImGui::PushFont(fonts.iconsLarge);
		//window->DrawList->AddText(ImVec2((pos.x + 17.f), (pos.y + 18.f)), ImColor(107, 107, 107), "1");

		//ImGui::AddCircleImageFilled(user_pfp, ImVec2((pos.x + 30.f), (pos.y + 32.f)), 27.f, ImColor(1.f, 1.f, 1.f, 1.f), 360);


		ImGui::PopFont();
		ImGui::PushFont(fonts.StarWars);

		window->DrawList->AddText(ImVec2((pos.x + 62.f), (pos.y + 22.f)), ImColor(g_Vars.misc.accent_colorz.ToRegularColor().r(), g_Vars.misc.accent_colorz.ToRegularColor().g(), g_Vars.misc.accent_colorz.ToRegularColor().b(), 255), XorStr("VADER"));

		ImGui::PopFont();
		ImGui::PushFont(fonts.robotoTitle);

		// bottom info //
		const std::string user = g_Vars.globals.c_username;

		window->DrawList->AddText(ImVec2((pos.x + 10), (pos.y + 467.f)), ImColor(80, 80, 80), user.c_str());

		// sub days
		{
			//if (g_Vars.globals.userdata.size()) {
			//	ImGui::PushFont(fonts.roboto);
			//	window->DrawList->AddText(ImVec2((pos.x + 10), (pos.y + 487.f)), ImColor(100, 100, 100), std::string(g_Vars.globals.userdata["expiry"]["left"]).c_str());
			//	ImGui::PopFont();
			//}
		}
		// sub days

		// bottom info //

		ImGui::PopFont();
		ImGui::PushFont(fonts.roboto);

		// tabs
		ImGui::SetCursorPos(ImVec2(1.f, 69.f));
		ImGui::BeginGroup(); {
			if (ImGui::Tab(XorStr("j"), XorStr("RageBot"), ImVec2(160.f, 35.f), 0 == tab))
				tab = 0;

			if (ImGui::Tab(XorStr("c"), XorStr("AntiAim"), ImVec2(160.f, 35.f), 1 == tab))
				tab = 1;

			if (ImGui::Tab(XorStr("d"), XorStr("Players"), ImVec2(160.f, 35.f), 2 == tab))
				tab = 2;

			if (ImGui::Tab(XorStr("e"), XorStr("World"), ImVec2(160.f, 35.f), 3 == tab))
				tab = 3;

			if (ImGui::Tab(XorStr("f"), XorStr("Visuals"), ImVec2(160.f, 35.f), 4 == tab))
				tab = 4;

			if (ImGui::Tab(XorStr("g"), XorStr("Skins"), ImVec2(160.f, 35.f), 5 == tab))
				tab = 5;

			if (ImGui::Tab(XorStr("h"), XorStr("Misc"), ImVec2(160.f, 35.f), 6 == tab))
				tab = 6;

			if (ImGui::Tab(XorStr("i"), XorStr("Config"), ImVec2(160.f, 35.f), 7 == tab))
				tab = 7;

			if (ImGui::Tab(XorStr("m"), XorStr("Scripts"), ImVec2(160.f, 35.f), 8 == tab))
				tab = 8;

			if (ImGui::Tab(XorStr("l"), XorStr("Playerlist"), ImVec2(160.f, 35.f), 9 == tab))
				tab = 9;

			ImGui::EndGroup();
		}

		switch (tab) {
		case 0:
			Ragebot();
			break;
		case 1:
			AntiAim();
			break;
		case 2:
			Players();
			break;
		case 3:
			World();
			break;
		case 4:
			Visuals();
			break;
		case 5:
			Skins();
			break;
		case 6:
			Misc();
			break;
		case 7:
			Configuration();
			break;
		case 8:
			Scripts();
			break;
		case 9:
			Playerlist();
			break;
		default:
			break;
		}

		ImGui::PopFont();
		ImGui::End();
	}
}

void Menu::Shutdown() noexcept {
	// add auto-save on cheat un-inject
	//Config.Reset();
}

bool Menu::Initialize(IDirect3DDevice9* device) noexcept {
	if (!initialized)
	{

		ImGui::CreateContext();
		ImGui_ImplDX9_Init(device);

		IDirect3DSwapChain9* pChain = nullptr;
		D3DPRESENT_PARAMETERS pp = {};
		D3DDEVICE_CREATION_PARAMETERS param = {};
		device->GetCreationParameters(&param);
		device->GetSwapChain(0, &pChain);

		if (pChain) pChain->GetPresentParameters(&pp);

		ImGui_ImplWin32_Init(param.hFocusWindow);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(0.f, 0.f);
		style.ScrollbarSize = 1.f;
		style.GrabMinSize = 0.f;

		ImGui::StyleColorsDark();

		ImFontConfig fontConfig;
		fontConfig.RasterizerFlags = RasterizerFlags::ForceAutoHint;

		fonts.roboto = io.Fonts->AddFontFromMemoryCompressedTTF(roboto_data, roboto_size,
			15.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.robotoTitle = io.Fonts->AddFontFromMemoryCompressedTTF(roboto_data, roboto_size,
			20.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.icons = io.Fonts->AddFontFromMemoryCompressedTTF(icons_data, icons_size,
			19.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.iconsLarge = io.Fonts->AddFontFromMemoryCompressedTTF(icons_data, icons_size,
			30.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.weaponIcons = io.Fonts->AddFontFromMemoryCompressedTTF(weapon_icons_data, weapon_icons_size,
			19.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.StarWars = io.Fonts->AddFontFromMemoryCompressedTTF(starwars2_compressed_data, starwars2_compressed_size,
			22.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.interfaces = io.Fonts->AddFontFromMemoryTTF((void*)interfaces, sizeof(interfaces),
			15.f * 1.25, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.watermark = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(gravityb_compressed_data, gravityb_compressed_size,
			16.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.cs = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(weaponicons, sizeof(weaponicons),
			14.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		fonts.cs_huge = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(weaponicons, sizeof(weaponicons),
			25.f, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());

		if(!logo_nuts)
			D3DXCreateTextureFromFileInMemoryEx(device, vader_smaller_png, sizeof(vader_smaller_png), 50, 55, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &logo_nuts);

		if (!logo_nuts_highres)
			D3DXCreateTextureFromFileInMemoryEx(device, logo_png, sizeof(logo_png), 400, 442, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &logo_nuts_highres);

		//if (!user_pfp)
		//	D3DXCreateTextureFromFileInMemoryEx(device, g_Vars.globals.userdata_pfp.memory, g_Vars.globals.userdata_pfp.size, 192, 192, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &user_pfp);

		initialized = true;
	}

	return initialized;
}

void Menu::Ragebot() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// SUB TABS --------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		if (ImGui::SubTab(XorStr("Main"), ImVec2(84.66f, 35.f), 0 == aimTab))
			aimTab = 0;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Other"), ImVec2(84.66f, 35.f), 1 == aimTab))
			aimTab = 1;

		ImGui::PopFont();
		ImGui::PushFont(fonts.weaponIcons);

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Y"), ImVec2(84.66f, 35.f), 2 == aimTab))
			aimTab = 2;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("a"), ImVec2(84.66f, 35.f), 3 == aimTab))
			aimTab = 3;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Z"), ImVec2(84.66f, 35.f), 4 == aimTab))
			aimTab = 4;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("G"), ImVec2(84.66f, 35.f), 5 == aimTab))
			aimTab = 5;

		ImGui::PopFont();
		ImGui::PushFont(fonts.roboto);

		ImGui::EndGroup();
	}
	ImGui::SetCursorPos(ImVec2(232.f, 58.f));
	ImGui::BeginGroup(); {
		ImGui::PopFont();
		ImGui::PushFont(fonts.weaponIcons);

		if (ImGui::SubTab(XorStr("A"), ImVec2(84.66f, 35.f), 6 == aimTab))
			aimTab = 6;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("T"), ImVec2(84.66f, 35.f), 7 == aimTab))
			aimTab = 7;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("c"), ImVec2(84.66f, 35.f), 8 == aimTab))
			aimTab = 8;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("N"), ImVec2(84.66f, 35.f), 9 == aimTab))
			aimTab = 9;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("g"), ImVec2(84.66f, 35.f), 10 == aimTab))
			aimTab = 10;

		ImGui::PopFont();
		ImGui::PushFont(fonts.roboto);

		ImGui::EndGroup();
	}

	CVariables::RAGE* rbot = nullptr;
	switch (aimTab) {
	case 1:
		rbot = &g_Vars.rage_default;
		break;
	case 2:
		rbot = &g_Vars.rage_autosnipers;
		break;
	case 3:
		rbot = &g_Vars.rage_scout;
		break;
	case 4:
		rbot = &g_Vars.rage_awp;
		break;
	case 5:
		rbot = &g_Vars.rage_pistols;
		break;
	case 6:
		rbot = &g_Vars.rage_heavypistols;
		break;
	case 7:
		rbot = &g_Vars.rage_rifles;
		break;
	case 8:
		rbot = &g_Vars.rage_shotguns;
		break;
	case 9:
		rbot = &g_Vars.rage_smgs;
		break;
	case 10:
		rbot = &g_Vars.rage_heavys;
		break;
	default:
		rbot = &g_Vars.rage_default;
		break;
	}

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 100.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Aimbot"), ImVec2(244.f, 401.f)); {

			const char* TargetSelection[] = { XorStr("Highest damage"), XorStr("Nearest to crosshair"), XorStr("Lowest health"), XorStr("Lowest distance"), XorStr("Lowest latency") };

			const char* MultipointIntensity[] = { XorStr("Low"), XorStr("Medium"), XorStr("High"), XorStr("Intense") };


			std::vector<MultiItem_t> hitboxes = {
				{ XorStr("Head"), &rbot->hitboxes_head },
				{ XorStr("Neck"), &rbot->hitboxes_neck },
				{ XorStr("Chest"), &rbot->hitboxes_chest },
				{ XorStr("Stomach"), &rbot->hitboxes_stomach },
				{ XorStr("Pelvis"), &rbot->hitboxes_pelvis },
				{ XorStr("Arms"), &rbot->hitboxes_arms },
				{ XorStr("Legs"), &rbot->hitboxes_legs },
				{ XorStr("Feet"), &rbot->hitboxes_feets },
			};

			std::vector<MultiItem_t> multipoints = {
				{ XorStr("Head"), &rbot->mp_hitboxes_head },
				{ XorStr("Chest"), &rbot->mp_hitboxes_chest },
				{ XorStr("Stomach"), &rbot->mp_hitboxes_stomach },
				{ XorStr("Legs"), &rbot->mp_hitboxes_legs },
				{ XorStr("Feet"), &rbot->mp_hitboxes_feets },
			};

			std::vector<MultiItem_t> prefer_body_cond = {
				{ XorStr("Always"), &rbot->prefer_body_always },
				{ XorStr("Not resolved"), &rbot->prefer_body_not_resolved },
				{ XorStr("In air"), &rbot->prefer_body_in_air },
				{ XorStr("Lethal x2"), &rbot->prefer_body_x2lethal },
				{ XorStr("Doubletapping"), &rbot->prefer_body_doubletapping },
			};

			std::vector<MultiItem_t> prefer_head_cond = {
				{ XorStr("Resolved"), &rbot->prefer_head_resolved },
			};

			std::vector<MultiItem_t> force_body_cond = {
				{ XorStr("x Misses"), &rbot->force_body_miss },
				{ XorStr("In air"), &rbot->force_body_air },
			};

			if (aimTab == 0) {
				ImGui::Checkbox(XorStr("Enable"), &g_Vars.rage.enabled);
				ImGui::Checkbox(XorStr("Silent aim"), &g_Vars.rage.silent_aim);
				ImGui::Checkbox(XorStr("Auto-fire"), &g_Vars.rage.auto_fire);
				ImGui::Checkbox(XorStr("Experimental resolver"), &g_Vars.misc.expermimental_resolver);
				//ImGui::Checkbox(XorStr("Visualize hitscan"), &g_Vars.misc.visualize_hitscan);
				ImGui::Checkbox(XorStr("Knife bot"), &g_Vars.misc.knife_bot);
				const char* knife_bot_type[] = { XorStr("Default"), XorStr("Backstab"), XorStr("Quick") };
				if (g_Vars.misc.knife_bot) {
					ImGui::Combo(XorStr("Knifebot type"), &g_Vars.misc.knife_bot_type, knife_bot_type, IM_ARRAYSIZE(knife_bot_type));
				}

				ImGui::Checkbox(XorStr("Zeus bot"), &g_Vars.misc.zeus_bot);
				if (g_Vars.misc.zeus_bot) {
					ImGui::SliderFloat(XorStr("Zeus bot hitchance"), &g_Vars.misc.zeus_bot_hitchance, 1.f, 80.f, XorStr("%.0f%%"));
				}
			}
			else {
				ImGui::Checkbox(XorStr("Enable"), &rbot->active);
				ImGui::Combo(XorStr("Target selection"), &rbot->target_selection, TargetSelection, IM_ARRAYSIZE(TargetSelection));
				ImGui::MultiCombo(XorStr("Hitboxes"), hitboxes);
				ImGui::MultiCombo(XorStr("Multipoints"), multipoints);
				ImGui::Combo(XorStr("Multipoint Intensity"), &rbot->multipoint_intensity, MultipointIntensity, IM_ARRAYSIZE(MultipointIntensity));
				ImGui::Checkbox(XorStr("Prefer body-aim"), &rbot->prefer_body);
				if (rbot->prefer_body) {
					ImGui::MultiCombo(XorStr("Prefer body-aim conditions"), prefer_body_cond);
				}
				ImGui::Checkbox(XorStr("Force body-aim"), &rbot->force_body);
				if (rbot->force_body) {
					ImGui::MultiCombo(XorStr("Force body-aim conditions"), force_body_cond);
					if (rbot->force_body_miss) {
						ImGui::SliderInt(XorStr("Max misses"), &rbot->force_body_miss_amount, 1, 5, XorStr("%d misses"));
					}
				}
				ImGui::Checkbox(XorStr("Prefer head-aim"), &rbot->prefer_head);
				if (rbot->prefer_head) {
					ImGui::MultiCombo(XorStr("Prefer head-aim conditions"), prefer_head_cond);
				}
				ImGui::Checkbox(XorStr("Ignore limbs when moving"), &rbot->ignorelimbs_ifwalking);
				ImGui::Checkbox(XorStr("Static point scale"), &rbot->static_point_scale);
				if (rbot->static_point_scale) {
					ImGui::SliderFloat(XorStr("Head scale"), &rbot->point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
					ImGui::SliderFloat(XorStr("Body scale"), &rbot->body_point_scale, 1.f, 100.0f, XorStr("%.0f%%"));
				}
				ImGui::SliderFloat(XorStr("Hitchance amount"), &rbot->hitchance, 0.0f, 100.0f, XorStr("%.0f%%"));

				ImGui::SliderInt(XorStr("Visible damage"), &rbot->min_damage_visible, 1, 130, std::string(rbot->min_damage_visible > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_visible - 100)))) : XorStr("%d hp")).c_str());
				ImGui::Checkbox(XorStr("Auto-wall"), &rbot->autowall);
				if (rbot->autowall) {
					ImGui::SliderInt(XorStr("Non Visible damage"), &rbot->min_damage, 1, 130, std::string(rbot->min_damage > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage - 100)))) : XorStr("%d hp")).c_str());
				}
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 100.f));
	ImGui::BeginGroup(); {
		if (aimTab == 0) {
			ImGui::BeginGroupBox(XorStr("Exploits"), ImVec2(244.f, 275.f)); {
				ImGui::Checkbox(XorStr("Doubletap"), &g_Vars.rage.exploit);
				ImGui::SameLine(213.f);
				ImGui::Keybind("##DTkey", &g_Vars.rage.key_dt.key, &g_Vars.rage.key_dt.cond);
				const char* doubletap_modes[] = { XorStr("Offensive"), XorStr("Defensive") };

				ImGui::Combo(XorStr("Doubletap type"), &g_Vars.rage.double_tap_type, doubletap_modes, IM_ARRAYSIZE(doubletap_modes));
				
				if (g_Vars.rage.exploit) {
					ImGui::Checkbox(XorStr("Extended teleport"), &g_Vars.rage.dt_defensive_teleport);
					ImGui::Checkbox(XorStr("Doubletap exploits"), &g_Vars.rage.dt_exploits);
					if (g_Vars.rage.dt_exploits) {
						std::vector<MultiItem_t> exploits = {
							{ XorStr("Standby choke"), &g_Vars.rage.exploit_standbychoke },
							{ XorStr("Lag exploit"), &g_Vars.rage.exploit_lag },
							{ XorStr("Lag peek"), &g_Vars.rage.exploit_lag_peek },
							{ XorStr("Break lag compensation"), &g_Vars.rage.exploit_lagcomp },
						};

						ImGui::MultiCombo(XorStr("Exploits"), exploits);
					}
				}

				ImGui::Checkbox(XorStr("Ping-spike"), &g_Vars.misc.extended_backtrack);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##Ping-spike Key"), &g_Vars.misc.extended_backtrack_key.key, &g_Vars.misc.extended_backtrack_key.cond);
				if (g_Vars.misc.extended_backtrack) {
					ImGui::SliderFloat(XorStr("Ping-spike amount"), &g_Vars.misc.extended_backtrack_time, 0.f, 1.f, XorStr("%.2fs"));
				}


				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			ImGui::BeginGroupBox(XorStr("Accuracy"), ImVec2(244.f, 190.f)); {

				const char* autoscopeoptions[] = { XorStr("Off"), XorStr("Always Scope"), XorStr("Hitchance Fail") };

				if (aimTab == 1 || aimTab == 2 || aimTab == 3 || aimTab == 4) {
					ImGui::Combo(XorStr("Auto-scope Options"), &rbot->autoscope, autoscopeoptions, IM_ARRAYSIZE(autoscopeoptions));
				}

				ImGui::Checkbox(XorStr("Auto-stop"), &rbot->autostop_check);
				std::vector<MultiItem_t> stop_options = {
					{ XorStr("Always stop"), &rbot->always_stop_nigga },
					{ XorStr("Force Accuracy"), &rbot->always_stop },
					{ XorStr("Between shots"), &rbot->between_shots },
				};

				if (rbot->autostop_check) {
					ImGui::MultiCombo(XorStr("Auto-stop options"), stop_options);
				}

				ImGui::Checkbox(XorStr("Delay shot on unduck"), &rbot->delay_shot_on_unducking);


				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}

		if (aimTab == 0) {
			ImGui::BeginGroupBox(XorStr("Other"), ImVec2(244.f, 106.f)); {

				ImGui::Text(XorStr("Force bodyaim"));
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##Force bodyaim key"), &g_Vars.rage.prefer_body.key, &g_Vars.rage.prefer_body.cond);

				ImGui::Text(XorStr("Override resolver"));
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##Override resolver key"), &g_Vars.rage.override_reoslver.key, &g_Vars.rage.override_reoslver.cond);


				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			ImGui::BeginGroupBox(XorStr("Other"), ImVec2(244.f, 191.f)); {

				ImGui::Checkbox(XorStr("Damage override"), &rbot->min_damage_override);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##MinDamageOverride"), &g_Vars.rage.key_dmg_override.key, &g_Vars.rage.key_dmg_override.cond);
				if (rbot->min_damage_override) {
					ImGui::SliderInt(XorStr("Dmg override amount"), &rbot->min_damage_override_amount, 1, 130, std::string(rbot->min_damage_override_amount > 100 ? (std::string(XorStr("HP + ")).append(std::string(std::to_string(rbot->min_damage_override_amount - 100)))) : XorStr("%d hp")).c_str());
				}

				ImGui::Checkbox(XorStr("Override hitscan"), &rbot->override_hitscan);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##OverrideHitscanKey"), &g_Vars.rage.override_key.key, &g_Vars.rage.override_key.cond);

				if (rbot->override_hitscan) {
					std::vector<MultiItem_t> override_hitboxes = {
						{ XorStr("Head"), &rbot->bt_hitboxes_head },
						{ XorStr("Chest"), &rbot->bt_hitboxes_chest },
						{ XorStr("Stomach"), &rbot->bt_hitboxes_stomach },
						{ XorStr("Pelvis"), &rbot->bt_hitboxes_pelvis },
						{ XorStr("Arms"), &rbot->bt_hitboxes_arms },
						{ XorStr("Legs"), &rbot->bt_hitboxes_legs },
						{ XorStr("Feet"), &rbot->bt_hitboxes_feets },
					};

					ImGui::MultiCombo(XorStr("Override hitboxes"), override_hitboxes);
				}

				if (g_Vars.rage.exploit) {
					ImGui::SliderFloat(XorStr("Doubletap hitchance"), &rbot->doubletap_hitchance, 1.f, 100.f, XorStr("%.0f%%"));
					ImGui::SliderFloat(XorStr("Doubletap minimum Dmg"), &rbot->doubletap_dmg, 1.f, 100.f, XorStr("%1.f hp"));
					ImGui::SliderFloat(XorStr("Doubletap speed"), &rbot->doubletap_speed, 11.f, 17.f, XorStr("%1.f ticks"));
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

int antiaimConditionTab = 0;

void Menu::AntiAim() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// SUB TABS --------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(280.f, 21.f));
	ImGui::BeginGroup(); {
		if (ImGui::SubTab(XorStr("Main"), ImVec2(169.33f, 35.f), 0 == antiaimTab))
			antiaimTab = 0;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Other"), ImVec2(169.33f, 35.f), 1 == antiaimTab))
			antiaimTab = 1;

		ImGui::EndGroup();
	}

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 76.f));
	ImGui::BeginGroup(); {
		if (antiaimTab == 0) {
			if (ImGui::SubTab(XorStr("Stand"), ImVec2(169.33f, 35.f), 0 == antiaimConditionTab))
				antiaimConditionTab = 0;
			ImGui::SameLine();
			if (ImGui::SubTab(XorStr("Move"), ImVec2(169.33f, 35.f), 1 == antiaimConditionTab))
				antiaimConditionTab = 1;
			ImGui::SameLine();
			if (ImGui::SubTab(XorStr("Air"), ImVec2(169.33f, 35.f), 2 == antiaimConditionTab))
				antiaimConditionTab = 2;

			ImGui::BeginGroupBox(XorStr("AntiAim"), ImVec2(244.f, 366.f)); {
				const char* styles[]{ XorStr("Static"), XorStr("Jitter"), XorStr("Spin") };
				const char* pitches[] = { XorStr("Off"), XorStr("Down"), XorStr("Up"), XorStr("Zero") };
				const char* real_yaw[] = { XorStr("Off"), XorStr("180"), XorStr("Rotate"), XorStr("Jitter"), XorStr("180z"), XorStr("Lowerbody"), XorStr("Custom") };
				const char* base_yaw[] = { XorStr("forward"), XorStr("backward") };
				const char* lby_modes[] = { XorStr("Off"), XorStr("Static"), XorStr("Double flick"), XorStr("Twist") };
				const char* fake_yaw[] = { XorStr("Off"), XorStr("Normal"), XorStr("Spin"), XorStr("Lowerbody") };
				const char* freestand_mode[] = { XorStr("Crosshair"), XorStr("Thickness") };

				CVariables::ANTIAIM_STATE* settings;

				switch (antiaimConditionTab) {
				case 0:
					settings = &g_Vars.antiaim_stand;
					break;
				case 1:
					settings = &g_Vars.antiaim_move;
					break;
				case 2:
					settings = &g_Vars.antiaim_air;
					break;
				}

				// enable AA.
				ImGui::Checkbox(XorStr("Enable"), &g_Vars.antiaim.enabled);
				ImGui::Checkbox(XorStr("Edge antiaim"), &g_Vars.antiaim.edge_aa);
				ImGui::Combo(XorStr("Pitch"), &settings->pitch, pitches, IM_ARRAYSIZE(pitches));
				ImGui::Combo(XorStr("Base yaw"), &settings->base_yaw, base_yaw, IM_ARRAYSIZE(base_yaw));
				ImGui::Combo(XorStr("Yaw"), &settings->yaw, real_yaw, IM_ARRAYSIZE(real_yaw));

				if (settings->yaw == 3) {
					ImGui::SliderInt(XorStr("Jitter range"), &settings->jitter_range, -180, 180, XorStr("%d"));
					ImGui::SliderInt(XorStr("Jitter swap"), &settings->jitter_speed, 1, 64, settings->jitter_speed == 1 ? XorStr("every %d tick") : XorStr("every %d ticks"));
				}
				else if (settings->yaw == 2) {
					ImGui::SliderFloat(XorStr("Rotation range"), &settings->rot_range, 1.f, 360.f, XorStr("%1.f"));
					ImGui::SliderFloat(XorStr("Rotation speed"), &settings->rot_speed, 1.f, 100.f, XorStr("%1.f"));
				}
				else if (settings->yaw == 6) {
					ImGui::SliderFloat(XorStr("Custom yaw"), &settings->custom_yaw, -180.f, 180.f, XorStr("%1.f"));
				}
				ImGui::Checkbox(XorStr("At targets"), &g_Vars.antiaim.at_targets);
				ImGui::Checkbox(XorStr("Break LBY"), &g_Vars.antiaim.lby_breaker);
				ImGui::Checkbox(XorStr("Pre-Break"), &g_Vars.antiaim.anti_lastmove);
				if (g_Vars.antiaim.lby_breaker) {
					ImGui::SliderFloat(XorStr("Break angle"), &g_Vars.antiaim.break_lby, -180.f, 180.f, XorStr("%1.f"));
					ImGui::SliderFloat(XorStr("First break angle"), &g_Vars.antiaim.break_lby_first, -180.f, 180.f, XorStr("%1.f"));
					ImGui::Checkbox(XorStr("Static angle"), &g_Vars.antiaim.static_angle);
					ImGui::Checkbox(XorStr("Optimal adjust"), &g_Vars.antiaim.optimal_adjust);
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					{
						ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15, 0.15, 0.15, 240));
						ImGui::SetTooltip(XorStr("Your LBY is automatically adjusted to ensure you always break LBY."));
						ImGui::PopStyleColor(1);
					}

					std::vector<MultiItem_t> lby_disablers = {
						{ XorStr("Fakewalking"), &g_Vars.antiaim.lby_disable_fakewalk },
						{ XorStr("Manual"), &g_Vars.antiaim.lby_disable_manual },
						{ XorStr("Unsafe"), &g_Vars.antiaim.lby_disable_unsafe },
					};

					ImGui::MultiCombo(XorStr("LBY disablers"), lby_disablers);
				}

				ImGui::Checkbox(XorStr("Freestanding"), &g_Vars.antiaim.freestand);
				if (g_Vars.antiaim.freestand) {
					ImGui::Combo(XorStr("Freestanding mode"), &g_Vars.antiaim.freestand_mode, freestand_mode, IM_ARRAYSIZE(freestand_mode));
					ImGui::SliderFloat(XorStr("Timeout"), &g_Vars.antiaim.timeout_time, 0.f, 10.f, XorStr("%1.f"));
					ImGui::SliderFloat(XorStr("Add Amount"), &g_Vars.antiaim.add_yaw, -180.f, 180.f, XorStr("%1.f"));

					std::vector<MultiItem_t> freestand_disablers = {
						{ XorStr("Fakewalking"), &g_Vars.antiaim.freestand_disable_fakewalk },
						{ XorStr("Running"), &g_Vars.antiaim.freestand_disable_run },
						{ XorStr("In air"), &g_Vars.antiaim.freestand_disable_air },
					};

					ImGui::Checkbox(XorStr("Lock freestanding"), &g_Vars.antiaim.freestand_lock);
					ImGui::MultiCombo(XorStr("Freestanding disablers"), freestand_disablers);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			ImGui::BeginGroupBox(XorStr("Exploits"), ImVec2(244.f, 425.f)); {
				const char* mind_trick_mode[] = { XorStr("Offensive"), XorStr("Defensive") };

				ImGui::Checkbox(XorStr("Fakewalk"), &g_Vars.misc.slow_walk);
				ImGui::SameLine(213.f);
				ImGui::Keybind("Fakewalk key", &g_Vars.misc.slow_walk_bind.key, &g_Vars.misc.slow_walk_bind.cond);
				if (g_Vars.misc.slow_walk) {
					ImGui::SliderInt(XorStr("Fake-walk speed"), &g_Vars.misc.slow_walk_speed, 2, 17, XorStr("%d"));
					ImGui::SliderFloat(XorStr("Slow-walk speed"), &g_Vars.misc.slowwalk_speed, 0.1f, 0.3f, XorStr("%.1f"));
				}

				ImGui::Checkbox(XorStr("Jedi Mind-Trick"), &g_Vars.misc.mind_trick);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##Mind-Trick key"), &g_Vars.misc.mind_trick_bind.key, &g_Vars.misc.mind_trick_bind.cond);
				if (g_Vars.misc.mind_trick) {
					ImGui::Combo(XorStr("Mind-Trick Mode"), &g_Vars.misc.mind_trick_mode, mind_trick_mode, IM_ARRAYSIZE(mind_trick_mode));
					ImGui::Checkbox(XorStr("Bypass Mrx"), &g_Vars.misc.bypass_mrx);
				}

				ImGui::Checkbox(XorStr("Move Exploit"), &g_Vars.misc.move_exploit);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("Move Exploit key"), &g_Vars.misc.move_exploit_key.key, &g_Vars.misc.move_exploit_key.cond);
				if (g_Vars.misc.move_exploit) {
					ImGui::SliderInt(XorStr("Move Exploit intensity"), &g_Vars.misc.move_exploit_intensity, 1, 16, XorStr("%d"));
				}

				ImGui::Checkbox(XorStr("Flick on Shift"), &g_Vars.antiaim.desync_on_dt);
				ImGui::SameLine(213.f);
				ImGui::Keybind(XorStr("##Desync Inverter key"), &g_Vars.antiaim.desync_on_dt_invert.key, &g_Vars.antiaim.desync_on_dt_invert.cond);

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 76.f));
	ImGui::BeginGroup(); {

		if (antiaimTab == 0) {
			ImGui::SetCursorPos(ImVec2(457.f, 135.f));
			ImGui::BeginGroupBox(XorStr("Misc"), ImVec2(244.f, 366.f)); {

				ImGui::Checkbox(XorStr("Condition specific"), &g_Vars.antiaim.condition_specific);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				{
					ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15, 0.15, 0.15, 240));
					ImGui::SetTooltip(XorStr("Anti-Aim modes for when you're standing, moving, or in air."));
					ImGui::PopStyleColor(1);
				}
				ImGui::SliderFloat(XorStr("Stand velocity threshold"), &g_Vars.antiaim.stand_velocity_threshold, 0.1f, 250.0f, XorStr("%.1f"));

				ImGui::Checkbox(XorStr("Distortion"), &g_Vars.antiaim.distort);
				if (g_Vars.antiaim.distort) {
					ImGui::Checkbox(XorStr("Override manual distortion"), &g_Vars.antiaim.distort_manual_aa);
					ImGui::Checkbox(XorStr("Force turn"), &g_Vars.antiaim.distort_twist);
					ImGui::SliderFloat(XorStr("Speed"), &g_Vars.antiaim.distort_speed, 1.f, 10.f, XorStr("%.1fs"));
					ImGui::SliderFloat(XorStr("Max time"), &g_Vars.antiaim.distort_max_time, 0.f, 10.f, XorStr("%.f"));
					ImGui::SliderFloat(XorStr("Range"), &g_Vars.antiaim.distort_range, -360.f, 360.f, XorStr("%.f"));

					std::vector<MultiItem_t> distort_disablers = {
						{ XorStr("Fakewalking"), &g_Vars.antiaim.distort_disable_fakewalk },
						{ XorStr("Running"), &g_Vars.antiaim.distort_disable_run },
						{ XorStr("Airborne"), &g_Vars.antiaim.distort_disable_air },
					};

					ImGui::MultiCombo(XorStr("Distortion disablers"), distort_disablers);
				}

				ImGui::Checkbox(XorStr("Manual"), &g_Vars.antiaim.manual);
				if (g_Vars.antiaim.manual) {
					ImGui::Checkbox(XorStr("Manual arrows"), &g_Vars.antiaim.manual_arrows);
					if (g_Vars.antiaim.manual_arrows) {
						ColorPicker(XorStr("Manual color"), g_Vars.antiaim.manual_color, true, false);
					}

					ImGui::Text(XorStr("Left"));
					ImGui::SameLine(213.f);
					ImGui::Keybind("Left key", &g_Vars.antiaim.manual_left_bind.key, &g_Vars.antiaim.manual_left_bind.cond);

					ImGui::Text(XorStr("Right"));
					ImGui::SameLine(213.f);
					ImGui::Keybind(XorStr("Right key"), &g_Vars.antiaim.manual_right_bind.key, &g_Vars.antiaim.manual_right_bind.cond);

					ImGui::Text(XorStr("Back"));
					ImGui::SameLine(213.f);
					ImGui::Keybind(XorStr("Back key"), &g_Vars.antiaim.manual_back_bind.key, &g_Vars.antiaim.manual_back_bind.cond);
				}

				ImGui::Checkbox(XorStr("Draw antiaim angles"), &g_Vars.esp.draw_antiaim_angles);
				if (g_Vars.esp.draw_antiaim_angles) {
					ColorPicker_w_name(XorStr("Real color"), g_Vars.esp.draw_antiaim_angles_real, false, false);
					ColorPicker_w_name(XorStr("Lby color"), g_Vars.esp.draw_antiaim_angles_lby, false, false);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			ImGui::BeginGroupBox(XorStr("FakeLag"), ImVec2(244.f, 425.f)); {
				ImGui::Checkbox(XorStr("Enable"), &g_Vars.fakelag.enabled);

				std::vector<MultiItem_t> fakelag_cond = { { XorStr("Standing"), &g_Vars.fakelag.when_standing }, { XorStr("Moving"), &g_Vars.fakelag.when_moving }, { XorStr("In air"), &g_Vars.fakelag.when_air } };
				std::vector<MultiItem_t> fakelag_triggers = { { XorStr("On Peek"), &g_Vars.fakelag.trigger_on_peek }, { XorStr("On Duck"), &g_Vars.fakelag.trigger_duck }, { XorStr("On Land"),&g_Vars.fakelag.trigger_land }, { XorStr("On Weapon Activity"),&g_Vars.fakelag.trigger_weapon_activity } };
				ImGui::MultiCombo(XorStr("Conditions"), fakelag_cond);
				ImGui::MultiCombo(XorStr("Triggers"), fakelag_triggers);

				const char* FakelagType[] = { XorStr("Maximum"), XorStr("Dynamic"), XorStr("Fluctuate") };
				ImGui::Combo(XorStr("Type"), &g_Vars.fakelag.choke_type, FakelagType, IM_ARRAYSIZE(FakelagType));
				ImGui::SliderFloat(XorStr("Limit"), &g_Vars.fakelag.choke, 0.f, 16.f, "%1.f");
				ImGui::SliderFloat(XorStr("Trigger Limit"), &g_Vars.fakelag.alternative_choke, 0.f, 16.f, "%1.f");

				ImGui::SliderFloat(XorStr("Variance"), &g_Vars.fakelag.variance, 0.0f, 100.0f, XorStr("%.0f%%"));

				ImGui::Checkbox(XorStr("Fakelag on shot"), &g_Vars.fakelag.fakelag_onshot);

				ImGui::Checkbox(XorStr("Visualize lag"), &g_Vars.fakelag.vis_lag);
				if (g_Vars.fakelag.vis_lag) {
					ColorPicker(XorStr("##Visualizelagcolor"), g_Vars.fakelag.vis_lag_color, true, false);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Players() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// SUB TABS --------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		if (ImGui::SubTab(XorStr("Enemies"), ImVec2(169.33f, 35.f), 0 == playerTab))
			playerTab = 0;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Friendlies"), ImVec2(169.33f, 35.f), 1 == playerTab))
			playerTab = 1;

		ImGui::SameLine();

		if (ImGui::SubTab(XorStr("Local player"), ImVec2(169.33f, 35.f), 2 == playerTab))
			playerTab = 2;

		ImGui::EndGroup();
	}

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 76.f));
	ImGui::BeginGroup(); {
		if (playerTab != 2) {
			ImGui::BeginGroupBox(XorStr("Extra sensory perception"), ImVec2(244.f, 425.f)); {
				if (playerTab == 0) {
					std::vector<MultiItem_t> flags = {
						{ XorStr("Zoom"), &g_Vars.esp.draw_scoped },
						{ XorStr("Vulnerable"), &g_Vars.esp.draw_vulnerable },
						{ XorStr("Flashed"), &g_Vars.esp.draw_flashed },
						{ XorStr("Money"), &g_Vars.esp.draw_money },
						{ XorStr("Kevlar"), &g_Vars.esp.draw_armor },
						{ XorStr("Bomb"), &g_Vars.esp.draw_bombc4 },
						{ XorStr("Defusing"), &g_Vars.esp.draw_defusing },
						{ XorStr("Distance"), &g_Vars.esp.draw_distance },
						{ XorStr("Grenade pin"), &g_Vars.esp.draw_grenade_pin },
						{ XorStr("Resolved"), &g_Vars.esp.draw_resolver },
						{ XorStr("Ping"), &g_Vars.esp.draw_ping },
						{ XorStr("Exploiting"), &g_Vars.esp.draw_exploiting },
					};


					ImGui::Checkbox("Enable", &g_Vars.esp.esp_enable);
					g_Vars.esp.team_check = true;
					ImGui::Checkbox(XorStr("Dormant"), &g_Vars.esp.fade_esp);
					ImGui::Checkbox(XorStr("Bounding box"), &g_Vars.esp.box);
					if (g_Vars.esp.box) {
						ColorPicker(XorStr("##Bounding box Color"), g_Vars.esp.box_color, true, false);
					}
					ImGui::Checkbox("Name", &g_Vars.esp.name);
					if (g_Vars.esp.name) {
						ColorPicker(XorStr("##NameColor"), g_Vars.esp.name_color, false, false);
					}
					ImGui::Checkbox(XorStr("Health"), &g_Vars.esp.health);
					if (g_Vars.esp.health) {
						ColorPicker(XorStr("##HealthColor"), g_Vars.esp.health_color, false, false);
						ImGui::Checkbox(XorStr("Override health color"), &g_Vars.esp.health_override);
					}
					ImGui::Checkbox(XorStr("Weapon"), &g_Vars.esp.weapon);
					if (g_Vars.esp.weapon) {
						ColorPicker(XorStr("##WeaponColor"), g_Vars.esp.weapon_color, false, false);
					}
					ImGui::Checkbox(XorStr("Weapon icon"), &g_Vars.esp.weapon_icon);
					if (g_Vars.esp.weapon_icon) {
						ColorPicker(XorStr("##WeaponIconColor"), g_Vars.esp.weapon_icon_color, false, false);
					}
					ImGui::Checkbox(XorStr("Ammo"), &g_Vars.esp.draw_ammo_bar);
					if (g_Vars.esp.draw_ammo_bar) {
						ColorPicker(XorStr("##AmmoColor"), g_Vars.esp.ammo_color, false, false);
					}
					ImGui::Checkbox(XorStr("LBY timer"), &g_Vars.esp.draw_lby_bar);
					if (g_Vars.esp.draw_lby_bar) {
						ColorPicker("##LBYColor", g_Vars.esp.lby_color, false, false);
					}
					ImGui::MultiCombo(XorStr("Flags"), flags);
					ImGui::Checkbox(XorStr("Skeleton"), &g_Vars.esp.skeleton);
					if (g_Vars.esp.skeleton) {
						ColorPicker(XorStr("##SkeletonColor"), g_Vars.esp.skeleton_color, true, false);
					}
					ImGui::Checkbox(XorStr("Radar"), &g_Vars.misc.ingame_radar);
					ImGui::Checkbox(XorStr("Out of view arrows"), &g_Vars.esp.offscren_enabled);
					if (g_Vars.esp.offscren_enabled) {
						ColorPicker(XorStr("##OffscreenColor"), g_Vars.esp.offscreen_color, true, false);
						ImGui::SliderFloat(XorStr("Offscreen size"), &g_Vars.esp.offscren_size, 4, 16.f, XorStr("%1.fpx"));
						ImGui::SliderFloat(XorStr("Offscreen distance"), &g_Vars.esp.offscren_distance, 10, 100.f, XorStr("%1.f%%"));
					}
					ImGui::Checkbox(XorStr("Footsteps"), &g_Vars.esp.footsteps);
					if (g_Vars.esp.footsteps) {
						ColorPicker(XorStr("footsteps color"), g_Vars.esp.footsteps_color, true, false);
						ImGui::SliderFloat(XorStr("Footsteps thickness"), &g_Vars.esp.footsteps_thickness, 1.f, 10.f, XorStr("%.f"));
						ImGui::SliderFloat(XorStr("Footsteps radius"), &g_Vars.esp.footsteps_radius, 50.f, 500.f, XorStr("%.f"));
					}
					ImGui::Checkbox(XorStr("Dlight"), &g_Vars.esp.dlight_enemy_enable);
					if (g_Vars.esp.dlight_enemy_enable) {
						ColorPicker(XorStr("Dlight"), g_Vars.esp.dlight_enemy_color, false, false);
						ImGui::SliderInt(XorStr("Dlight radius"), &g_Vars.esp.dlight_enemy_radius, 0, 275, XorStr("%d"));
					}
				}
				else if (playerTab == 1) {
					ImGui::Checkbox(XorStr("Name"), &g_Vars.esp.teamname);
					if (g_Vars.esp.teamname) {
						ColorPicker(XorStr("##Name2Color"), g_Vars.esp.team_name_color, false, false);
					}

					ImGui::Text(XorStr("Teammate esp is a WIP"));
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			const char* chams_mats_local[] = { XorStr("Disabled"),  XorStr("Texture"), XorStr("Flat"), XorStr("Custom") };
			const char* chams_mats_overlay_local[] = { XorStr("Disabled"), XorStr("Glow"), XorStr("Animated") };
			const char* glow_types[] = { XorStr("Standard"), XorStr("Pulse"), XorStr("Inner") };

			ImGui::BeginGroupBox(XorStr("Player viewmodel"), ImVec2(244.f, 200.f)); {
				ImGui::Checkbox(XorStr("Transparency while scoped"), &g_Vars.esp.blur_in_scoped);
				if (g_Vars.esp.blur_in_scoped) {
					ImGui::SliderFloat(XorStr("Scoped transparency"), &g_Vars.esp.blur_in_scoped_value, 0.0f, 100.f, XorStr("%.f"));
				}
				ImGui::Checkbox(XorStr("Local Skeleton"), &g_Vars.esp.local_skeleton);
				ColorPicker(XorStr("Skeleton Color"), g_Vars.esp.local_skeleton_color, true, false);
				ImGui::Checkbox(XorStr("Local Dlight"), &g_Vars.esp.dlight_local_enable);
				if (g_Vars.esp.dlight_local_enable) {
					ColorPicker(XorStr("Local Dlight"), g_Vars.esp.dlight_local_color, false, false);
					ImGui::SliderInt(XorStr("Local Dlight radius"), &g_Vars.esp.dlight_local_radius, 0, 275, XorStr("%d"));
				}
				ImGui::Checkbox(XorStr("Local Glow"), &g_Vars.esp.glow_local);
				ColorPicker(XorStr("##localglowcolor"), g_Vars.esp.glow_local_color, true, false);
				if (g_Vars.esp.glow_local) {
					ImGui::Combo(XorStr("Local Glow Type"), &g_Vars.esp.glow_type_local, glow_types, IM_ARRAYSIZE(glow_types));
				}

				ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.chams_local);
				if (g_Vars.esp.chams_local) {

					ImGui::Combo(XorStr("Local"), &g_Vars.esp.new_chams_local, chams_mats_local, IM_ARRAYSIZE(chams_mats_local));
					ColorPicker_w_name(XorStr("Local color"), g_Vars.esp.chams_local_color, true, false);
					if (g_Vars.esp.new_chams_local == 3) {
						ImGui::SliderFloat(XorStr("Local pearlescence"), &g_Vars.esp.chams_local_pearlescence, 0.f, 100.f, XorStr("%.f"));
						ColorPicker_w_name(XorStr("Local pearlescence color"), g_Vars.esp.chams_local_pearlescence_color, true, false);
						ImGui::SliderFloat(XorStr("Local shine"), &g_Vars.esp.chams_local_shine, 0.f, 100.f, XorStr("%.f"));
					}

					ImGui::Combo(XorStr("Local overlay"), &g_Vars.esp.new_chams_local_overlay, chams_mats_overlay_local, IM_ARRAYSIZE(chams_mats_overlay_local));
					ColorPicker_w_name(XorStr("Local overlay color"), g_Vars.esp.new_chams_local_overlay_color, true, false);
					if (g_Vars.esp.new_chams_local_overlay == 1) {
						ImGui::SliderFloat(XorStr("Glow strength ##local"), &g_Vars.esp.chams_local_outline_value, 0.f, 100.f, XorStr("%.f"));
					}
					ImGui::Checkbox(XorStr("Local overlay wireframe"), &g_Vars.esp.chams_local_outline_wireframe);
					ImGui::Checkbox(XorStr("Draw original model ##local"), &g_Vars.esp.new_chams_local_original_model);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}

			ImGui::BeginGroupBox(XorStr("Attachments viewmodel"), ImVec2(244.f, 200.f)); {

				ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.chams_attachments);
				if (g_Vars.esp.chams_attachments) {
					ImGui::Combo(XorStr("Attachments"), &g_Vars.esp.attachments_chams_mat, chams_mats_local, IM_ARRAYSIZE(chams_mats_local));
					ColorPicker_w_name(XorStr("Attachments color"), g_Vars.esp.attachments_chams_color, true, false);
					if (g_Vars.esp.attachments_chams_mat == 3) {
						ImGui::SliderFloat(XorStr("Attachments pearlescence"), &g_Vars.esp.chams_attachments_pearlescence, 0.f, 100.f, XorStr("%.f"));
						ColorPicker_w_name(XorStr("Attachments pearlescence color"), g_Vars.esp.chams_attachments_pearlescence_color, true, false);
						ImGui::SliderFloat(XorStr("Attachments shine"), &g_Vars.esp.chams_attachments_shine, 0.f, 100.f, XorStr("%.f"));
					}

					ImGui::Combo(XorStr("Attachments overlay"), &g_Vars.esp.new_chams_attachments_overlay, chams_mats_overlay_local, IM_ARRAYSIZE(chams_mats_overlay_local));
					ColorPicker_w_name(XorStr("Attachments overlay color"), g_Vars.esp.new_chams_attachments_overlay_color, true, true);
					if (g_Vars.esp.new_chams_attachments_overlay == 1) {
						ImGui::SliderFloat(XorStr("Glow strength ##attachments"), &g_Vars.esp.chams_attachments_outline_value, 0.f, 100.f, XorStr("%.f"));
					}
					ImGui::Checkbox(XorStr("Attachments overlay wireframe"), &g_Vars.esp.chams_attachments_outline_wireframe);
					ImGui::Checkbox(XorStr("Draw original model ##attachment"), &g_Vars.esp.new_chams_attachments_original_model);

				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
	
		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 76.f));
	ImGui::BeginGroup(); {

		const char* chams_mats[] = { XorStr("Disabled"),  XorStr("Texture"), XorStr("Flat"), XorStr("Custom") };
		const char* chams_mats_overlay[] = { XorStr("Disabled"), XorStr("Glow") };
		const char* chams_mats_backtrack[] = { XorStr("Flat"), XorStr("Texture"), XorStr("Glow") };
		const char* glow_types[] = { XorStr("Standard"), XorStr("Pulse"), XorStr("Inner") };
		const char* chams_mats_overlay_viewmodel[] = { XorStr("Disabled"), XorStr("Glow"), XorStr("Animated") };
		const char* chams_filter_menu[] = { XorStr("Enemy"), XorStr("Local"), XorStr("Viewmodel"), XorStr("Glow") };

		if (playerTab != 2) {
			ImGui::BeginGroupBox(XorStr("Player model"), ImVec2(244.f, 425.f)); {
				if (playerTab == 0) {
					ImGui::Checkbox(XorStr("Backtrack Chams"), &g_Vars.esp.chams_history);
					if (g_Vars.esp.chams_history) {
						ColorPicker(XorStr("##enemybacktrackcolor"), g_Vars.esp.chams_history_color, true, false);
						ImGui::Combo(XorStr("Backtrack material"), &g_Vars.esp.chams_history_material, chams_mats_backtrack, IM_ARRAYSIZE(chams_mats_backtrack));
						if (g_Vars.esp.chams_history_material == 2) {
							ImGui::SliderFloat(XorStr("Glow strength ##backtrack"), &g_Vars.esp.chams_history_outline_value, 0.f, 100.f, XorStr("%.f"));
						}
					}
					ImGui::Checkbox(XorStr("Enemy Glow"), &g_Vars.esp.glow_enemy);
					ColorPicker(XorStr("##enemyglowcolor"), g_Vars.esp.glow_enemy_color, true, false);
					if (g_Vars.esp.glow_enemy) {
						ImGui::Combo(XorStr("Enemy Glow Type"), &g_Vars.esp.glow_type_enemy, glow_types, IM_ARRAYSIZE(glow_types));
					}
					ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.chams_enemy);
					if (g_Vars.esp.chams_enemy) {
						ImGui::Checkbox(XorStr("Enemy visible chams"), &g_Vars.esp.enemy_chams_vis);
						if (g_Vars.esp.enemy_chams_vis) {

							ImGui::Combo(XorStr("Enemy Visible"), &g_Vars.esp.new_chams_enemy, chams_mats, IM_ARRAYSIZE(chams_mats));
							ColorPicker_w_name(XorStr("Visible color"), g_Vars.esp.new_chams_enemy_color, true, false);
							if (g_Vars.esp.new_chams_enemy == 3) {
								ImGui::SliderFloat(XorStr("Enemy visible pearlescence"), &g_Vars.esp.chams_enemy_pearlescence, 0.f, 100.f, XorStr("%.f"));
								ColorPicker_w_name(XorStr("Visible pearlescence color"), g_Vars.esp.chams_enemy_pearlescence_color, true, false);
								ImGui::SliderFloat(XorStr("Enemy visible shine"), &g_Vars.esp.chams_enemy_shine, 0.f, 100.f, XorStr("%.f"));
							}
							ImGui::Combo(XorStr("Enemy visible overlay"), &g_Vars.esp.new_chams_enemy_overlay, chams_mats_overlay, IM_ARRAYSIZE(chams_mats_overlay));
							ColorPicker_w_name(XorStr("Visible overlay color"), g_Vars.esp.new_chams_enemy_overlay_color, true, false);
							if (g_Vars.esp.new_chams_enemy_overlay == 1) {
								ImGui::SliderFloat(XorStr("Glow strength ##enemy"), &g_Vars.esp.chams_enemy_outline_value, 0.f, 100.f, XorStr("%.f"));
							}
							ImGui::Checkbox(XorStr("Enemy visible overlay wireframe"), &g_Vars.esp.chams_enemy_outline_wireframe);
						}

						ImGui::Checkbox(XorStr("Enemy XQZ chams"), &g_Vars.esp.enemy_chams_xqz);
						if (g_Vars.esp.enemy_chams_xqz) {
							ImGui::Combo(XorStr("Enemy XQZ"), &g_Vars.esp.new_chams_enemy_xqz, chams_mats, IM_ARRAYSIZE(chams_mats));
							ColorPicker_w_name(XorStr("Enemy XQZ color"), g_Vars.esp.new_chams_enemy_xqz_color, true, false);
							if (g_Vars.esp.new_chams_enemy_xqz == 3) {
								ImGui::SliderFloat(XorStr("Enemy XQZ pearlescence"), &g_Vars.esp.chams_enemy_xqz_pearlescence, 0.f, 100.f, XorStr("%.f"));
								ColorPicker_w_name(XorStr("XQZ pearlescence color"), g_Vars.esp.chams_enemy_xqz_pearlescence_color, true, false);
								ImGui::SliderFloat(XorStr("Enemy XQZ shine"), &g_Vars.esp.chams_enemy_xqz_shine, 0.f, 100.f, XorStr("%.f"));
							}
							ImGui::Combo(XorStr("Enemy XQZ overlay"), &g_Vars.esp.new_chams_enemy_xqz_overlay, chams_mats_overlay, IM_ARRAYSIZE(chams_mats_overlay));
							ColorPicker_w_name(XorStr("Enemy XQZ overlay color"), g_Vars.esp.new_chams_enemy_xqz_overlay_color, true, false);
							if (g_Vars.esp.new_chams_enemy_xqz_overlay == 1) {
								ImGui::SliderFloat(XorStr("Glow strength ##enemyxqz"), &g_Vars.esp.chams_enemy_xqz_outline_value, 0.f, 100.f, XorStr("%.f"));
							}
							ImGui::Checkbox(XorStr("Enemy XQZ overlay wireframe"), &g_Vars.esp.chams_enemy_outline_xqz_wireframe);
						}
					}
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}
		else {
			const char* chams_mats_local[] = { XorStr("Disabled"),  XorStr("Texture"), XorStr("Flat"), XorStr("Custom") };
			const char* chams_mats_overlay_local[] = { XorStr("Disabled"), XorStr("Glow"), XorStr("Animated") };

			ImGui::BeginGroupBox(XorStr("Weapon viewmodel"), ImVec2(244.f, 200.f)); {

				ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.chams_weapon);
				if (g_Vars.esp.chams_weapon) {

					ImGui::Combo(XorStr("Weapon"), &g_Vars.esp.weapon_chams_mat, chams_mats_local, IM_ARRAYSIZE(chams_mats_local));
					ColorPicker_w_name(XorStr("Weapon color"), g_Vars.esp.weapon_chams_color, true, false);
					if (g_Vars.esp.weapon_chams_mat == 3) {
						ImGui::SliderFloat(XorStr("Weapon pearlescence"), &g_Vars.esp.chams_weapon_pearlescence, 0.f, 100.f, XorStr("%.f"));
						ColorPicker_w_name(XorStr("Weapon pearlescence color"), g_Vars.esp.chams_weapon_pearlescence_color, true, false);
						ImGui::SliderFloat(XorStr("Weapon shine"), &g_Vars.esp.chams_weapon_shine, 0.f, 100.f, "%.f");
					}

					ImGui::Combo(XorStr("Weapon overlay"), &g_Vars.esp.new_chams_weapon_overlay, chams_mats_overlay_viewmodel, IM_ARRAYSIZE(chams_mats_overlay_viewmodel));
					ColorPicker_w_name(XorStr("Weapon overlay color"), g_Vars.esp.new_chams_weapon_overlay_color, true, false);
					if (g_Vars.esp.new_chams_weapon_overlay == 1) {
						ImGui::SliderFloat(XorStr("Glow strength ##weapon"), &g_Vars.esp.chams_weapon_outline_value, 0.f, 100.f, XorStr("%.f"));
					}
					ImGui::Checkbox(XorStr("Weapon overlay wireframe"), &g_Vars.esp.chams_weapon_outline_wireframe);
					ImGui::Checkbox(XorStr("Draw original model ##weapon"), &g_Vars.esp.new_chams_weapon_original_model);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}

			ImGui::BeginGroupBox(XorStr("Hands viewmodel"), ImVec2(244.f, 200.f)); {

				ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.chams_hands);
				if (g_Vars.esp.chams_hands) {
					ImGui::Combo(XorStr("Hands"), &g_Vars.esp.hands_chams_mat, chams_mats_local, IM_ARRAYSIZE(chams_mats_local));
					ColorPicker_w_name(XorStr("Hands color"), g_Vars.esp.hands_chams_color, true, false);
					if (g_Vars.esp.hands_chams_mat == 3) {
						ImGui::SliderFloat(XorStr("Hands pearlescence"), &g_Vars.esp.chams_hands_pearlescence, 0.f, 100.f, XorStr("%.f"));
						ColorPicker_w_name(XorStr("Hands pearlescence color"), g_Vars.esp.chams_hands_pearlescence_color, true, false);
						ImGui::SliderFloat(XorStr("Hands shine"), &g_Vars.esp.chams_hands_shine, 0.f, 100.f, XorStr("%.f"));
					}

					ImGui::Combo(XorStr("Hands overlay"), &g_Vars.esp.new_chams_hands_overlay, chams_mats_overlay_viewmodel, IM_ARRAYSIZE(chams_mats_overlay_viewmodel));
					ColorPicker_w_name(XorStr("Hands overlay color"), g_Vars.esp.new_chams_hands_overlay_color, true, false);
					if (g_Vars.esp.new_chams_hands_overlay == 1) {
						ImGui::SliderFloat(XorStr("Glow strength ##hands"), &g_Vars.esp.chams_hands_outline_value, 0.f, 100.f, XorStr("%.f"));
					}
					ImGui::Checkbox(XorStr("Hands overlay wireframe"), &g_Vars.esp.chams_hands_outline_wireframe);
					ImGui::Checkbox(XorStr("Draw original model ##hands"), &g_Vars.esp.new_chams_hands_original_model);
				}

				ImGui::Spacing();
				ImGui::EndGroupBox();
			}
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::World() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Environment"), ImVec2(244.f, 327.f)); {
			std::vector<MultiItem_t> worldAdjustment = {
				{ XorStr("Nightmode"), &g_Vars.esp.night_mode },
				{ XorStr("Fullbright"), &g_Vars.esp.fullbright },
				{ XorStr("Skybox Color"), &g_Vars.esp.skybox },
			};
			const char* skyboxes[]{ XorStr("Default"), XorStr("cs_baggage_skybox"), XorStr("cs_tibet"), XorStr("embassy"), XorStr("italy"), XorStr("jungle"), XorStr("nukeblank"), XorStr("office"), XorStr("sky_csgo_cloudy01"), XorStr("sky_csgo_night02"), XorStr("sky_csgo_night02b"), XorStr("sky_dust"), XorStr("sky_venice"), XorStr("vertigo"), XorStr("vietnamsky_descent"), XorStr("Custom") };
			const char* override_textures[]{ XorStr("Disabled"), XorStr("Dev (Grey)"), XorStr("Dev (Orange)"), XorStr("Custom") };
			const char* shot_visualization_options[] = { XorStr("Off"), XorStr("Hitboxes"), XorStr("Chams") };
			const char* tracers[]{ XorStr("Line"), XorStr("Beam") };
			const char* beam_models[]{ XorStr("Laserbeam"), XorStr("Purplelaser"), XorStr("Physbeam") };

			ImGui::MultiCombo(XorStr("World Adjustment"), worldAdjustment);
			if (g_Vars.esp.skybox) {
				ColorPicker_w_name(XorStr("Skybox Color"), g_Vars.esp.skybox_modulation, false, false);
			}
			if (g_Vars.esp.night_mode) {
				ImGui::SliderFloat(XorStr("World brightness"), &g_Vars.esp.world_adjustement_value, 1.f, 100.0f, XorStr("%.0f%%"));
				ImGui::SliderFloat(XorStr("Prop brightness"), &g_Vars.esp.prop_adjustement_value, 1.f, 100.0f, XorStr("%.0f%%"));
				ImGui::SliderFloat(XorStr("Prop transparency"), &g_Vars.esp.transparent_props, 0.f, 100.0f, XorStr("%.0f%%"));
			}
			ImGui::Combo(XorStr("Skybox"), &g_Vars.esp.sky_changer, skyboxes, IM_ARRAYSIZE(skyboxes));
			if (g_Vars.esp.sky_changer == 15) {

				static char sky_custom[64] = "\0";

				if (!g_Vars.esp.custom_skybox.empty())
					strcpy_s(sky_custom, sizeof(sky_custom), g_Vars.esp.custom_skybox.c_str());

				ImGui::Text(XorStr("Name"));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

				if (ImGui::InputText(XorStr("##customsky"), sky_custom, sizeof(sky_custom)))
					g_Vars.esp.custom_skybox = sky_custom;

				ImGui::PopStyleVar();

			}
			ImGui::Combo(XorStr("Override world textures"), &g_Vars.esp.custom_world_textures, override_textures, IM_ARRAYSIZE(override_textures));
			if (g_Vars.esp.custom_world_textures == 3) {
				ImGui::InputText(XorStr("##CustomTexture"), &g_Vars.esp.custom_world_texture_string);
			}
			ImGui::Combo(XorStr("Shot Visualization"), &g_Vars.esp.shot_visualization, shot_visualization_options, IM_ARRAYSIZE(shot_visualization_options));
			switch (g_Vars.esp.shot_visualization) {
			case 0:
			{
				break;
			}
			case 1:
			{
				ColorPicker_w_name(XorStr("Hitboxes color"), g_Vars.esp.hitboxes_color, true, false);
				break;
			}
			case 2:
			{
				const char* shot_chams_mat[] = { XorStr("Glow"), XorStr("Flat") };

				ImGui::Combo(XorStr("Shot Material"), &g_Vars.esp.new_chams_onshot_mat, shot_chams_mat, IM_ARRAYSIZE(shot_chams_mat));
				if (g_Vars.esp.new_chams_onshot_mat == 0) {
					ImGui::SliderFloat(XorStr("Glow strength ##shot"), &g_Vars.esp.new_chams_onshot_mat_glow_value, 0.f, 100.f, XorStr("%.f"));
				}
				ColorPicker_w_name(XorStr("Shot chams color"), g_Vars.esp.hitmatrix_color, true, false);
				ImGui::SliderFloat(XorStr("Expire time ##chams"), &g_Vars.esp.hitmatrix_time, 0.2f, 3.0f, XorStr("%.1f seconds"));
				break;
			}
			}
			ImGui::Checkbox("Modulate fire", &g_Vars.esp.molotov_color_enable);
			if (g_Vars.esp.molotov_color_enable) {
				ColorPicker(XorStr("##ModulateFire"), g_Vars.esp.molotov_color, false, false);
			}
			ImGui::Checkbox(XorStr("Sunset mode"), &g_Vars.esp.sunset_enable);
			if (g_Vars.esp.sunset_enable) {
				ImGui::SliderFloat(XorStr("Sunset y"), &g_Vars.esp.sunset_rot_y, 0.f, 360.0f, XorStr("%.1f"));
				ImGui::SliderFloat(XorStr("Sunset x"), &g_Vars.esp.sunset_rot_x, 0.f, 360.0f, XorStr("%.1f"));
			}
			ImGui::Checkbox(XorStr("Ambient lighting"), &g_Vars.esp.ambient_ligtning);
			if (g_Vars.esp.ambient_ligtning) {
				ColorPicker(XorStr("##AmbientColor"), g_Vars.esp.ambient_ligtning_color, false, false);
			}
			ImGui::Checkbox(XorStr("Tesla Impact"), &g_Vars.esp.tesla_impact);
			ImGui::Checkbox(XorStr("Client Impacts"), &g_Vars.misc.impacts_spoof);
			if (g_Vars.misc.impacts_spoof) {
				ColorPicker(XorStr("##ClientImpactsColor"), g_Vars.esp.client_impacts, true, false);
			}
			ImGui::Checkbox(XorStr("Server Impacts"), &g_Vars.misc.server_impacts_spoof);
			if (g_Vars.misc.server_impacts_spoof) {
				ColorPicker(XorStr("##ServerColor"), g_Vars.esp.server_impacts, true, false);
			}
			ImGui::Checkbox(XorStr("Bullet Tracers"), &g_Vars.esp.beam_enabled);
			if (g_Vars.esp.beam_enabled) {
				ImGui::Checkbox(XorStr("Enemy Tracers"), &g_Vars.esp.beam_enemy_enable);
				if (g_Vars.esp.beam_enemy_enable) {
					ColorPicker(XorStr("Enemy Tracer Color"), g_Vars.esp.beam_color_enemy, false, false);
				}
				ImGui::Checkbox(XorStr("Local Tracers"), &g_Vars.esp.beam_local_enable);
				if (g_Vars.esp.beam_local_enable) {
					ColorPicker(XorStr("Local Tracer Color"), g_Vars.esp.beam_color_local, false, false);
				}

				ImGui::Combo(XorStr("Bullet Tracer Type"), &g_Vars.esp.beam_type, tracers, IM_ARRAYSIZE(tracers));
			}
			if (g_Vars.esp.beam_type == 1 && g_Vars.esp.beam_enabled) {
				ImGui::Combo(XorStr("Beam Type"), &g_Vars.esp.beam_model, beam_models, IM_ARRAYSIZE(beam_models));
			}


			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Weapons"), ImVec2(244.f, 133.f)); {
			const char* glow_types[] = { XorStr("Standard"), XorStr("Pulse"), XorStr("Inner") };
			const char* tracers[]{ XorStr("Icon"), XorStr("Text") };

			ImGui::Checkbox("Name", &g_Vars.esp.dropped_weapons);
			if (g_Vars.esp.dropped_weapons) {
				ImGui::Combo(XorStr("Name type"), &g_Vars.esp.dropped_weapons_font, tracers, IM_ARRAYSIZE(tracers));
			}

			ImGui::Checkbox("Ammo", &g_Vars.esp.dropped_weapons_ammo);
			ColorPicker_w_name(XorStr("Weapons color"), g_Vars.esp.dropped_weapons_color, false, false);
			ImGui::Checkbox(XorStr("Glow"), &g_Vars.esp.glow_weapons);
			ColorPicker(XorStr("##weaponsglowcolor"), g_Vars.esp.glow_weapons_color, true, false);
			if (g_Vars.esp.glow_weapons) {
				ImGui::Combo(XorStr("Glow Type"), &g_Vars.esp.glow_type_weapons, glow_types, IM_ARRAYSIZE(glow_types));
			}


			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Objectives"), ImVec2(244.f, 133.f)); {
			ImGui::Checkbox(XorStr("Bomb timer"), &g_Vars.esp.draw_c4_bar);
			if (g_Vars.esp.draw_c4_bar) {
				ColorPicker(XorStr("##BombColor"), g_Vars.esp.c4_color, false, false);
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Projectiles"), ImVec2(244.f, 327.f)); {
			const char* glow_types[] = { XorStr("Standard"), XorStr("Pulse"), XorStr("Inner") };

			ImGui::Checkbox(XorStr("Enable"), &g_Vars.esp.nades);
			ImGui::Checkbox(XorStr("Glow"), &g_Vars.esp.glow_grenade);
			ColorPicker(XorStr("##grenadesglowcolor"), g_Vars.esp.glow_grenade_color, true, false);
			if (g_Vars.esp.glow_grenade) {
				ImGui::Combo(XorStr("Glow Type"), &g_Vars.esp.glow_type_grenades, glow_types, IM_ARRAYSIZE(glow_types));
			}
			ImGui::Checkbox(XorStr("Molotov timer"), &g_Vars.esp.molotov_timer);
			ImGui::Checkbox(XorStr("Molotov radius"), &g_Vars.esp.molotov_radius);
			ImGui::Checkbox(XorStr("Smoke timer"), &g_Vars.esp.smoke_timer);
			ImGui::Checkbox(XorStr("Smoke radius"), &g_Vars.esp.smoke_radius);

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Visuals() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("View"), ImVec2(244.f, 195.f)); {
			ImGui::Checkbox(XorStr("Viewmodel on scope"), &g_Vars.esp.force_viewmodel_scoped);
			ImGui::Checkbox(XorStr("Aspect ratio"), &g_Vars.esp.aspect_ratio);
			if (g_Vars.esp.aspect_ratio) {
				ImGui::SliderFloat(XorStr("Aspect ratio value"), &g_Vars.esp.aspect_ratio_value, 0.02f, 5.f, XorStr("%.2f"));
			}
			ImGui::SliderFloat(XorStr("Field of view"), &g_Vars.esp.world_fov, 0.f, 200.f, XorStr("%.0f degrees"));
			ImGui::SliderFloat(XorStr("Viewmodel FOV"), &g_Vars.misc.viewmodel_fov, 0.f, 200.f, XorStr("%.0f degrees"));
			ImGui::Checkbox(XorStr("Viewmodel changer"), &g_Vars.misc.viewmodel_change);
			if (g_Vars.misc.viewmodel_change) {
				ImGui::SliderInt(XorStr("Viewmodel X"), &g_Vars.misc.viewmodel_x, -50.f, 50.f, XorStr("%.0f degrees"));
				ImGui::SliderInt(XorStr("Viewmodel Y"), &g_Vars.misc.viewmodel_y, -50.f, 50.f, XorStr("%.0f degrees"));
				ImGui::SliderInt(XorStr("Viewmodel Z"), &g_Vars.misc.viewmodel_z, -50.f, 50.f, XorStr("%.0f degrees"));
			}



			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Removals"), ImVec2(244.f, 265.f)); {
			ImGui::Checkbox(XorStr("Smoke particles"), &g_Vars.esp.remove_smoke);
			ImGui::Checkbox(XorStr("Flash effect"), &g_Vars.esp.remove_flash);
			ImGui::Checkbox(XorStr("Scope"), &g_Vars.esp.remove_scope);
			ImGui::Checkbox(XorStr("Zoom"), &g_Vars.esp.remove_scope_zoom);
			ImGui::Checkbox(XorStr("Recoil shake"), &g_Vars.esp.remove_recoil_shake);
			ImGui::Checkbox(XorStr("Recoil punch"), &g_Vars.esp.remove_recoil_punch);
			ImGui::Checkbox(XorStr("View bob"), &g_Vars.esp.remove_bob);
			ImGui::Checkbox(XorStr("Sleeves"), &g_Vars.esp.remove_sleeves);
			ImGui::Checkbox(XorStr("Post processing"), &g_Vars.esp.remove_post_proccesing);

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Hitmarker"), ImVec2(244.f, 195.f)); {
			std::vector<MultiItem_t> hitmarkers = {
					{ XorStr("World"), &g_Vars.esp.visualize_hitmarker_world },
					{ XorStr("Screen"), &g_Vars.esp.vizualize_hitmarker },
					{ XorStr("Damage"), &g_Vars.esp.visualize_damage },
			};

			ImGui::MultiCombo(XorStr("Hitmarkers"), hitmarkers);

			ImGui::Checkbox(XorStr("Hitsound"), &g_Vars.misc.hitsound);
			if (g_Vars.misc.hitsound) {
				const char* hitsounds[] = { XorStr("Default"), XorStr("Custom"), XorStr("Bameware"), XorStr("Vader") };
				ImGui::Combo(XorStr("Type"), &g_Vars.misc.hitsound_type, hitsounds, IM_ARRAYSIZE(hitsounds));

				if (g_Vars.misc.hitsound_type == 1) {
					static char custom_hitsound[64] = "\0";

					if (!g_Vars.misc.custom_hitsound.empty())
						strcpy_s(custom_hitsound, sizeof(custom_hitsound), g_Vars.misc.custom_hitsound.c_str());

					if (ImGui::InputText(XorStr("##Custom Hitsound"), custom_hitsound, sizeof(custom_hitsound)))
						g_Vars.misc.custom_hitsound = custom_hitsound;

					ImGui::SliderFloat(XorStr("Hitsound volume"), &g_Vars.misc.hitsound_volume, 1.f, 100.f, XorStr("%.f"));
				}
			}
			ImGui::Checkbox(XorStr("F12 sound (microphone)"), &g_Vars.misc.f12_kill_sound);
			ImGui::Checkbox(XorStr("Killsound"), &g_Vars.misc.killsound);
			if (g_Vars.misc.killsound) {
				static char custom_killsound[64] = "\0";

				if (!g_Vars.misc.custom_killsound.empty())
					strcpy_s(custom_killsound, sizeof(custom_killsound), g_Vars.misc.custom_killsound.c_str());

				if (ImGui::InputText(XorStr("##Custom Killsound"), custom_killsound, sizeof(custom_killsound)))
					g_Vars.misc.custom_killsound = custom_killsound;

				ImGui::SliderFloat(XorStr("Killsound volume"), &g_Vars.misc.killsound_volume, 1.f, 100.f, XorStr("%.f"));
			}

			ImGui::Checkbox(XorStr("Tesla on Kill"), &g_Vars.esp.tesla_kill);
			if (g_Vars.esp.tesla_kill) {
				ColorPicker(XorStr("tesla kill color"), g_Vars.esp.tesla_kill_color, false, false);
				ImGui::SliderFloat(XorStr("Tesla Width"), &g_Vars.esp.tesla_kill_width, 0.f, 10.f, XorStr("%.f"));
				ImGui::SliderFloat(XorStr("Tesla Beams"), &g_Vars.esp.tesla_kill_beams, 0.f, 20.f, XorStr("%.f"));
				ImGui::SliderFloat(XorStr("Tesla Radius"), &g_Vars.esp.tesla_kill_radius, 0.f, 200.f, XorStr("%.f"));
				ImGui::SliderFloat(XorStr("Tesla Time"), &g_Vars.esp.tesla_kill_time, 0.1f, 3.f, XorStr("%.1f"));
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Other"), ImVec2(244.f, 265.f)); {

			ImGui::Checkbox(XorStr("Thirdperson"), &g_Vars.misc.third_person);
			ImGui::SameLine(213.f);
			ImGui::Keybind(XorStr("##Thirdperson Key"), &g_Vars.misc.third_person_bind.key, &g_Vars.misc.third_person_bind.cond);
			if (g_Vars.misc.third_person) {
				ImGui::Checkbox(XorStr("Disable on grenade"), &g_Vars.misc.third_person_on_grenade);
				ImGui::SliderFloat(XorStr("Thirdperson distance"), &g_Vars.misc.third_person_dist, 0.f, 250.f, XorStr("%.0f%%"));
			}
			ImGui::Checkbox(XorStr("Skip occlusion"), &g_Vars.esp.skip_occulusion);
			ImGui::Checkbox(XorStr("Preserve killfeed"), &g_Vars.esp.preserve_killfeed);
			ImGui::Checkbox("Force crosshair", &g_Vars.esp.force_sniper_crosshair);
			ImGui::Checkbox(XorStr("Penetration crosshair"), &g_Vars.esp.autowall_crosshair);
			ImGui::Checkbox(XorStr("Spread crosshair"), &g_Vars.esp.spread_crosshair);
			if (g_Vars.esp.spread_crosshair) {
				ColorPicker(XorStr("##SpreadCrosshairColor"), g_Vars.esp.spread_crosshair_color, true, false);
			}
			ImGui::Checkbox(XorStr("Halo"), &g_Vars.esp.halo_above_head);
			if (g_Vars.esp.halo_above_head) {
				ColorPicker(XorStr("Halo Color"), g_Vars.esp.halo_above_head_color, true, false);
			}
			ImGui::Checkbox(XorStr("Grenade trajectory"), &g_Vars.esp.NadePred);
			if (g_Vars.esp.NadePred) {
				ColorPicker(XorStr("##GrenadeColor"), g_Vars.esp.nade_pred_color, false, false);
			}
			ImGui::Checkbox(XorStr("Grenade warning"), &g_Vars.esp.Grenadewarning);
			if (g_Vars.esp.Grenadewarning) {
				std::vector<MultiItem_t> Draw_options = {
					{ XorStr("Icon"), &g_Vars.esp.Grenadewarning_icon },
					{ XorStr("Circle"), &g_Vars.esp.Grenadewarning_circle },
					{ XorStr("Tracer"), &g_Vars.esp.Grenadetracer },
				};

				ImGui::MultiCombo(XorStr("Draw options"), Draw_options);

				if (g_Vars.esp.Grenadewarning_icon) {
					ColorPicker_w_name(XorStr("Icon color"), g_Vars.esp.Grenadewarning_icon_color, false, false);
				}
				if (g_Vars.esp.Grenadewarning_circle) {
					ColorPicker_w_name(XorStr("Circle color"), g_Vars.esp.Grenadewarning_circle_color, false, false);
					ColorPicker_w_name(XorStr("Circle warning color"), g_Vars.esp.Grenadewarning_circlewarning_color, false, false);
				}
				if (g_Vars.esp.Grenadetracer) {
					ColorPicker_w_name(XorStr("Tracer color"), g_Vars.esp.Grenadewarning_tracer_color, true, false);
				}

			}
			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Skins() {
	ImGuiStyle& style = ImGui::GetStyle();

	const char* knife_models[]{ XorStr("Bayonet"), XorStr("Bowie"), XorStr("Butterfly"), XorStr("Falchion"), XorStr("Flip"), XorStr("Gut"), XorStr("Tactical"), XorStr("Karambit"), XorStr("M9 Bayonet"), XorStr("Shadow Daggers") };
	const char* glove_models[]{ XorStr("Default"), XorStr("Bloodhound"), XorStr("Sport"), XorStr("Driver"), XorStr("Hand Wraps"), XorStr("Motorcycle"), XorStr("Specialist") };
	const char* bloodhound_skins[]{ XorStr("Charred"), XorStr("Snakebite"), XorStr("Bronzed"), XorStr("Guerilla") };
	const char* sport_skins[]{ XorStr("Hedge Maze"), XorStr("Pandoras Box"), XorStr("Superconductor"), XorStr("Arid"), XorStr("Vice"), XorStr("Omega"), XorStr("Amphibious"), XorStr("Bronze Morph") };
	const char* driver_skins[]{ XorStr("Lunar Weave"), XorStr("Convoy"), XorStr("Crimson Weave"), XorStr("Diamondback"), XorStr("Overtake"), XorStr("Racing Green"), XorStr("King Snake"), XorStr("Imperial Plaid") };
	const char* handwarps_skins[]{ XorStr("Leather"), XorStr("Spruce DDPAT"), XorStr("Slaughter"), XorStr("Badlands"), XorStr("Cobalt Skulls"), XorStr("Overprint"), XorStr("Duct Tape"), XorStr("Arboreal") };
	const char* motorcycle_skins[]{ XorStr("Eclipse"), XorStr("Spearmint"), XorStr("Boom!"), XorStr("Cool Mint"), XorStr("Turtle"), XorStr("Transport"), XorStr("Polygon"), XorStr("POW!") };
	const char* specialist_skins[]{ XorStr("Forest DDPAT"), XorStr("Crimson Kimono"), XorStr("Emerald Web"), XorStr("Foundation"), XorStr("Crimson Web"), XorStr("Buckshot"), XorStr("Fade"), XorStr("Mogul") };

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Main"), ImVec2(244.f, 480.f)); {
			ImGui::Checkbox(XorStr("Enable"), &g_Vars.misc.enable_skins);
			if (g_Vars.misc.enable_skins) {
				ImGui::Combo(XorStr("Knife Model"), &g_Vars.misc.knife_model, knife_models, IM_ARRAYSIZE(knife_models));
				//ImGui::InputInt(XorStr("Knife Skin"), &g_Vars.misc.knife_skin);

				ImGui::Checkbox(XorStr("Enable Gloves"), &g_Vars.misc.enable_gloves);

				if (g_Vars.misc.enable_gloves) {
					ImGui::Combo(XorStr("Glove Model"), &g_Vars.misc.gloves_model, glove_models, IM_ARRAYSIZE(glove_models));

					switch (g_Vars.misc.gloves_model) {
					case 1:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, bloodhound_skins, IM_ARRAYSIZE(bloodhound_skins));
						break;
					case 2:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, sport_skins, IM_ARRAYSIZE(sport_skins));
						break;
					case 3:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, driver_skins, IM_ARRAYSIZE(driver_skins));
						break;
					case 4:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, handwarps_skins, IM_ARRAYSIZE(handwarps_skins));
						break;
					case 5:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, motorcycle_skins, IM_ARRAYSIZE(motorcycle_skins));
						break;
					case 6:
						ImGui::Combo(XorStr("Glove Skin"), &g_Vars.misc.gloves_skin, specialist_skins, IM_ARRAYSIZE(specialist_skins));
						break;
					}
				}


				if (ImGui::Button(XorStr("Apply Skins")))
				{
					g_Vars.m_global_skin_changer.m_update_skins = true;
				}
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}
		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Weapon skins"), ImVec2(244.f, 480.f)); {
			if (g_Vars.misc.enable_skins) { // retarded ass code dont look :D
				const char* weapon_list[]{ XorStr("Glock-18"), XorStr("P2000"), XorStr("USP-S"), XorStr("P250"), XorStr("Dual Berettas"), XorStr("Tec-9"), XorStr("Five-SeveN"), XorStr("CZ75-Auto"), XorStr("Desert Eagle"), XorStr("R8 Revolver"), XorStr("Nova"), XorStr("XM1014"), XorStr("MAG-7"), XorStr("Sawed-Off"), XorStr("MAC-10"), XorStr("MP9"), XorStr("UMP-45"), XorStr("MP7"), XorStr("PP-Bizon"), XorStr("P90"), XorStr("Galil AR"), XorStr("FAMAS"), XorStr("AK-47"), XorStr("M4A4"), XorStr("M4A1-S"), XorStr("SG 553"), XorStr("AUG"), XorStr("SSG 08"), XorStr("AWP"), XorStr("G3SG1"), XorStr("SCAR-20"), XorStr("M249"), XorStr("Negev"), XorStr("Knife") };

				ImGui::Combo(XorStr("Weapon"), &g_Vars.misc.weapon_select, weapon_list, IM_ARRAYSIZE(weapon_list));

				static char uname[128];
				ImGui::InputText(XorStr("search"), uname, 128);

				ImGui::ListBoxHeader(XorStr("##skins"), ImVec2(250, ImGui::GetWindowSize().y - 80));

				if (!skin_kits.empty())
				{
					std::sort(skin_kits.begin(), skin_kits.end());

					switch (g_Vars.misc.weapon_select) {
					case 0:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.glock_skin = skin_kits[selected_skin].id;
						}

						break;
					case 1:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.p2k_skin = skin_kits[selected_skin].id;
						}
						break;
					case 2:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.usp_skin = skin_kits[selected_skin].id;
						}
						break;
					case 3:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.p250_skin = skin_kits[selected_skin].id;
						}
						break;
					case 4:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.elite_skin = skin_kits[selected_skin].id;
						}
						break;
					case 5:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.tec9_skin = skin_kits[selected_skin].id;
						}
						break;
					case 6:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.fiveseven_skin = skin_kits[selected_skin].id;
						}
						break;
					case 7:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.cz75a_skin = skin_kits[selected_skin].id;
						}
						break;
					case 8:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.deagle_skin = skin_kits[selected_skin].id;
						}
						break;
					case 9:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.revolver_skin = skin_kits[selected_skin].id;
						}
						break;
					case 10:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.nova_skin = skin_kits[selected_skin].id;
						}
						break;
					case 11:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.xm1014_skin = skin_kits[selected_skin].id;
						}
						break;
					case 12:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.mag7_skin = skin_kits[selected_skin].id;
						}
						break;
					case 13:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.sawedoff_skin = skin_kits[selected_skin].id;
						}
						break;
					case 14:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.mac10_skin = skin_kits[selected_skin].id;
						}
						break;
					case 15:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.mp9_skin = skin_kits[selected_skin].id;
						}
						break;
					case 16:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.ump45_skin = skin_kits[selected_skin].id;
						}
						break;
					case 17:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.mp7_skin = skin_kits[selected_skin].id;
						}
						break;
					case 18:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.bizon_skin = skin_kits[selected_skin].id;
						}
						break;
					case 19:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.p90_skin = skin_kits[selected_skin].id;
						}
						break;
					case 20:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.galilar_skin = skin_kits[selected_skin].id;
						}
						break;
					case 21:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.famas_skin = skin_kits[selected_skin].id;
						}
						break;
					case 22:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.ak47_skin = skin_kits[selected_skin].id;
						}
						break;
					case 23:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.m4a1_skin = skin_kits[selected_skin].id;
						}
						break;
					case 24:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.m4a1s_skin = skin_kits[selected_skin].id;
						}
						break;
					case 25:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.sg556_skin = skin_kits[selected_skin].id;
						}
						break;
					case 26:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.aug_skin = skin_kits[selected_skin].id;
						}
						break;
					case 27:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.ssg08_skin = skin_kits[selected_skin].id;
						}
						break;
					case 28:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.awp_skin = skin_kits[selected_skin].id;
						}
						break;
					case 29:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.g3sg1_skin = skin_kits[selected_skin].id;
						}
						break;
					case 30:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.scar20_skin = skin_kits[selected_skin].id;
						}
						break;
					case 31:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.m249_skin = skin_kits[selected_skin].id;
						}
						break;
					case 32:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.negev_skin = skin_kits[selected_skin].id;
						}
						break;
					case 33:
						for (auto i = 0; i < skin_kits.size(); ++i) {
							static int selected_skin = 0;

							const auto item_selected = (i == selected_skin);

							auto search = std::string(uname);
							auto name = std::string(skin_kits[i].name);

							std::transform(search.begin(), search.end(), search.begin(), ::tolower);
							std::transform(name.begin(), name.end(), name.begin(), ::tolower);

							if (search != XorStr("") && name.find(search) == std::string::npos)
								continue;

							if (skin_kits[i].name == XorStr("-"))
								continue;

							ImGui::PushID(i);
							if (ImGui::Selectable(skin_kits[i].name.c_str(), item_selected))
							{
								selected_skin = i;
							}
							ImGui::PopID();

							g_Vars.misc.knife_skin = skin_kits[selected_skin].id;
						}
						break;
					}
				}

				ImGui::ListBoxFooter();

				//ImGui::InputInt(XorStr("Usp Skin"), &g_Vars.misc.usp_skin);
				//ImGui::InputInt(XorStr("P2000 Skin"), &g_Vars.misc.p2k_skin);
				//ImGui::InputInt(XorStr("Glock Skin"), &g_Vars.misc.glock_skin);
				//ImGui::InputInt(XorStr("P250 Skin"), &g_Vars.misc.p250_skin);
				//ImGui::InputInt(XorStr("Five7 Skin"), &g_Vars.misc.fiveseven_skin);
				//ImGui::InputInt(XorStr("Tec9 Skin"), &g_Vars.misc.tec9_skin);
				//ImGui::InputInt(XorStr("CZ75A Skin"), &g_Vars.misc.cz75a_skin);
				//ImGui::InputInt(XorStr("Elite Skin"), &g_Vars.misc.elite_skin);
				//ImGui::InputInt(XorStr("Deagle Skin"), &g_Vars.misc.deagle_skin);
				//ImGui::InputInt(XorStr("R8 Skin"), &g_Vars.misc.revolver_skin);
				//ImGui::InputInt(XorStr("Famas Skin"), &g_Vars.misc.famas_skin);
				//ImGui::InputInt(XorStr("Galil Skin"), &g_Vars.misc.galilar_skin);
				//ImGui::InputInt(XorStr("M4a1 Skin"), &g_Vars.misc.m4a1_skin);
				//ImGui::InputInt(XorStr("M4a1s Skin"), &g_Vars.misc.m4a1s_skin);
				//ImGui::InputInt(XorStr("Ak47 Skin"), &g_Vars.misc.ak47_skin);
				//ImGui::InputInt(XorStr("Sg556 Skin"), &g_Vars.misc.sg556_skin);
				//ImGui::InputInt(XorStr("Aug Skin"), &g_Vars.misc.aug_skin);
				//ImGui::InputInt(XorStr("Ssg08 Skin"), &g_Vars.misc.ssg08_skin);
				//ImGui::InputInt(XorStr("Awp Skin"), &g_Vars.misc.awp_skin);
				//ImGui::InputInt(XorStr("Scar20 Skin"), &g_Vars.misc.scar20_skin);
				//ImGui::InputInt(XorStr("G3sg1 Skin"), &g_Vars.misc.g3sg1_skin);
				//ImGui::InputInt(XorStr("Sawoff Skin"), &g_Vars.misc.sawedoff_skin);
				//ImGui::InputInt(XorStr("M249 Skin"), &g_Vars.misc.m249_skin);
				//ImGui::InputInt(XorStr("Negev Skin"), &g_Vars.misc.negev_skin);
				//ImGui::InputInt(XorStr("Mag7 Skin"), &g_Vars.misc.mag7_skin);
				//ImGui::InputInt(XorStr("Xm Skin"), &g_Vars.misc.xm1014_skin);
				//ImGui::InputInt(XorStr("Nova Skin"), &g_Vars.misc.nova_skin);
				//ImGui::InputInt(XorStr("Bizon Skin"), &g_Vars.misc.bizon_skin);
				//ImGui::InputInt(XorStr("Mp7 Skin"), &g_Vars.misc.mp7_skin);
				//ImGui::InputInt(XorStr("Mp9 Skin"), &g_Vars.misc.mp9_skin);
				//ImGui::InputInt(XorStr("Mac10 Skin"), &g_Vars.misc.mac10_skin);
				//ImGui::InputInt(XorStr("P90 Skin"), &g_Vars.misc.p90_skin);
				//ImGui::InputInt(XorStr("Ump45 Skin"), &g_Vars.misc.ump45_skin);
			}
			ImGui::Spacing();
			ImGui::EndGroupBox();
		}
		ImGui::EndGroup();
	}
}

void Menu::Misc() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Matchmaking"), ImVec2(244.f, 100.f)); {
			ImGui::Checkbox("Anti untrusted", &g_Vars.misc.anti_untrusted);
			ImGui::Checkbox("Unlock inventory", &g_Vars.misc.unlock_inventory);

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Movement"), ImVec2(244.f, 360.f)); {
			ImGui::Checkbox("Bunnyhop", &g_Vars.misc.autojump);
			ImGui::Checkbox(XorStr("Auto Strafe"), &g_Vars.misc.autostrafer);
			ImGui::Checkbox(XorStr("WASD-Strafer"), &g_Vars.misc.autostrafer_wasd);
			ImGui::Checkbox("Fast stop", &g_Vars.misc.quickstop);
			ImGui::Checkbox("Infinite duck stamina", &g_Vars.misc.fastduck);
			ImGui::Checkbox(XorStr("Slide Walk"), &g_Vars.misc.slide_walk);

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {

		ImGui::BeginGroupBox(XorStr("Misc"), ImVec2(244.f, 480.f)); {
			std::vector<MultiItem_t> notifications = {
				{ XorStr("Damage dealt"), &g_Vars.esp.event_dmg },
				{ XorStr("Damage taken"), &g_Vars.esp.event_harm },
				{ XorStr("Misses"), &g_Vars.esp.event_resolver },
			};

			ImGui::MultiCombo(XorStr("Event logs"), notifications);

			const char* clantag_options[] = { XorStr("Off"), XorStr("Default"), XorStr("Custom") };
			ImGui::Combo(XorStr("Clantag"), &g_Vars.misc.clantag_changer, clantag_options, IM_ARRAYSIZE(clantag_options));
			if (g_Vars.misc.clantag_changer == 2) {
				ImGui::InputText(XorStr("##Custom Clantag"), &g_Vars.misc.custom_clantag);
			}

			std::vector<MultiItem_t> local_extras = {
				{ XorStr("Zero Pitch on Land"), &g_Vars.esp.zeropitch },
				{ XorStr("Minion"), &g_Vars.esp.break_bones },
			};

			ImGui::MultiCombo(XorStr("Local Extras"), local_extras);

			//std::vector<MultiItem_t> indicators = {
			//	{ XorStr("PING"), &g_Vars.esp.indicator_ping },
			//	{ XorStr("DT"), &g_Vars.esp.indicator_exploits },
			//	{ XorStr("LBY"), &g_Vars.esp.indicator_lby },
			//	{ XorStr("LC"), &g_Vars.esp.indicator_lagcomp },
			//	{ XorStr("DMG"), &g_Vars.esp.indicator_mindmg },
			//};

			//ImGui::MultiCombo(XorStr("Indicators"), indicators);

			const char* models[]{ XorStr("Off"), XorStr("Darth Vader"), XorStr("StormTrooper"), XorStr("Custom") };

#if defined(BETA_MODE) || defined(DEV)
			ImGui::Text(XorStr("----------beta/debug----------"));
			ImGui::Checkbox(XorStr("Resolver flags"), &g_Vars.misc.resolver_flags);
			ImGui::Checkbox(XorStr("Hide debug flags"), &g_Vars.misc.undercover_flags);
			ImGui::Text(XorStr("----------beta/debug----------"));
#endif

#ifdef DEV
			ImGui::Text(XorStr("----------dev----------"));
			ImGui::Checkbox(XorStr("Anonymous mode"), &g_Vars.misc.anonymous_mode_cope);
			if (g_Vars.misc.anonymous_mode_cope) {
				ImGui::Text(XorStr("NOTE: REJOIN IS NEEDED TO APPLY"));
			}
			ImGui::Text(XorStr("----------dev----------"));
#endif

			ImGui::Checkbox(XorStr("Model Changer"), &g_Vars.misc.model_changer);
			if (g_Vars.misc.model_changer) {
				ImGui::Combo(XorStr("Models"), &g_Vars.misc.models, models, IM_ARRAYSIZE(models));

				if (g_Vars.misc.models == 3) {
					ImGui::InputText(XorStr("##Custom Model"), &g_Vars.misc.custom_model);
				}
			}

			ImGui::Checkbox(XorStr("Auto-Peek"), &g_Vars.misc.autopeek);
			ImGui::SameLine(213.f);
			ImGui::Keybind(XorStr("##AutoPeekKey"), &g_Vars.misc.autopeek_bind.key, &g_Vars.misc.autopeek_bind.cond);

			if (g_Vars.misc.autopeek) {
				ColorPicker_w_name(XorStr("Auto-Peek color"), g_Vars.misc.autopeek_color, false, false);
			}


			ImGui::Checkbox(XorStr("Fake-Duck"), &g_Vars.misc.fakeduck);
			ImGui::SameLine(213.f);
			ImGui::Keybind(XorStr("##FDkey"), &g_Vars.misc.fakeduck_bind.key, &g_Vars.misc.fakeduck_bind.cond);

			ImGui::Checkbox(XorStr("Instant bomb defuse"), &g_Vars.misc.balls);

			ImGui::Checkbox(XorStr("Buy Bot"), &g_Vars.misc.autobuy_enabled);

			const char* first_weapon_str[]{ XorStr("None"), XorStr("SCAR-20 / G3SG1"), XorStr("SSG-08"), XorStr("AWP") };
			const char* second_weapon_str[]{ XorStr("None"), XorStr("Dualies"), XorStr("Desert Eagle / R8 Revolver"), XorStr("Tec-9 / Five-SeveN") };

			std::vector<MultiItem_t> other_weapon_conditions = {
				{ XorStr("Armor"), &g_Vars.misc.autobuy_armor },
				{ XorStr("Flashbang"), &g_Vars.misc.autobuy_flashbang },
				{ XorStr("HE Grenade"), &g_Vars.misc.autobuy_hegrenade },
				{ XorStr("Molotov"), &g_Vars.misc.autobuy_molotovgrenade },
				{ XorStr("Smoke"), &g_Vars.misc.autobuy_smokegreanade },
				{ XorStr("Decoy"), &g_Vars.misc.autobuy_decoy },
				{ XorStr("Taser"), &g_Vars.misc.autobuy_zeus },
				{ XorStr("Defuse kit"), &g_Vars.misc.autobuy_defusekit },
			};

			if (g_Vars.misc.autobuy_enabled) {
				ImGui::Combo(XorStr("Primary Weapon"), &g_Vars.misc.autobuy_first_weapon, first_weapon_str, IM_ARRAYSIZE(first_weapon_str));
				ImGui::Combo(XorStr("Secondary Weapon"), &g_Vars.misc.autobuy_second_weapon, second_weapon_str, IM_ARRAYSIZE(second_weapon_str));
				ImGui::MultiCombo(std::string(XorStr("Utility")).c_str(), other_weapon_conditions);
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Configuration() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	static int selected_cfg;
	static std::string config_name;
	static std::vector<std::string> cfg_list;
	static bool initialise_configs = true;
	bool reinit = false;
	if (initialise_configs || (GetTickCount() % 3000) == 0) {
		cfg_list = ConfigManager::GetConfigs();
		initialise_configs = false;
		reinit = true;
	}

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("File"), ImVec2(244.f, 245.f)); {
			//ImGui::SetNextItemWidth(231.f);

			ImGui::Text(XorStr("Name")); ImGui::SameLine(); // my ghetto way of having the name first then the input box
			ImGui::InputText(XorStr("##name"), &config_name);


			if (!cfg_list.empty()) {
				for (auto penis : cfg_list)
					if (ImGui::Selectable(penis.c_str(), ConfigManager::GetConfigID(penis) == selected_cfg))
						selected_cfg = ConfigManager::GetConfigID(penis);
			}
			else
				ImGui::Text(XorStr("No configs"));

			if (reinit) {
				if (selected_cfg >= cfg_list.size())
					selected_cfg = cfg_list.size() - 1;

				if (selected_cfg < 0)
					selected_cfg = 0;
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Configuration"), ImVec2(244.f, 215.f)); {
				if (!cfg_list.empty()) {
					if (ImGui::Button(XorStr("Save"), ImVec2(213.f, 20.f)))
					{
						ImGui::OpenPopup(XorStr("Confirmation_Save"));
					}

					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Save")))
					{
						if (ImGui::Button(XorStr("Are you sure?")))
						{
							//LuaConfigSystem::Save();
							ConfigManager::SaveConfig(cfg_list.at(selected_cfg));
							ILoggerEvent::Get()->PushEvent(XorStr("Saved config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
							ImGui::CloseCurrentPopup();
						}
						//if (ImGui::Button(XorStr("Cancel"), ImVec2(120, 0)))
						//{
						//	ImGui::CloseCurrentPopup();
						//}
						ImGui::EndPopup();
					}

					if (ImGui::Button(XorStr("Load"), ImVec2(213.f, 20.f)))
					{
						ImGui::OpenPopup(XorStr("Confirmation_Load"));
					}

					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Load")))
					{
						if (ImGui::Button(XorStr("Are you sure?")))
						{
							if (selected_cfg <= cfg_list.size() && selected_cfg >= 0) {
								ConfigManager::ResetConfig();

								//LuaConfigSystem::Load();
								ConfigManager::LoadConfig(cfg_list.at(selected_cfg));
								ILoggerEvent::Get()->PushEvent(XorStr("Loaded config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
								g_Vars.m_global_skin_changer.m_update_skins = true;
								g_Vars.m_global_skin_changer.m_update_gloves = true;
								ImGui::CloseCurrentPopup();
							}
						}
						ImGui::EndPopup();
					}

					if (ImGui::Button(XorStr("Delete"), ImVec2(213.f, 20.f)))
					{
						ImGui::OpenPopup(XorStr("Confirmation_Delete"));
					}

					if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Delete")))
					{
						if (ImGui::Button(XorStr("Are you sure?")))
						{
							ConfigManager::RemoveConfig(cfg_list.at(selected_cfg));
							cfg_list = ConfigManager::GetConfigs();
							ILoggerEvent::Get()->PushEvent(XorStr("Deleted config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}

				if (ImGui::Button(XorStr("Create"), ImVec2(213.f, 20.f))) {
					if (!config_name.empty()) {
						ConfigManager::CreateConfig(config_name);
						cfg_list = ConfigManager::GetConfigs();
					}
					else
						ILoggerEvent::Get()->PushEvent(XorStr("Create Error! Make sure you have typed a name for the config."), FloatColor(1.f, 0.f, 0.f), true, XorStr(""));
				}

				if (ImGui::Button(XorStr("Reset"), ImVec2(213.f, 20.f))) {
					ImGui::OpenPopup(XorStr("Confirmation_Reset"));
				}

				if (ImGui::BeginPopupContextItem(XorStr("Confirmation_Reset")))
				{
					if (ImGui::Button(XorStr("Are you sure?")))
					{
						ConfigManager::ResetConfig();
						ILoggerEvent::Get()->PushEvent(XorStr("Reset config"), FloatColor(1.f, 1.f, 1.f), true, XorStr(""));
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				if (ImGui::Button(XorStr("Open config folder"), ImVec2(213.f, 20.f)))
				{
					ConfigManager::OpenConfigFolder();
				}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox("Settings", ImVec2(244.f, 480.f)); {
			
			ColorPicker_w_name(XorStr("Accent color"), g_Vars.misc.accent_colorz, false, false);
			ImGui::Checkbox(XorStr("Watermark"), &g_Vars.misc.watermark);
			ImGui::Checkbox(XorStr("Keybinds list"), &g_Vars.esp.keybind_window_enabled);
			ImGui::Checkbox(XorStr("Spectators list"), &g_Vars.esp.spec_window_enabled);

#ifdef DEV
			if (ImGui::Button(XorStr("Unload"), ImVec2(213.f, 20.f)))
			{
				g_Vars.globals.hackUnload = true;
			}
#endif

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Scripts() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("File"), ImVec2(244.f, 275.f)); {

			static char lname2[128];
			ImGui::InputText(XorStr("search"), lname2, 128);

			for (auto s : g_lua.scripts)
			{
				auto search = std::string(lname2);
				auto name = std::string(s.c_str());

				std::transform(search.begin(), search.end(), search.begin(), ::tolower);
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);
				if (search != XorStr("") && name.find(search) == std::string::npos)
					continue;

				if (ImGui::Selectable(s.c_str(), g_lua.loaded.at(g_lua.get_script_id(s)), NULL, ImVec2(0, 0))) {
					auto scriptId = g_lua.get_script_id(s);
					if (g_lua.loaded.at(scriptId)) g_lua.unload_script(scriptId); else g_lua.load_script(scriptId);
				}
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::BeginGroupBox(XorStr("Options"), ImVec2(244.f, 185.f)); {

			ImGui::Checkbox(XorStr("Allow HTTP Requests"), &g_Vars.misc.lua_allow_http_requests);

			if (ImGui::Button(XorStr("Refresh scripts"), ImVec2(213.f, 20.f))) g_lua.refresh_scripts();

			if (ImGui::Button(XorStr("Reload active"), ImVec2(213.f, 20.f))) g_lua.reload_all_scripts();

			if (ImGui::Button(XorStr("Unload all"), ImVec2(213.f, 20.f))) g_lua.unload_all_scripts();

			if (ImGui::Button(XorStr("Open scripts folder"), ImVec2(213.f, 20.f)))
			{
				ConfigManager::OpenScriptsFolder();
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox("Elements", ImVec2(244.f, 480.f)); {

			for (auto& current : g_lua.scripts)
			{
				auto& items = g_lua.items.at(g_lua.get_script_id(current));

				for (auto& item : items)
				{
					switch (item.second.type)
					{
					case NEXT_LINE:
						break;
					case CHECK_BOX:
						ImGui::Checkbox(item.first.c_str(), &LuaConfigSystem::C_BOOL[item.second.key]);
						break;
					case SLIDER:
						ImGui::SliderFloat(item.first.c_str(), &LuaConfigSystem::C_FLOAT[item.second.key], (float)item.second.slider_float_min, (float)item.second.slider_float_max, item.second.format.c_str());
						break;
					case COLOR_PICKER:
						ColorPicker_w_name(item.first.c_str(), LuaConfigSystem::C_COLOR[item.second.key], true, false);
						break;
					case TEXT:
						ImGui::Text(item.first.c_str());
						break;
					case BUTTON:
						if (ImGui::Button(item.first.c_str(), ImVec2(213.f, 20.f)))
						{
							LuaConfigSystem::C_BOOL[item.second.key] = true;
						}
						else
							LuaConfigSystem::C_BOOL[item.second.key] = false;
						break;
					}
				}
			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}

void Menu::Playerlist() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.ItemSpacing = ImVec2(0.f, 24.f);

	static std::vector <Player_list_data> players;
	static auto current_player = 0;

	// LEFT GROUP -------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(193.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox(XorStr("Players"), ImVec2(244.f, 480.f)); {

			if (!g_Vars.globals.player_list.refreshing)
			{
				players.clear();

				for (auto player : g_Vars.globals.player_list.players)
					players.emplace_back(player);
			}

			static char uname[128];
			ImGui::InputText(XorStr("search"), uname, 128);

			ImGui::ListBoxHeader(XorStr("##players"), ImVec2(0, ImGui::GetWindowSize().y - 80));

			if (!players.empty())
			{
				std::vector <std::string> player_names;

				for (auto player : players) {
					auto search = std::string(uname);
					auto name = std::string(player.name);

					std::transform(search.begin(), search.end(), search.begin(), ::tolower);
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);

					if (search != XorStr("") && name.find(search) == std::string::npos)
						continue;

					player_names.emplace_back(player.name);
				}

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				ImGui::ListBoxConfigArray(XorStr("##PLAYERLIST"), &current_player, player_names, 14);
				ImGui::PopStyleVar();
			}

			ImGui::ListBoxFooter();
			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	// RIGHT GROUP ------------------------------------------------------------------------
	ImGui::SetCursorPos(ImVec2(457.f, 21.f));
	ImGui::BeginGroup(); {
		ImGui::BeginGroupBox("Settings", ImVec2(244.f, 480.f)); {
			if (!players.empty())
			{
				if (current_player >= players.size())
					current_player = players.size() - 1;


				ImGui::Checkbox(XorStr("Whitelist"), &g_Vars.globals.player_list.white_list[players.at(current_player).i]);
				ImGui::Checkbox(XorStr("Override Pitch"), &g_Vars.globals.player_list.override_pitch[players.at(current_player).i]);
				if (g_Vars.globals.player_list.override_pitch[players.at(current_player).i]) {
					ImGui::SliderFloat(XorStr("Pitch Angle"), &g_Vars.globals.player_list.override_pitch_slider[players.at(current_player).i], -89.f, 89.f, XorStr("%1.f degrees"));
				}

			}

			ImGui::Spacing();
			ImGui::EndGroupBox();
		}

		ImGui::EndGroup();
	}

	style.ItemSpacing = ImVec2(4.f, 4.f);
}