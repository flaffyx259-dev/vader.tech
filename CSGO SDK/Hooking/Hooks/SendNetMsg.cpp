#include "../hooked.hpp"
#include "../../Utils/InputSys.hpp"
#include "../../SDK/Classes/Exploits.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../Features/Rage/ExtendedBactrack.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../Features/Rage/TickbaseShift.hpp"
#include "../../Features/Rage/LagCompensation.hpp"

int lastsent = 0;
int lastsent_crack = 0;

struct VoiceDataCustom
{
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t sequence_bytes{};
	uint32_t section_number{};
	uint32_t uncompressed_sample_offset{};

	__forceinline uint8_t* get_raw_data()
	{
		return (uint8_t*)this;
	}
};

struct CCLCMsg_VoiceData_Legacy
{
	uint32_t INetMessage_Vtable; //0x0000
	char pad_0004[4]; //0x0004
	uint32_t CCLCMsg_VoiceData_Vtable; //0x0008
	char pad_000C[8]; //0x000C
	void* data; //0x0014
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C
	int32_t cached_size; //0x0030

	uint32_t flags; //0x0034

	uint8_t no_stack_overflow[0xFF];

	__forceinline void set_data(VoiceDataCustom* cdata)
	{
		xuid_low = cdata->xuid_low;
		xuid_high = cdata->xuid_high;
		sequence_bytes = cdata->sequence_bytes;
		section_number = cdata->section_number;
		uncompressed_sample_offset = cdata->uncompressed_sample_offset;
	}
};

struct lame_string_t

{
	char data[16]{};
	uint32_t current_len = 0;
	uint32_t max_len = 15;
};


struct CIncomingSequence {
	int InSequence;
	int ReliableState;
};

std::vector<CIncomingSequence> IncomingSequences;

void WriteUsercmd( bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd ) {
	__asm
	{
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    Engine::Displacement.Function.m_WriteUsercmd
		add     esp, 4
	}
}

void exploit_thing(CCLCMsg_Move_t* CL_Move, INetChannel* pNetChan) {
	if (g_Vars.misc.balls) {
		CL_Move->m_nBackupCommands = 1;
		CL_Move->m_nNewCommands = 0;

		// this isnt necessarily needed but it makes sure that dropped_packets is 0 at all times, classy was having issues with this probably cuz of shit internet
		pNetChan->m_nChokedPackets = 200;

		//C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		//if (LocalPlayer && LocalPlayer->IsAlive()) {
		//	const auto time_shift = TICKS_TO_TIME(Interfaces::m_pEngine->GetServerTick() - LocalPlayer->m_nTickBase());
		//	printf(XorStr("shifted seconds: %f\n"), time_shift);
		//}

		//Interfaces::m_pClientState->m_nChokedCommands() = 200;
	}
}

