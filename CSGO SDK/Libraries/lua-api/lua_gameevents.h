#pragma once
#include "../../SDK/sdk.hpp"
#include <lua-wrapper/include/lua.h>
#include <sol/sol.hpp>
#include <map>

struct c_luagameevents {
	int scriptId;
	sol::protected_function func;
};

class c_luagameeventmanager {
public:
	void register_gameevent(std::string eventName, int scriptId, sol::protected_function func);
	void unregister_gameevents(int scriptId);

	std::vector<c_luagameevents> get_gameevents(std::string eventName);

private:
	std::map<std::string, std::vector<c_luagameevents>> gameevents;
};


inline c_luagameeventmanager g_luagameeventmanager;
