#include "TickbaseShift.hpp"
#include "../../source.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../SDK/Classes/weapon.hpp"
#include "../Miscellaneous/Movement.hpp"
#include "../Game/Prediction.hpp"
#include "../../Libraries/minhook-master/include/MinHook.h"
#include "../../Hooking/Hooked.hpp"
#include "../../SDK/Displacement.hpp"

void copy_command(CUserCmd* cmd, int tickbase_shift)
{
	static auto cl_forwardspeed = Interfaces::m_pCvar->FindVar(XorStr("cl_forwardspeed"));
	static auto cl_sidespeed = Interfaces::m_pCvar->FindVar(XorStr("cl_sidespeed"));

	if (g_Vars.misc.autopeek_bind.enabled)
		cmd->sidemove = abs(cmd->sidemove) > 10.f ? copysignf(450.f, cmd->sidemove) : 0.f;
	else
	{
		if (g_Vars.rage.dt_defensive_teleport && g_Vars.misc.autopeek_bind.enabled)
		{
			cmd->sidemove = abs(cmd->sidemove) > 15.f ? copysignf(450.f, cmd->sidemove) : 0.f;
		}
		else if (g_Vars.rage.dt_defensive_teleport)
		{
			if ((cmd->forwardmove) > 10.f)
				cmd->forwardmove = cmd->forwardmove;
			else if ((cmd->forwardmove) < -10.f)
				cmd->forwardmove = -cmd->forwardmove;

			if ((cmd->sidemove) > 10.f)
				cmd->sidemove = cmd->sidemove;
			else if ((cmd->sidemove) < -10.f)
				cmd->sidemove = -cmd->sidemove;
		}
		else {
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;
		}
	}
}

void* g_pLocal = nullptr;
TickbaseSystem g_TickbaseController;

TickbaseShift_t::TickbaseShift_t(int _cmdnum, int _tickbase) :
	cmdnum(_cmdnum), tickbase(_tickbase)
{
	;
}

#define OFFSET_LASTOUTGOING 0x4CAC
#define OFFSET_CHOKED 0x4CB0
#define OFFSET_TICKBASE 0x3404

bool TickbaseSystem::IsTickcountValid(int nTick) {
	return nTick >= (Interfaces::m_pGlobalVars->tickcount + int(1 / Interfaces::m_pGlobalVars->interval_per_tick) + g_Vars.sv_max_usercmd_future_ticks->GetInt());
}

