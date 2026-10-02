#include "netdata.h"

NetData g_netdata{};;

void NetData::store(CUserCmd* cmd) {
	int          tickbase;
	int          slot;
	StoredData_t* data;

	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || !local->IsAlive() || !Interfaces::m_pEngine->IsInGame()) {
		reset();
		return;
	}

	tickbase = local->m_nTickBase();
	slot = cmd->command_number;

	// get current record and store data.
	data = &m_data[slot % MULTIPLAYER_BACKUP];

	data->m_tickbase = tickbase;
	data->m_command = slot;
	data->m_punch = local->m_aimPunchAngle();
	data->m_punch_vel = local->m_aimPunchAngleVel();
	data->m_view_offset = local->m_vecViewOffset();
	data->m_velocity_modifier = local->m_flVelocityModifier();
	data->m_duckAmount = local->m_flDuckAmount();
	data->m_flFallVelocity = local->m_flFallVelocity();
	data->m_is_filled = true;
}

void NetData::apply() {
	int          tickbase;
	StoredData_t* data;
	QAngle        punch_delta, punch_vel_delta;
	Vector       view_delta;
	float        modifier_delta;

	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || !local->IsAlive() || !Interfaces::m_pEngine->IsInGame()) {
		reset();
		return;
	}

	tickbase = local->m_nTickBase();

	// get current record and validate.
	data = &m_data[tickbase % MULTIPLAYER_BACKUP];

	if (!data->m_is_filled)
		return;

	if (local->m_nTickBase() != data->m_tickbase)
		return;

	// get deltas.
	// note - dex;  before, when you stop shooting, punch values would sit around 0.03125 and then goto 0 next update.
	//              with this fix applied, values slowly decay under 0.03125.
	punch_delta = local->m_aimPunchAngle() - data->m_punch;
	punch_vel_delta = local->m_aimPunchAngleVel() - data->m_punch_vel;
	view_delta = local->m_vecViewOffset() - data->m_view_offset;
	modifier_delta = local->m_flVelocityModifier() - data->m_velocity_modifier;
	auto duck_amount = local->m_flDuckAmount() - data->m_duckAmount;
	auto fall_velocity_delta = local->m_flFallVelocity() - data->m_flFallVelocity;

	// get deltas.
	 // note - dex;  before, when you stop shooting, punch values would sit around 0.03125 and then goto 0 next update.
	 //              with this fix applied, values slowly decay under 0.03125.
	auto delta = [](const QAngle& a, const QAngle& b) {
		auto delta = a - b;
		return std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
	};

	// set data.
	if (delta(local->m_aimPunchAngle(), data->m_punch) <= 0.03125f) {
		local->m_aimPunchAngle() = data->m_punch;
	}

	if (delta(local->m_aimPunchAngleVel(), data->m_punch_vel) <= 0.03125f) {
		local->m_aimPunchAngleVel() = data->m_punch_vel;
	}

	if (local->m_vecViewOffset().Distance(data->m_view_offset) <= 0.03125f) {
		local->m_vecViewOffset() = data->m_view_offset;
	}

	//modify deltas restoration
	if (std::abs(modifier_delta) <= 0.00625f)
		local->m_flVelocityModifier() = data->m_velocity_modifier;

	if (std::abs(duck_amount) <= 0.03125f)
		local->m_flDuckAmount() = data->m_duckAmount;

	if (std::abs(fall_velocity_delta) <= 0.03125f)
		local->m_flFallVelocity() = data->m_flFallVelocity;
}

void NetData::reset() {
	m_data.fill(StoredData_t());
}

void NetData::reduction(CUserCmd* cmd) {
	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || !m_data.empty())
		return;

	// we predicted him, but our tickbase breaks
	// so lets fix it.
	if (cmd->hasbeenpredicted) {
		for (auto& data : m_data) {
			data.m_tickbase--;
		}

		// exit out now.
		return;
	}
}