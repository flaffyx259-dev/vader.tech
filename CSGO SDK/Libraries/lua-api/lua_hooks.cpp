#include "lua_hooks.h" 

void c_luahookmanager::register_hook(std::string eventName, int scriptId, sol::protected_function func) {
	c_luahooks hk = { scriptId, func };

	this->hooks[eventName].push_back(hk);
}

void c_luahookmanager::unregister_hooks(int scriptId) {
	for (auto& ev : this->hooks) {
		int pos = 0;

		for (auto& hk : ev.second) {
			if (hk.scriptId == scriptId)
				ev.second.erase(ev.second.begin() + pos);

			pos++;
		}
	}
}

std::vector<c_luahooks> c_luahookmanager::get_hooks(std::string eventName) {
	return this->hooks[eventName];
}