void TickbaseSystem::OnCLMove(bool bFinalTick, float accumulated_extra_samples) {
#ifndef STANDALONE_CSGO
	s_nTicksSinceUse++;
#endif

	//if we have low fps, we need to send our current batch 
	//before we can start building or we'll get a prediction error
	if (!bFinalTick)
	{
		s_bFreshFrame = false;
	}

	//can only start building on the final tick of this frame
	if ((!bFinalTick && !s_bBuilding) || !g_pLocal)
	{
		//level change; reset our shifts
		if (!g_pLocal)
		{
			g_iTickbaseShifts.clear();
		}

		Hooked::oCL_Move(bFinalTick, accumulated_extra_samples);
		return;
	}


	const bool bStart = s_bBuilding;

	if (g_Vars.rage.double_tap_type == 0) {
		s_bBuilding = /*m_didFakeFlick || */((g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit)
			&& s_nTicksSinceUse >= s_nTicksRequired
			&& !m_bSupressRecharge || (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick_mode == 1));
	}
	else {
		s_bBuilding = /*m_didFakeFlick || */((g_Vars.rage.key_dt.enabled && g_Vars.rage.exploit)
			&& s_nTicksSinceUse >= s_nTicksRequired || (g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick_mode == 1));
	}

	if (bStart && !s_bBuilding && ((s_nExtraProcessingTicks > 0 && !s_bAckedBuild) || (int)s_nExtraProcessingTicks < s_iClockCorrectionTicks))
	{
		s_bBuilding = true;

		//wait for a fresh frame on low fps to start charging 
	}
	else if (!bStart && s_bBuilding && !s_bFreshFrame)
	{
		if (bFinalTick)
		{
			s_bFreshFrame = true;
		}

		s_iServerIdealTick++;
		Hooked::oCL_Move(bFinalTick, accumulated_extra_samples);
		return;
	}

	//do this afterwards so we catch when we're on the final tick
	//if we had multiple ticks this frame
	if (bFinalTick)
	{
		s_bFreshFrame = true;
	}

#ifndef STANDALONE_CSGO
	if (!bStart && s_bBuilding)
	{
		s_nTicksSinceStarted = 0;
	}

	if (s_bBuilding)
	{
		s_nTicksSinceStarted++;
		if (s_nTicksSinceStarted <= s_nTicksDelay)
		{
			//g_Vars.globals.m_pCmd->tick_count = INT_MAX;
			s_iServerIdealTick++;
			Hooked::oCL_Move(bFinalTick, accumulated_extra_samples);
			return;
		}
	}
#endif

	if (s_bBuilding && s_nExtraProcessingTicks < s_nSpeed)
	{
		s_bAckedBuild = false;

		//keep track of the server's ideal tickbase
		//note: should really do this once when host_currentframetick is 0 
		//but it doesn't matter since we don't do anything here anyway
		s_iServerIdealTick++;

		//allow time for this cmd to process; increase m_iTicksAllowedForProcessing
		s_nExtraProcessingTicks++;
		return;
	}

	int cmdnumber = 1;
	int choke = *(int*)((size_t)Interfaces::m_pClientState.Xor() + OFFSET_CHOKED);
	cmdnumber += *(int*)((size_t)Interfaces::m_pClientState.Xor() + OFFSET_LASTOUTGOING);
	cmdnumber += choke;

	//where the server wants us to be
	int arrive = s_iServerIdealTick;

	bool inya = false;

	//if we charged eg 15 ticks, our client's tickbase will be 15 ticks behind
	//and the next tick we send will cause our tickbase to be adjusted
	//so account for that
	if (!s_bAckedBuild)
	{
		s_bAckedBuild = true;

		//note that we should really be adding host_frameticks - 1 - host_currentframetick to estimated 
		//but that will only matter on low fps

		int estimated = *(int*)((size_t)g_pLocal + OFFSET_TICKBASE) + **(int**)Engine::Displacement.Data.m_uHostFrameTicks + choke;
		if (estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks)
		{
			estimated = arrive - **(int**)Engine::Displacement.Data.m_uHostFrameTicks - choke + 1;
			g_iTickbaseShifts.emplace_back(cmdnumber, estimated);
		}

		Hooked::oCL_Move(bFinalTick, accumulated_extra_samples);

		//keep track of time
		s_iServerIdealTick++;
	}
	//if you want to have absolutely perfect prediction on low fps with doubletap, 
	//only double tap on the first tick of a frame with multiple ticks
	//this is because older ticks in the same frame will now be being run 
	//with a different tickbase if you shift later on
	//this does not really matter^^ which is why on shot anti aim with fakelag is a thing
	else if (!s_bBuilding && s_nExtraProcessingTicks > 0)
	{
#ifndef STANDALONE_CSGO
		jmpRunExtraCommands :
#endif

		//the + 1 is because of the real command we are due
		int estimated = *(int*)((size_t)g_pLocal + OFFSET_TICKBASE) + **(int**)Engine::Displacement.Data.m_uHostFrameTicks + s_nExtraProcessingTicks + choke;
		if (estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks)
		{
			estimated = arrive - s_nExtraProcessingTicks - choke - **(int**)Engine::Displacement.Data.m_uHostFrameTicks + 1;

			const size_t realcmd = **(int**)Engine::Displacement.Data.m_uHostFrameTicks + s_nExtraProcessingTicks + choke;
			for (size_t i = 0; i < realcmd; i++)
			{
				//now account for the shift on all our new commands
				g_iTickbaseShifts.emplace_back(cmdnumber, estimated);

				cmdnumber++;
				estimated++;
			}

			//and then make sure the command after all of the above 
			//starts in the right spot
			//estimated = arrive + 1; // + 1 here??
			//g_iTickbaseShifts.emplace_back(cmdnumber, estimated);
		}

		if (!inya)
		{
			Hooked::oCL_Move(false, accumulated_extra_samples);

			__asm
			{
				xorps xmm0, xmm0
			}
		}

		while (s_nExtraProcessingTicks > 0)
		{
			bShifting = true;
			bFinalTick = s_nExtraProcessingTicks == 1;
			//g_Vars.globals.m_pCmd->tick_count = INT_MAX;
			Hooked::oCL_Move(bFinalTick, 0.f);

			if (inya)
			{
				inya = false;
				__asm
				{
					xorps xmm0, xmm0
				}
			}

			s_nExtraProcessingTicks--;

			//if (s_nExtraProcessingTicks <= 1) {
			//	lastShiftedCmdNr = Interfaces::m_pClientState->m_nLastOutgoingCommand();
			//}

		}
		bShifting = false;

		//keep track of time
		s_iServerIdealTick++;

#ifndef STANDALONE_CSGO
		s_nTicksSinceUse = 0;
#endif
	}
	else
	{
		//otherwise copy the 'prestine' server ideal tick (m_nTickBase)
		//note that this will actually break on really low doubletap speeds 
		//(ie <= sv_clockcorrection_msecs because the server won't adjust your tickbase) 
		if (g_iTickbaseShifts.size())
		{
			s_iServerIdealTick++;
		}
		else
		{
			s_iServerIdealTick = *(int*)((size_t)g_pLocal + OFFSET_TICKBASE) + 1;
		}

#ifndef STANDALONE_CSGO
		int start = *(int*)((size_t)g_pLocal + OFFSET_TICKBASE);
		bool bPred = s_bBuilding && s_nExtraProcessingTicks > 0;

		if (bPred)
		{
			s_bInMove = true;
			s_iMoveTickBase = start;

			int estimated = start + s_nExtraProcessingTicks + **(int**)Engine::Displacement.Data.m_uHostFrameTicks + choke;
			if (estimated > arrive + s_iClockCorrectionTicks || estimated < arrive - s_iClockCorrectionTicks)
			{
				estimated = arrive - s_nExtraProcessingTicks - choke - **(int**)Engine::Displacement.Data.m_uHostFrameTicks + 1;

				s_iMoveTickBase = estimated;
				*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = estimated;
			}
		}
		else
		{
			s_bInMove = false;
		}
#endif

		// not building or it's not time to send
		Hooked::oCL_Move(bFinalTick, accumulated_extra_samples);

#ifndef STANDALONE_CSGO
		if (bPred && !(g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled && g_Vars.misc.mind_trick_mode == 1))
		{
			s_bInMove = false;
			*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = start;

			if (Interfaces::m_pInput.Xor())
			{
				typedef CUserCmd* (__thiscall* GetUserCmdFn_t)(void*, int, int);
				GetUserCmdFn_t fn = (GetUserCmdFn_t)((*(void***)Interfaces::m_pInput.Xor())[8]);
				if (fn)
				{
					CUserCmd* cmd = fn(Interfaces::m_pInput.Xor(), -1, cmdnumber);
					if (cmd)
					{
						auto local = C_CSPlayer::GetLocalPlayer();

						C_WeaponCSBaseGun* Weapon = (C_WeaponCSBaseGun*)local->m_hActiveWeapon().Get();

						if (!Weapon)
							return;

						auto weaponInfo = Weapon->GetCSWeaponData();
						if (!weaponInfo.IsValid())
							return;

						if (cmd->buttons & (1 << 0) && weaponInfo->m_iWeaponType != WEAPONTYPE_GRENADE && Weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && Weapon->m_iItemDefinitionIndex() != WEAPON_C4 && m_bForceUnChargeState)
						{
							if (g_Vars.rage.double_tap_type == 0) {
								copy_command(cmd, s_nSpeed);
								s_bBuilding = false;
								inya = true;
								goto jmpRunExtraCommands;
							}
							else {
								copy_command(cmd, s_nSpeed);
								m_bSupressRecharge = true;
								//inya = true; // man what the fuck does this do?
								goto jmpRunExtraCommands;
							}
						}
						//else if (g_Vars.globals.m_bShotReady && Weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) {
						//	s_bBuilding = false;
						//	inya = true;
						//	goto jmpRunExtraCommands;
						//}
					}
				}
			}
		}
#endif
	}
}