void BypassChokeLimit( CCLCMsg_Move_t* CL_Move, INetChannel* pNetChan ) {
	// not shifting or dont need do extra fakelag
	if( CL_Move->m_nNewCommands != 15 || Interfaces::m_pClientState->m_nChokedCommands( ) <= 14 )
		return;

	using assign_lol = std::string& ( __thiscall* )( void*, uint8_t*, size_t );
	auto assign_std_autistic_string = ( assign_lol )Engine::Displacement.Function.m_StdStringAssign;

	// rebuild CL_SendMove
	uint8_t data[ 4000 ];
	bf_write buf;
	buf.m_nDataBytes = 4000;
	buf.m_nDataBits = 32000;
	buf.m_pData = data;
	buf.m_iCurBit = false;
	buf.m_bOverflow = false;
	buf.m_bAssertOnOverflow = false;
	buf.m_pDebugName = false;
	int numCmd = Interfaces::m_pClientState->m_nChokedCommands( ) + 1;
	int nextCmdNr = Interfaces::m_pClientState->m_nLastOutgoingCommand( ) + numCmd;
	if( numCmd > 62 )
		numCmd = 62;

	bool bOk = true;

	auto to = nextCmdNr - numCmd + 1;
	auto from = -1;
	if( to <= nextCmdNr ) {
		int newcmdnr = to >= ( nextCmdNr - numCmd + 1 );
		do {
			bOk = bOk && Interfaces::m_pInput->WriteUsercmdDeltaToBuffer( 0, &buf, from, to, to >= newcmdnr );
			from = to++;
		} while( to <= nextCmdNr );
	}

	if( bOk ) {
		/*if( g_TickbaseController.iCommandsToShift > 0 ) {
			CUserCmd from_cmd, to_cmd;
			from_cmd = Interfaces::m_pInput->m_pCommands[ nextCmdNr % MULTIPLAYER_BACKUP ];
			to_cmd = from_cmd;
			to_cmd.tick_count = INT_MAX;

			do {
				if( numCmd >= 62 ) {
					g_TickbaseController.iCommandsToShift = 0;
					break;
				}

				to_cmd.command_number++;
				WriteUsercmd( &buf, &to_cmd, &from_cmd );

				g_TickbaseController.iCommandsToShift--;
				numCmd++;
			} while( g_TickbaseController.iCommandsToShift > 0 );
		}
		else {
			g_TickbaseController.iCommandsToShift = 0;
		}*/

		// bypass choke limit
		CL_Move->m_nNewCommands = numCmd;
		CL_Move->m_nBackupCommands = 0;

		int curbit = ( buf.m_iCurBit + 7 ) >> 3;
		assign_std_autistic_string( CL_Move->m_data, buf.m_pData, curbit );
	}
}

