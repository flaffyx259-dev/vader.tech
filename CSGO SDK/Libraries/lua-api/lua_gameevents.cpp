#include "lua_gameevents.h" 

void c_luagameeventmanager::register_gameevent(std::string eventName, int scriptId, sol::protected_function func) {
	c_luagameevents hk = { scriptId, func };

	this->gameevents[eventName].push_back(hk);
}

void c_luagameeventmanager::unregister_gameevents(int scriptId) {
	for (auto& ev : this->gameevents) {
		int pos = 0;

		for (auto& hk : ev.second) {
			if (hk.scriptId == scriptId)
				ev.second.erase(ev.second.begin() + pos);

			pos++;
		}
	}
}

std::vector<c_luagameevents> c_luagameeventmanager::get_gameevents(std::string eventName) {
	return this->gameevents[eventName];
}