// TODO: Move me
void InvokeRunSimulation(void* this_, float curtime, int cmdnum, CUserCmd* cmd, size_t local) {
	__asm {
		push local
		push cmd
		push cmdnum

		movss xmm2, curtime
		mov ecx, this_

		call Hooked::RunSimulationDetor.m_pOldFunction
	}
}

void TickbaseSystem::OnRunSimulation(void* this_, int iCommandNumber, CUserCmd* pCmd, size_t local) {
	g_pLocal = (void*)local;

	float curtime;
	__asm
	{
		movss curtime, xmm2
	}

	for (int i = 0; i < (int)g_iTickbaseShifts.size(); i++)
	{
		//ideally you compare the sequence we set this tickbase shift to
		//with the last acknowledged sequence
		if ((g_iTickbaseShifts[i].cmdnum < iCommandNumber - s_iNetBackup) ||
			(g_iTickbaseShifts[i].cmdnum > iCommandNumber + s_iNetBackup))
		{
			g_iTickbaseShifts.erase(g_iTickbaseShifts.begin() + i);
			i--;
		}
	}

	int tickbase = -1;
	for (size_t i = 0; i < g_iTickbaseShifts.size(); i++)
	{
		const auto& elem = g_iTickbaseShifts[i];

		if (elem.cmdnum == iCommandNumber)
		{
			tickbase = elem.tickbase;
			break;
		}
	}

	//apply our new shifted tickbase 
	if (tickbase != -1 && local)
	{
		//*(int*)(local + OFFSET_TICKBASE) = tickbase;
		curtime = tickbase * s_flTickInterval;
	}

	//run simulation is the perfect place to do this because
	//all other predictables (ie your weapon)
	//will be run in the right curtime 
	InvokeRunSimulation(this_, curtime, iCommandNumber, pCmd, local);
}

void TickbaseSystem::OnPredictionUpdate(void* prediction, void*, int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) {
	typedef void(__thiscall* PredictionUpdateFn_t)(void*, int, bool, int, int);
	PredictionUpdateFn_t fn = (PredictionUpdateFn_t)Hooked::PredictionUpdateDetor.m_pOldFunction;
	fn(prediction, startframe, validframe, incoming_acknowledged, outgoing_command);

	if (s_bInMove && g_pLocal) {
		*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = s_iMoveTickBase;
	}

	if (g_pLocal) {
		for (size_t i = 0; i < g_iTickbaseShifts.size(); i++) {
			const auto& elem = g_iTickbaseShifts[i];

			if (elem.cmdnum == (outgoing_command + 1)) {
				//*(int*)((size_t)g_pLocal + OFFSET_TICKBASE) = elem.tickbase;
				break;
			}
		}
	}
}

bool TickbaseSystem::Building() const {
	return s_bBuilding && !s_nExtraProcessingTicks && Interfaces::m_pClientState->m_nChokedCommands() > 0;
}

bool TickbaseSystem::Using() const {
	return !s_bBuilding && s_nExtraProcessingTicks;
}