bool __fastcall Hooked::SendNetMsg( INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice ) {
	g_Vars.globals.szLastHookCalled = XorStr( "33" );
	if( pNetChan != Interfaces::m_pEngine->GetNetChannelInfo( ) || !g_Vars.globals.HackIsReady )
		return oSendNetMsg( pNetChan, msg, bForceReliable, bVoice );

	if( msg.GetType( ) == 14 ) // Return and don't send messsage if its FileCRCCheck
		return false;

	if( msg.GetGroup( ) == 11 ) {
		BypassChokeLimit( ( CCLCMsg_Move_t* )&msg, pNetChan );
		exploit_thing((CCLCMsg_Move_t*)&msg, pNetChan);

		// VADER DEV
#if defined(DEV)
		if (!g_Vars.misc.anonymous_mode_cope) {
			constexpr int EXPIRE_DURATION = 5000; // miliseconds-ish?
			bool should_send = GetTickCount() - lastsent > EXPIRE_DURATION;
			if (should_send) {
				Voice_Vader packet;
				strcpy(packet.cheat_name, XorStr("vader.techdevballs"));
				packet.make_sure = 1;
				packet.username = g_Vars.globals.c_username.c_str();
				VoiceDataCustom data;
				memcpy(data.get_raw_data(), &packet, sizeof(packet));

				CCLCMsg_VoiceData_Legacy msg;
				memset(&msg, 0, sizeof(msg));

				static DWORD m_construct_voice_message = (DWORD)Memory::Scan(XorStr("engine.dll"), XorStr("56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7"));

				auto func = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
				func((void*)&msg, nullptr);

				// set our data
				msg.set_data(&data);

				// mad!
				lame_string_t lame_string;

				// set rest
				msg.data = &lame_string;
				msg.format = 0; // VoiceFormat_Steam
				msg.flags = 63; // all flags!

				// send it
				oSendNetMsg(pNetChan, (INetMessage&)msg, false, true);

				lastsent = GetTickCount();
			}
		}
#endif

		// VADER BETA
#if defined(BETA_MODE)
		constexpr int EXPIRE_DURATION = 5000; // miliseconds-ish?
		bool should_send = GetTickCount() - lastsent > EXPIRE_DURATION;
		if (should_send) {
			Voice_Vader packet;
			strcpy(packet.cheat_name, XorStr("vader.techbetaballs"));
			packet.make_sure = 1;
			packet.username = g_Vars.globals.c_username.c_str();
			VoiceDataCustom data;
			memcpy(data.get_raw_data(), &packet, sizeof(packet));

			CCLCMsg_VoiceData_Legacy msg;
			memset(&msg, 0, sizeof(msg));

			static DWORD m_construct_voice_message = (DWORD)Memory::Scan(XorStr("engine.dll"), XorStr("56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7"));

			auto func = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
			func((void*)&msg, nullptr);

			// set our data
			msg.set_data(&data);

			// mad!
			lame_string_t lame_string;

			// set rest
			msg.data = &lame_string;
			msg.format = 0; // VoiceFormat_Steam
			msg.flags = 63; // all flags!

			// send it
			oSendNetMsg(pNetChan, (INetMessage&)msg, false, true);

			lastsent = GetTickCount();
		}
#endif

		// VADER LIVE
#if defined(LIVE_MODE)
		constexpr int EXPIRE_DURATION = 5000; // miliseconds-ish?
		bool should_send = GetTickCount() - lastsent > EXPIRE_DURATION;
		if (should_send) {
			Voice_Vader packet;
			strcpy(packet.cheat_name, XorStr("vader.techballs"));
			packet.make_sure = 1;
			packet.username = g_Vars.globals.c_username.c_str();
			VoiceDataCustom data;
			memcpy(data.get_raw_data(), &packet, sizeof(packet));

			CCLCMsg_VoiceData_Legacy msg;
			memset(&msg, 0, sizeof(msg));

			static DWORD m_construct_voice_message = (DWORD)Memory::Scan(XorStr("engine.dll"), XorStr("56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7"));

			auto func = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
			func((void*)&msg, nullptr);

			// set our data
			msg.set_data(&data);

			// mad!
			lame_string_t lame_string;

			// set rest
			msg.data = &lame_string;
			msg.format = 0; // VoiceFormat_Steam
			msg.flags = 63; // all flags!

			// send it
			oSendNetMsg(pNetChan, (INetMessage&)msg, false, true);

			lastsent = GetTickCount();
		}
#endif


		// CRACK USERS LOL
		constexpr int EXPIRE_DURATION_CRACK = 10000; // miliseconds-ish?
		bool should_send_crack = GetTickCount() - lastsent_crack > EXPIRE_DURATION_CRACK;
		if (should_send_crack) {
			Voice_Crack packet;
			strcpy(packet.cheat_name, XorStr("usucknigga"));
			packet.xuid_high = 1;
			packet.sec_bytes = -858993664;
			packet.sec_number = 1204316679; // this sets your yaw, i got no idea how the math works so i just put a rand number in you could also generate a random int and put there but whatever. - exon
			packet.sample_offset = -858993664;
			VoiceDataCustom data;
			memcpy(data.get_raw_data(), &packet, sizeof(packet));

			CCLCMsg_VoiceData_Legacy msg;
			memset(&msg, 0, sizeof(msg));

			static DWORD m_construct_voice_message = (DWORD)Memory::Scan(XorStr("engine.dll"), XorStr("56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7"));

			auto func = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
			func((void*)&msg, nullptr);

			// set our data
			msg.set_data(&data);

			// mad!
			lame_string_t lame_string;

			// set rest
			msg.data = &lame_string;
			msg.format = 0; // VoiceFormat_Steam
			msg.flags = 63; // all flags!

			// send it
			oSendNetMsg(pNetChan, (INetMessage&)msg, false, true);

			lastsent_crack = GetTickCount();
		}
		// CRACK USERS LOL
	}
	else if( msg.GetGroup( ) == 9 ) { // group 9 is VoiceData
	// Fixing fakelag with voice
		bVoice = true;
		g_Vars.globals.VoiceEnable = true;
	}
	else
		g_Vars.globals.VoiceEnable = false;

	return oSendNetMsg( pNetChan, msg, bForceReliable, bVoice );
}

