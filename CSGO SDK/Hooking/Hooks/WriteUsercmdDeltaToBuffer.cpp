#include "../Hooked.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../Features/Rage/TickbaseShift.hpp"
#include "../../SDK/Classes/Player.hpp"

bool inSendMove = false, firstSendMovePack = false;
int sinceuse = 0;

void WriteUsercmdD(bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd) {
	__asm
	{
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    Engine::Displacement.Function.m_WriteUsercmd
		add     esp, 4
	}
}

bool __fastcall Hooked::WriteUsercmdDeltaToBuffer(void* ECX, void* EDX, int nSlot, bf_write* buffer, int o_from, int o_to, bool isnewcommand) {
	g_Vars.globals.szLastHookCalled = XorStr("50");
	auto LocalPlayer = C_CSPlayer::GetLocalPlayer();

	if (!LocalPlayer
		|| !Interfaces::m_pEngine->IsConnected()
		|| !LocalPlayer->IsAlive())
		return oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, isnewcommand);

	if (!g_Vars.globals.shift_amount || Interfaces::m_pClientState->m_nChokedCommands() > 3)
		return oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, isnewcommand);

	if (o_from != -1)
		return true;

	// CL_SendMove function
	auto CL_SendMove = []()
	{
		using CL_SendMove_t = void(__fastcall*)(void);
		static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Memory::Scan(XorStr("engine.dll"), XorStr("55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98"));

		CL_SendMoveF();
	};

	auto p_new_commands = (int*)((DWORD)buffer - 0x2C);
	auto p_backup_commands = (int*)((DWORD)buffer - 0x30);
	auto new_commands = *p_new_commands;

	if (!inSendMove)
	{
		if (new_commands <= 0)
			return false;

		inSendMove = true;
		firstSendMovePack = true;
		g_Vars.globals.shift_amount += new_commands;

		while (g_Vars.globals.shift_amount > 0)
		{
			CL_SendMove();
			firstSendMovePack = false;
		}

		inSendMove = false;
		return false;
	}

	if (!firstSendMovePack)
	{
		int32_t loss = std::min(g_Vars.globals.shift_amount, 16);

		g_Vars.globals.shift_amount -= loss;
		Interfaces::m_pEngine->GetNetChannelInfo()->m_nOutSequenceNr += loss;
	}

	auto next_cmd_nr = Interfaces::m_pClientState->m_nLastOutgoingCommand() + Interfaces::m_pClientState->m_nChokedCommands() + 1;

	auto total_new_commands = std::clamp(g_Vars.globals.shift_amount, 0, 62);
	g_Vars.globals.shift_amount -= total_new_commands;

	o_from = -1;

	*p_new_commands = total_new_commands;
	*p_backup_commands = 0;

	for (o_to = next_cmd_nr - new_commands + 1; o_to <= next_cmd_nr; o_to++)
	{
		if (!oWriteUsercmdDeltaToBuffer(ECX, nSlot, buffer, o_from, o_to, true))
			return false;

		o_from = o_to;
	}

	CUserCmd* last_real_cmd = Interfaces::m_pInput->GetUserCmd(nSlot, o_from);
	CUserCmd from_cmd;

	if (last_real_cmd)
		memcpy(&from_cmd, last_real_cmd, sizeof(CUserCmd));

	CUserCmd to_cmd;
	memcpy(&to_cmd, &from_cmd, sizeof(CUserCmd));

	to_cmd.command_number++;
	to_cmd.tick_count += 200;

	for (int i = new_commands; i <= total_new_commands; i++)
	{
		WriteUsercmdD(buffer, &to_cmd, &from_cmd);
		memcpy(&from_cmd, &to_cmd, sizeof(CUserCmd));
		from_cmd.command_number++;
		from_cmd.tick_count++;
	}

	return true;
}
