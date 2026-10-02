#pragma once

#define STEAM_KEYSIZE 2048 // max size needed to contain a steam authentication key (both server and client)

// How many bits to use to encode an edict.
#define    MAX_EDICT_BITS                11            // # of bits needed to represent max edicts
// Max # of edicts in a level
#define    MAX_EDICTS                    (1<<MAX_EDICT_BITS)

// How many bits to use to encode an server class index
#define MAX_SERVER_CLASS_BITS        9
// Max # of networkable server classes
#define MAX_SERVER_CLASSES            (1<<MAX_SERVER_CLASS_BITS)

#define SIGNED_GUID_LEN 32 // Hashed CD Key (32 hex alphabetic chars + 0 terminator )

// Used for networking ehandles.
#define NUM_ENT_ENTRY_BITS        (MAX_EDICT_BITS + 5) //Swarm/old sdk is + 2, csgo is 5
#define NUM_ENT_ENTRIES            (1 << NUM_ENT_ENTRY_BITS)
#define NUM_SERIAL_NUM_BITS        (32 - NUM_ENT_ENTRY_BITS)
#define NUM_SERIAL_NUM_SHIFT_BITS (32 - NUM_SERIAL_NUM_BITS)
#define ENT_ENTRY_MASK            (( 1 << NUM_SERIAL_NUM_BITS) - 1)
#define INVALID_EHANDLE_INDEX    0xFFFFFFFF

class ClientClass;
struct CEventInfo {
public:
	enum {
		EVENT_INDEX_BITS = 8,
		EVENT_DATA_LEN_BITS = 11,
		MAX_EVENT_DATA = 192,  // ( 1<<8 bits == 256, but only using 192 below )
	};

	// 0 implies not in use
	short					m_class_id;
	float					m_fire_delay;
	const void* m_send_table;
	const ClientClass* m_client_class;
	int						m_bits;
	unsigned char* m_data;
	int						m_flags;
	char pad_18[ 0x18 ];
	CEventInfo* m_next;
};

class INetChannel;
class IClientNetworkable;

class CClockDriftMgr
{
public:
    enum
    {
        // This controls how much it smoothes out the samples from the server.
        NUM_CLOCKDRIFT_SAMPLES = 16
    };

    // This holds how many ticks the client is ahead each time we get a server tick.
    // We average these together to get our estimate of how far ahead we are.
    float m_ClockOffsets[NUM_CLOCKDRIFT_SAMPLES]; //0x0128
    int m_iCurClockOffset; // 0x0168

    int m_nServerTick; // 0x016C        // Last-received tick from the server.
    int m_nClientTick; // 0x0170        // The client's own tick counter (specifically, for interpolation during rendering).
        // The server may be on a slightly different tick and the client will drift towards it.
}; //Size: 76

class CBaseClientState
{
public:
    char pad_000C[140]; //0x0000
    __int32 m_Socket; //0x0098
    INetChannel* m_pNetChannel; //0x009C
    unsigned __int32 m_nChallengeNr; //0x00A0
    char pad_00A0[4]; //0x00A4
    double m_flConnectTime; //0x00A8
    unsigned __int32 m_nRetryNumber; //0x00B0
    char pad_00B4[84]; //0x00B4
    __int32 m_nSignonState; //0x0108
    char pad_010C[4]; //0x010C
    double m_flNextCmdTime; //0x0114
    __int32 m_nServerCount; //0x0118
    __int32 m_nCurrentSequence; //0x011C
    char pad_0120[8]; //0x0120
    CClockDriftMgr m_ClockDriftMgr; //0x0128
    __int32 m_nDeltaTick; //0x0174
    char pad_0178[4]; //0x0178
    __int32 m_nViewEntity; //0x017C
    __int32 m_nPlayerSlot; //0x0180
    bool m_bPaused; //0x0184
    char pad_0185[3]; //0x0185
    char m_szLevelName[260]; //0x0188
    char m_szLevelNameShort[40]; //0x028C
    char pad_02B4[212]; //0x02B4
    unsigned __int32 m_nMaxClients; //0x0310
    char pad_0314[232]; //0x0314
    class PackedEntity* m_pEntityBaselines[2][MAX_EDICTS]; //0x03FC
    class C_ServerClassInfo* m_pServerClasses; //0x43FC
    __int32 m_nServerClasses; //0x4400
    __int32 m_nServerClassBits; //0x4404
    char m_szEncrytionKey[STEAM_KEYSIZE]; //0x4408
    unsigned __int32 m_iEncryptionKeySize; //0x4C08
    char pad_4C0C[148]; //0x4C0C

    void ForceFullUpdate() { m_nDeltaTick = -1; }

    IClientNetworkable* GetNetworkable()
    {
        return (IClientNetworkable*)((unsigned __int32)this + 8);
    }
};

class CClientState : public CBaseClientState
{
public:
    int& m_nDeltaTick();
    int& m_nLastOutgoingCommand();
    int& m_nChokedCommands();
    int& m_nLastCommandAck();
    int& m_nMaxClients();
    bool& m_bIsHLTV();
    CEventInfo* m_pEvents();
};