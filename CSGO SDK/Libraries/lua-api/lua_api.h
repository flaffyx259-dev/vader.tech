#pragma once
#include <filesystem>
#include <lua-wrapper/include/lua.hpp>
#include <sol/sol.hpp>
#include "lua_hooks.h"
#include "lua_gameevents.h"
#include "../../SDK/CLuaConfigs.h"
#pragma comment(lib, "Libraries/lua-wrapper/include/lua51.lib")

enum menu_item_type
{
	NEXT_LINE,
	CHECK_BOX,
	SLIDER,
	COLOR_PICKER,
	TEXT,
	BUTTON
};

class menu_item
{
public:
	bool check_box_value = false;
	bool button_value = false;

	std::string key = XorStr("");

	std::vector <std::string> combo_box_labels;
	int combo_box_value = 0;

	float slider_float_min = 0.0f;
	float slider_float_max = 0.0f;
	float slider_float_value = 0.0f;

	std::string format = XorStr("%1.f");

	FloatColor color_picker_value = FloatColor(1.0f, 1.0f, 1.0f, 1.0f);
	menu_item_type type = NEXT_LINE;

	menu_item()
	{
		type = NEXT_LINE;
	}

	menu_item(const menu_item& item)
	{
		check_box_value = item.check_box_value;
		button_value = item.button_value;

		combo_box_labels = item.combo_box_labels;
		combo_box_value = item.combo_box_value;

		slider_float_min = item.slider_float_min;
		slider_float_max = item.slider_float_max;
		slider_float_value = item.slider_float_value;

		color_picker_value = item.color_picker_value;
		type = item.type;
		key = item.key;
		format = item.format;
	}

	menu_item& operator=(const menu_item& item)
	{
		check_box_value = item.check_box_value;
		button_value = item.button_value;

		combo_box_labels = item.combo_box_labels;
		combo_box_value = item.combo_box_value;

		slider_float_min = item.slider_float_min;
		slider_float_max = item.slider_float_max;
		slider_float_value = item.slider_float_value;

		color_picker_value = item.color_picker_value;
		type = item.type;
		key = item.key;
		format = item.format;

		return *this;
	}

	menu_item(std::string key2, bool value)
	{
		key = key2;
		check_box_value = value;
		type = CHECK_BOX;
	}

	menu_item(std::string key2, float min, float max, float value, std::string format2)
	{
		key = key2;
		slider_float_min = min;
		slider_float_max = max;
		slider_float_value = value;
		format = format2;

		type = SLIDER;
	}

	menu_item(std::string key2, FloatColor value) //-V818
	{
		key = key2;
		color_picker_value = value;
		type = COLOR_PICKER;
	}

	menu_item(const std::string name)
	{
		type = TEXT;
	}

	menu_item(std::string key2, bool value, bool random)
	{
		key = key2;
		button_value = value;
		type = BUTTON;
	}
};

class c_lua {
public:
	bool initialize();

	void refresh_scripts();

	void load_script(int id);
	void unload_script(int id);

	void reload_all_scripts();
	void unload_all_scripts();

	int get_script_id(std::string name);
	int get_script_id_by_path(std::string path);

	std::vector<bool> loaded;
	std::vector<std::string> scripts;

	std::vector <std::vector <std::pair <std::string, menu_item>>> items;

	sol::state lua;

private:
	std::string get_script_path(std::string name);
	std::string get_script_path(int id);

	std::vector<std::filesystem::path> pathes;
};

extern c_lua g_lua;