#define NET_FRAMES_BACKUP 64 // must be power of 2. 
#define NET_FRAMES_MASK ( NET_FRAMES_BACKUP - 1 )
int __fastcall Hooked::SendDatagram( INetChannel* pNetChan, void* edx, void* buf ) {
	g_Vars.globals.szLastHookCalled = XorStr( "33" );
	if( pNetChan != Interfaces::m_pEngine->GetNetChannelInfo( ) || !g_Vars.globals.HackIsReady || !g_Vars.misc.extended_backtrack || !g_Vars.misc.extended_backtrack_key.enabled)
		return oSendDatagram( pNetChan, buf );

	//int backup2 = pNetChan->m_nInSequenceNr;

	//if (g_Vars.misc.extended_backtrack && g_Vars.misc.extended_backtrack_key.enabled) {
	//	float ping = g_Vars.misc.extended_backtrack_time;

	//	float correct = std::max(0.f, (ping) - pNetChan->GetLatency(FLOW_OUTGOING) - (std::max(g_Vars.cl_interp->GetFloat(), g_Vars.cl_interp_ratio->GetFloat() / g_Vars.cl_updaterate->GetFloat())));
	//	
	//	pNetChan->m_nInSequenceNr += 2 * NET_FRAMES_MASK - static_cast<uint32_t>(NET_FRAMES_MASK * correct);
	//}
	//
	//int ret = oSendDatagram(pNetChan, buf);

	//pNetChan->m_nInSequenceNr = backup2;

	//return ret;

	auto v10 = pNetChan->m_nInSequenceNr;
	auto v16 = pNetChan->m_nInReliableState;
	auto v17 = pNetChan->GetLatency( FLOW_OUTGOING );
	if( v17 < g_Vars.misc.extended_backtrack_time ) {
		auto v13 = pNetChan->m_nInSequenceNr - TIME_TO_TICKS( g_Vars.misc.extended_backtrack_time - v17 );
		pNetChan->m_nInSequenceNr = v13;
		for( auto& seq : IncomingSequences ) {
			if( seq.InSequence != v13 )
				continue;

			pNetChan->m_nInReliableState = seq.ReliableState;
		}
	}

	auto result = oSendDatagram( pNetChan, buf );
	pNetChan->m_nInSequenceNr = v10;
	pNetChan->m_nInReliableState = v16;
	return result;
}

void __fastcall Hooked::ProcessPacket(INetChannel* pNetChan, void* edx, void* packet, bool header) {
	g_Vars.globals.szLastHookCalled = XorStr("34");
	oProcessPacket(pNetChan, packet, header);

	IncomingSequences.push_back(CIncomingSequence{ pNetChan->m_nInSequenceNr, pNetChan->m_nInReliableState });
	for (auto it = IncomingSequences.begin(); it != IncomingSequences.end(); ++it) {
		auto delta = abs(pNetChan->m_nInSequenceNr - it->InSequence);
		if (delta > 128) {
			it = IncomingSequences.erase(it);
		}
	}

	// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
	for( CEventInfo* it{ Interfaces::m_pClientState->m_pEvents( ) }; it != nullptr; it = it->m_next ) {
		if( !it->m_class_id )
			continue;

		// set all delays to instant.
		it->m_fire_delay = 0.f;
	}

	// game events are actually fired in OnRenderStart which is WAY later after they are received
	// effective delay by lerp time, now we call them right after theyre received (all receive proxies are invoked without delay).
	Interfaces::m_pEngine->FireEvents( );
}

void __fastcall Hooked::Shutdown( INetChannel* pNetChan, void* EDX, const char* reason ) {
	g_Vars.globals.szLastHookCalled = XorStr( "35" );
	return oShutdown( pNetChan, reason );
}

bool __fastcall Hooked::LooseFileAllowed( void* ecx, void* edx ) {
	return true;
}

void __fastcall Hooked::CheckFileCRCsWithServer( void* ecx, void* edx ) {
	return;
}
