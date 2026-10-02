#pragma once
#include "../../SDK/sdk.hpp"
#include <lua-wrapper/include/lua.h>
#include <sol/sol.hpp>
#include <map>

struct c_luahooks {
	int scriptId;
	sol::protected_function func;
};

class c_luahookmanager {
public:
	void register_hook(std::string eventName, int scriptId, sol::protected_function func);
	void unregister_hooks(int scriptId);

	std::vector<c_luahooks> get_hooks(std::string eventName);

private:
	std::map<std::string, std::vector<c_luahooks>> hooks;
};


inline c_luahookmanager g_luahookmanager;
