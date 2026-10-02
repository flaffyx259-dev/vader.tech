#pragma once
#include "IMGAY/imgui.h"
#include "IMGAY/PostProcessing.h"

extern IDirect3DTexture9* logo_nuts;

namespace Menu {
	inline int tab = 0;
	inline int playerTab = 0;
	inline int antiaimTab = 0;
	inline int aimTab = 0;
	inline bool opened = false;
	inline bool initialized = false;
	inline bool Loaded = false;
	inline ImVec2 menu_position;
	inline int windowFlags = (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	inline struct Fonts_t {
		ImFont* robotoTitle;
		ImFont* roboto;
		ImFont* icons;
		ImFont* iconsLarge;
		ImFont* weaponIcons;
		ImFont* StarWars;
		ImFont* interfaces;
		ImFont* watermark;
		ImFont* cs;
		ImFont* cs_huge;
	} fonts;

	void Loading() noexcept;
	void Render() noexcept;
	void Shutdown() noexcept;
	bool Initialize(IDirect3DDevice9*) noexcept;

	void Ragebot();
	void AntiAim();
	void Players();
	void World();
	void Visuals();
	void Skins();
	void Misc();
	void Configuration();
	void Scripts();
	void Playerlist();
}