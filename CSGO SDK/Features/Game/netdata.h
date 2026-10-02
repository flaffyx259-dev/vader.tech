#pragma once

#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../../source.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../SDK/displacement.hpp"
#include "../Rage/TickbaseShift.hpp"
#include "../../Hooking/Hooked.hpp"
#include <deque>

class NetData {
private:
	class StoredData_t {
	public:
		int    m_tickbase, m_command;
		QAngle  m_punch;
		QAngle  m_punch_vel;
		Vector m_view_offset;
		float  m_velocity_modifier;
		float m_duckAmount;
		float m_flFallVelocity;
		bool m_is_filled;

	public:
		__forceinline StoredData_t() : m_tickbase{ }, m_command{ }, m_punch{ }, m_punch_vel{ }, m_view_offset{ }, m_velocity_modifier { }, m_duckAmount{ }, m_flFallVelocity { }, m_is_filled { } {};
	};

	std::array< StoredData_t, MULTIPLAYER_BACKUP > m_data;

public:
	void store(CUserCmd* cmd);
	void apply();
	void reset();
	void reduction(CUserCmd* cmd);
};

extern NetData g_netdata;