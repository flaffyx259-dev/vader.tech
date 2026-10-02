#include "source.hpp"

#include "Hooking/Hooked.hpp"
#include "Utils/InputSys.hpp"

#include "SDK/Classes/PropManager.hpp"
#include "SDK/Displacement.hpp"

#include "SDK/Classes/Player.hpp"

#include "Features/Visuals/Glow.hpp"
#include "Features/Miscellaneous/GameEvent.hpp"

#include "Renderer/Render.hpp"
#include "Features/Miscellaneous/SkinChanger.hpp"
#include "Features/Miscellaneous/KitParser.hpp"

#include "Hooking/hooker.hpp"

#include "Features/Visuals/CChams.hpp"

#include "Features/Game/Prediction.hpp"
#include "Loader/Exports.h"

#include "Utils/fnv.h"
#include "Utils/lazy_importer.hpp"
#include "Features/Miscellaneous/MusicPlayer.hpp"

#include <fstream>

extern ClientClass* CCSPlayerClass;
extern CreateClientClassFn oCreateCCSPlayer;

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define PAD( size ) uint8_t MACRO_CONCAT( _pad, __COUNTER__ )[ size ];

//56 8D 51 3C BE
matrix3x4_t g_HeadBone;

class CParticleCollection;
class C_INIT_RandomColor {
	BYTE pad_0[92];
public:
	Vector	m_flNormColorMin;
	Vector	m_flNormColorMax;
};


using FnProcessInterpolatedList = void( __cdecl* )( );
FnProcessInterpolatedList oProcessInterpolatedList;
void __cdecl hkProcessInterpolatedList( ) {
	g_Vars.globals.szLastHookCalled = XorStr( "38" );
	**( bool** )( Engine::Displacement.Data.s_bAllowExtrapolation ) = false;
	oProcessInterpolatedList( );
}

using FnResetGetWaterContentsForPointCache = void( __thiscall* )( void* );
FnResetGetWaterContentsForPointCache oResetGetWaterContentsForPointCache;
void __fastcall hkResetGetWaterContentsForPointCache( void* ecx, void* edx ) {
	g_Vars.globals.szLastHookCalled = XorStr( "39" );
	if( !Engine::Prediction::Instance( )->InPrediction( ) )
		oResetGetWaterContentsForPointCache( ecx );

	return;
}

matrix3x4_t HeadBone;
unsigned int GetVirtual(void* class_, unsigned int index) { return (unsigned int)(*(int**)class_)[index]; }
#define GetV(x, y) reinterpret_cast<void*>(GetVirtual(x, y))

using FnModifyEyePosition = void( __thiscall* )( C_CSPlayer*, Vector* );
FnModifyEyePosition oModifyEyePoisition;
void __fastcall hkModifyEyePosition( C_CSPlayer* ecx, void* edx, Vector* eye_position ) {
	g_Vars.globals.szLastHookCalled = XorStr( "40" );
	auto local_player = C_CSPlayer::GetLocalPlayer( );
	if( !local_player ) {
		oModifyEyePoisition( ecx, eye_position );
		return;
	}

	if( g_Vars.globals.m_bInCreateMove )
		oModifyEyePoisition( ecx, eye_position );

	return;
};

using FnAddBoxOverlay = void( __thiscall* )( void*, const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration );
FnAddBoxOverlay oAddBoxOverlay;
void __fastcall hkAddBoxOverlay( void* ecx, void* edx, const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration ) {
	g_Vars.globals.szLastHookCalled = XorStr( "41" );

	static uintptr_t fire_bulltes = 0;
	if( !fire_bulltes )
		fire_bulltes = Memory::Scan( XorStr( "client.dll" ), XorStr( "3B 3D ?? ?? ?? ?? 75 4C" ) );

	if( !g_Vars.misc.impacts_spoof || uintptr_t( _ReturnAddress( ) ) != fire_bulltes )
		return oAddBoxOverlay( ecx, origin, mins, max, orientation, r, g, b, a, duration );

	return oAddBoxOverlay( ecx, origin, mins, max, orientation,
		g_Vars.esp.client_impacts.r * 255, g_Vars.esp.client_impacts.g * 255, g_Vars.esp.client_impacts.b * 255, g_Vars.esp.client_impacts.a * 255,
		duration );
}

using FnFireEvents = void( __thiscall* )( void* );
FnFireEvents oFireEvents;
void __fastcall hkFireEvents( void* ecx, void* edx ) {
	g_Vars.globals.szLastHookCalled = XorStr( "42" );
	auto local = C_CSPlayer::GetLocalPlayer( );
	if( local && !local->IsDead( ) ) {
		// get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
		for( CEventInfo* it{ Interfaces::m_pClientState->m_pEvents( ) }; it != nullptr; it = it->m_next ) {
			if( !it->m_class_id )
				continue;

			// set all delays to instant.
			it->m_fire_delay = 0.f;
		}
	}

	oFireEvents( ecx );
}

using ParticleCollection = void(__thiscall*)(C_INIT_RandomColor* thisPtr, CParticleCollection* pParticles, int start_p, int nParticleCount, int nAttributeWriteMask, void* pContext);
ParticleCollection oRandomColor_InitNewParticlesScalar;

void __fastcall Hooked_RandomColor_InitNewParticlesScalar(C_INIT_RandomColor* thisPtr, void* edx, CParticleCollection* pParticles, int start_p, int nParticleCount, int nAttributeWriteMask, void* pContext) { // https://www.unknowncheats.me/forum/3119811-post19.html
	g_Vars.globals.szLastHookCalled = XorStr("48");
	Vector o_min = thisPtr->m_flNormColorMin;
	Vector o_max = thisPtr->m_flNormColorMax;

	if (g_Vars.esp.molotov_color_enable) {

		const char* mat_name = *(char**)(*(uintptr_t*)((uintptr_t)pParticles + 0x48) + 0x40);
		assert(mat_name);

		switch (fnv::hashRuntime(mat_name))
		{
		case fnv::hash("particle\\fire_burning_character\\fire_env_fire.vmt"):
		case fnv::hash("particle\\fire_burning_character\\fire_env_fire_depthblend.vmt"):
		case fnv::hash("particle\\fire_burning_character\\fire_burning_character_depthblend.vmt"):
		case fnv::hash("particle\\fire_burning_character\\fire_burning_character.vmt"):
		case fnv::hash("particle\\fire_burning_character\\fire_burning_character_nodepth.vmt"):
		case fnv::hash("particle\\particle_flares\\particle_flare_001.vmt"):
		case fnv::hash("particle\\particle_flares\\particle_flare_004.vmt"):
		case fnv::hash("particle\\particle_flares\\particle_flare_004b_mod_ob.vmt"):
		case fnv::hash("particle\\particle_flares\\particle_flare_004b_mod_z.vmt"):
		case fnv::hash("particle\\fire_explosion_1\\fire_explosion_1_bright.vmt"):
		case fnv::hash("particle\\fire_explosion_1\\fire_explosion_1b.vmt"):
		case fnv::hash("particle\\fire_particle_4\\fire_particle_4.vmt"):
		case fnv::hash("particle\\fire_explosion_1\\fire_explosion_1_oriented.vmt"):
			thisPtr->m_flNormColorMin = thisPtr->m_flNormColorMax = g_Vars.esp.molotov_color;
			break;
			//default:
				//printf("%s\n", mat_name);
		}
	}

	oRandomColor_InitNewParticlesScalar(thisPtr, pParticles, start_p, nParticleCount, nAttributeWriteMask, pContext);

	thisPtr->m_flNormColorMin = o_min;
	thisPtr->m_flNormColorMax = o_max;
}

void __fastcall Hooked::hkVoiceData(void* ecx, void* edx, void* msg) {
	g_Vars.globals.szLastHookCalled = XorStr("52");

	if (!msg) {
		oVoiceData(ecx, msg);
		return;
	}

	auto local = C_CSPlayer::GetLocalPlayer();

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


	struct CSVCMsg_VoiceData_Legacy
	{
		char pad_0000[8]; //0x0000
		int32_t client; //0x0008
		int32_t audible_mask; //0x000C
		uint32_t xuid_low{};
		uint32_t xuid_high{};
		void* voide_data_; //0x0018
		int32_t proximity; //0x001C
		//int32_t caster; //0x0020
		int32_t format; //0x0020
		int32_t sequence_bytes; //0x0024
		uint32_t section_number; //0x0028
		uint32_t uncompressed_sample_offset; //0x002C

		__forceinline VoiceDataCustom get_data()
		{
			VoiceDataCustom cdata;
			cdata.xuid_low = xuid_low;
			cdata.xuid_high = xuid_high;
			cdata.sequence_bytes = sequence_bytes;
			cdata.section_number = section_number;
			cdata.uncompressed_sample_offset = uncompressed_sample_offset;
			return cdata;
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

	CSVCMsg_VoiceData_Legacy* m = (CSVCMsg_VoiceData_Legacy*)msg;
	int sender_index = m->client + 1;
	VoiceDataCustom data = m->get_data();

	if(!local) {
		oVoiceData(ecx, msg);
		return;
	}

	if(local->EntIndex() == sender_index) {
		oVoiceData(ecx, msg);
		return;
	}

	if (m->format != 0) {
		oVoiceData(ecx, msg);
		return;
	}

	// check if its empty
	if (data.section_number == 0 && data.sequence_bytes == 0 && data.uncompressed_sample_offset == 0) {
		oVoiceData(ecx, msg);
		return;
	}

	//const char* formatter = {
	//		"CSVCMsg_VoiceData_Legacy : \n"
	//		"   client                     : %d\n"
	//		"   audible_mask               : %d\n"
	//		"   xuid_low                   : %d\n"
	//		"   xuid_high                  : %d\n"
	//		"   proximity                  : %d\n"
	//		"   format                     : %d\n"
	//		"   sequence_bytes             : %d\n"
	//		"   section_number             : %d\n"
	//		"   uncompressed_sample_offset : %d\n"
	//};

	//char buffer[4096];

	//snprintf(buffer, 4096 * 2, formatter2, g_csgo.m_globals->m_tick_count, OT_ESP->Counter, OT_ESP->x, Decode(OT_ESP->x), OT_ESP->y, Decode(OT_ESP->y), OT_ESP->z, Decode(OT_ESP->z));

	//snprintf(buffer, 4096, formatter, m->client, m->audible_mask, m->xuid_low, m->xuid_high, m->proximity, m->format, m->sequence_bytes, m->section_number, m->uncompressed_sample_offset);

	//if (buffer) {
	//	printf(buffer);
	//}

	Voice_Vader* packet = (Voice_Vader*)data.get_raw_data();

	if (!strcmp(packet->cheat_name, XorStr("vader.techballs"))) { // vader user
		player_info_t player_info;

		if (Interfaces::m_pEngine->GetPlayerInfo(sender_index, &player_info)) {
			g_Vars.globals.vader_user.push_back(player_info.userId);
		}
	}

	if (!strcmp(packet->cheat_name, XorStr("vader.techbetaballs"))) { // vader beta
		player_info_t player_info;

		if (Interfaces::m_pEngine->GetPlayerInfo(sender_index, &player_info)) {
			g_Vars.globals.vader_beta.push_back(player_info.userId);
		}
	}

	if (!strcmp(packet->cheat_name, XorStr("vader.techdevballs"))) { // vader dev
		player_info_t player_info;

		if (Interfaces::m_pEngine->GetPlayerInfo(sender_index, &player_info)) {
			g_Vars.globals.vader_dev.push_back(player_info.userId);
		}
	}

	if (!strcmp(packet->cheat_name, XorStr("vader.tech")) || !strcmp(packet->cheat_name, XorStr("vader.tech2"))) { // vader crack
		player_info_t player_info;

		if (Interfaces::m_pEngine->GetPlayerInfo(sender_index, &player_info)) {
			g_Vars.globals.vader_crack.push_back(player_info.userId);
		}
	}

	if (!strcmp(packet->cheat_name, XorStr("vader.teach21"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("vader.teacher1"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("amongusimposter1"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("niggers1231"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("faggots1231"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("niggers1231"))) {
		LI_FN(exit)(69);
	}

	if (!strcmp(packet->cheat_name, XorStr("usucknigga1"))) {
		g_Vars.globals.m_rce_forceup = true;
	}

	oVoiceData(ecx, msg);
}


using ShouldDrawViewModel = bool(__thiscall*)(void*);
ShouldDrawViewModel oShouldDrawViewModel;
bool __fastcall hkShouldDrawViewModel(void* ecx, void* edx) { // https://www.unknowncheats.me/forum/counterstrike-global-offensive/455514-viewmodel-drawing-scoped.html
	g_Vars.globals.szLastHookCalled = XorStr("49");
	auto local = C_CSPlayer::GetLocalPlayer();

	if (!local || local->IsDead())
		return oShouldDrawViewModel(ecx);

	if (local->m_bIsScoped() && g_Vars.esp.force_viewmodel_scoped)
		return true;


	return oShouldDrawViewModel(ecx);
}


using net_showfragments_t = bool( __thiscall* )( void* );
net_showfragments_t o_net_show_fragments;
bool __fastcall net_show_fragments( void* cvar, void* edx ) {
	g_Vars.globals.szLastHookCalled = XorStr( "43" );

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );

	if( !pLocal || pLocal->IsDead( ) )
		return o_net_show_fragments( cvar );

	if( !Interfaces::m_pEngine->IsInGame( )  )
		return o_net_show_fragments( cvar );

	auto netchannel = Encrypted_t<INetChannel>( Interfaces::m_pEngine->GetNetChannelInfo( ) );
	if( !netchannel.IsValid( ) )
		return o_net_show_fragments( cvar );

	static auto read_sub_channel_data_ret = reinterpret_cast< uintptr_t* >( Memory::Scan( XorStr("engine.dll"), XorStr("85 C0 74 12 53 FF 75 0C 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C") ) );
	static auto check_receiving_list_ret = reinterpret_cast< uintptr_t* >( Memory::Scan( XorStr("engine.dll"), XorStr("8B 1D ? ? ? ? 85 C0 74 16 FF B6") ) );

	static uint32_t last_fragment = 0;

	if( _ReturnAddress( ) == reinterpret_cast< void* >(check_receiving_list_ret) && last_fragment > 0 ) {
		const auto data = &reinterpret_cast< uint32_t* >( netchannel.Xor( ) )[ 0x54 ];
		const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

		if( bytes_fragments == last_fragment ) {
			auto& buffer = reinterpret_cast< uint32_t* >( data )[ 0x42 ];
			buffer = 0;
		}
	}

	if( _ReturnAddress( ) == read_sub_channel_data_ret) {
		const auto data = &reinterpret_cast< uint32_t* >( netchannel.Xor( ) )[ 0x54 ];
		const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

		last_fragment = bytes_fragments;
	}

	return o_net_show_fragments( cvar );
}

struct MaterialSystemConfig_t {
	int m_Width;
	int m_Height;
	int m_Format;
	int m_RefreshRate;

	float m_fMonitorGamma;
	float m_fGammaTVRangeMin;
	float m_fGammaTVRangeMax;
	float m_fGammaTVExponent;
	bool m_bGammaTVEnabled;
	bool m_bTripleBuffered;
	int m_nAASamples;
	int m_nForceAnisotropicLevel;
	int m_nSkipMipLevels;
	int m_nDxSupportLevel;
	int m_nFlags;
	bool m_bEditMode;
	char m_nProxiesTestMode;
	bool m_bCompressedTextures;
	bool m_bFilterLightmaps;
	bool m_bFilterTextures;
	bool m_bReverseDepth;
	bool m_bBufferPrimitives;
	bool m_bDrawFlat;
	bool m_bMeasureFillRate;
	bool m_bVisualizeFillRate;
	bool m_bNoTransparency;
	bool m_bSoftwareLighting;
	bool m_bAllowCheats;
	char m_nShowMipLevels;
	bool m_bShowLowResImage;
	bool m_bShowNormalMap;
	bool m_bMipMapTextures;
	char m_nFullbright;
	bool m_bFastNoBump;
	bool m_bSuppressRendering;
	bool m_bDrawGray;
	bool m_bShowSpecular;
	bool m_bShowDiffuse;
	int m_nWindowedSizeLimitWidth;
	int m_nWindowedSizeLimitHeight;
	int m_nAAQuality;
	bool m_bShadowDepthTexture;
	bool m_bMotionBlur;
	bool m_bSupportFlashlight;
	bool m_bPaintEnabled;
	char pad[ 0xC ];
};


using FnTraceRay = void( __thiscall* )( void*, const Ray_t&, unsigned int, ITraceFilter*, CGameTrace* );
FnTraceRay oTraceRay;

// lol idk I have seen multiple people doing this xd
void __fastcall TraceRay( void* thisptr, void*, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace )
{
	if( !g_Vars.globals.m_InHBP )
		return oTraceRay( thisptr, ray, fMask, pTraceFilter, pTrace );

	oTraceRay( thisptr, ray, fMask, pTraceFilter, pTrace );

	pTrace->surface.flags |= SURF_SKY;
}


using FnOverrideConfig = bool( __thiscall* )( IMaterialSystem*, MaterialSystemConfig_t&, bool );
FnOverrideConfig oOverrideConfig;
bool __fastcall OverrideConfig( IMaterialSystem* ecx, void* edx, MaterialSystemConfig_t& config, bool bForceUpdate ) {
	g_Vars.globals.szLastHookCalled = XorStr( "44" );
	if( ecx == Interfaces::m_pMatSystem.Xor( ) && g_Vars.esp.fullbright ) {
		config.m_nFullbright = true;
	}

	return oOverrideConfig( ecx, config, bForceUpdate );
}

enum {
	NO_SCOPE_STATIC,
	NO_SCOPE_DYNAMIC
};

int no_scope_mode = NO_SCOPE_STATIC;

using FnDrawSetColor = void( __thiscall* )( void*, int, int, int, int );
FnDrawSetColor oDrawSetColor;
void __fastcall DrawSetColor( ISurface* thisptr, void* edx, int r, int g, int b, int a ) {
	g_Vars.globals.szLastHookCalled = XorStr( "45" );

	if( !g_Vars.esp.remove_scope ) {
		return oDrawSetColor( thisptr, r, g, b, a );
	}

	const auto return_address = uintptr_t( _ReturnAddress( ) );

	static auto return_to_scope_arc = Memory::Scan( XorStr( "client.dll" ), XorStr( "6A 00 FF 50 3C 8B 0D ? ? ? ? FF B7" ) ) + 5;
	static auto return_to_scope_lens = Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 50 3C 8B 4C 24 20" ) ) + 3;

	static auto return_to_scope_lines_clear = Memory::Scan( XorStr( "client.dll" ), XorStr( "0F 82 ? ? ? ? FF 50 3C" ) ) + 0x9;
	static auto return_to_scope_lines_blurry = Memory::Scan( XorStr( "client.dll" ), XorStr( "FF B7 ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B 4C 24 24" ) ) - 0x6;

	if( return_address == return_to_scope_arc
		|| return_address == return_to_scope_lens ) {
		// We don't want this to draw, so we set the alpha to 0
		return oDrawSetColor( thisptr, r, g, b, 0 );
	}


	if( g_Vars.esp.remove_scope_type == NO_SCOPE_DYNAMIC ||
		( return_address != return_to_scope_lines_clear &&
			return_address != return_to_scope_lines_blurry ) )
		return oDrawSetColor( thisptr, r, g, b, a );

	oDrawSetColor( thisptr, r, g, b, g_Vars.esp.remove_scope_blur ? 0 : a );
}

using FnProcessMovement = void( __thiscall* )( void*, int, uint8_t* );
FnProcessMovement oProcessMovement;
void __fastcall hkProcessMovement( void* pThis, void* edx, int player, uint8_t* moveData ) {
	g_Vars.globals.szLastHookCalled = XorStr( "46" );
	/*if ( !pThis || !edx )
	   return;*/

	*moveData &= 253u;
	oProcessMovement( pThis, player, moveData );
}


using FnDoExtraBonesProccesing = void( __thiscall* )( C_CSPlayer*, CStudioHdr*, Vector*, Quaternion*, matrix3x4_t*, void*, void* );
FnDoExtraBonesProccesing oDoExtraBonesProccesing;
void __fastcall DoExtraBonesProccesing( C_CSPlayer* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* rotations, matrix3x4_t* transforma, void* bone_list, void* ik_context ) {
	g_Vars.globals.szLastHookCalled = XorStr( "22" );

	const auto state = ecx->m_PlayerAnimState( );

	if( !state || !state->m_Player )
		return oDoExtraBonesProccesing( ecx, hdr, pos, rotations, transforma, bone_list, ik_context );

	const auto backup_tickcount = state->m_bOnGround;
	state->m_bOnGround = false;
	oDoExtraBonesProccesing( ecx, hdr, pos, rotations, transforma, bone_list, ik_context );
	state->m_bOnGround = backup_tickcount;
}

typedef void( __thiscall* fnBuildTransformations )( C_CSPlayer*, CStudioHdr*, Vector*, Quaternion*, const matrix3x4_t&, const int32_t, BYTE* );
fnBuildTransformations oBuildTransformations;
void __fastcall hkBuildTransformations( C_CSPlayer* pPlayer, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& transform, const int32_t mask, BYTE* computed ) {
	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
	if( !pPlayer || !pLocal || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) == pLocal->EntIndex( ) || !g_Vars.rage.enabled )
		return oBuildTransformations( pPlayer, hdr, pos, q, transform, mask, computed );

	// backup jiggle bones.
	const bool m_isJiggleBonesEnabledBackup = *( bool* )( uintptr_t( pPlayer ) + 0x292C );

	// overwrite jiggle bones and refuse the game from calling the
	// code responsible for animating our attachments/weapons.
	*( bool* )( uintptr_t( pPlayer ) + 0x292C ) = false;

	oBuildTransformations( pPlayer, hdr, pos, q, transform, mask, computed );

	// restore jiggle bones
	*( bool* )( uintptr_t( pPlayer ) + 0x292C ) = m_isJiggleBonesEnabledBackup;
}

typedef void( __thiscall* fnStandardBlendingRules )( void*, CStudioHdr*, Vector*, Quaternion*, float currentTime, int boneMask );
fnStandardBlendingRules oStandardBlendingRules;
void __fastcall hkStandardBlendingRules( C_CSPlayer* pPlayer, uint32_t, CStudioHdr* hdr, Vector* pos, Quaternion* q, float currentTime, int boneMask ) {
	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer( );
	if( !pPlayer || !pLocal || !pPlayer->IsPlayer( ) || pPlayer->EntIndex( ) == pLocal->EntIndex( ) ) {
		return oStandardBlendingRules( pPlayer, hdr, pos, q, currentTime, boneMask );;
	}

	//int newBoneMask = boneMask;
	//
	//if( pPlayer->EntIndex( ) != pLocal->EntIndex( ) && g_Vars.rage.enabled )
	//	newBoneMask = BONE_USED_BY_HITBOX;

	if( !( pPlayer->m_fEffects( ) & EF_NOINTERP ) )
		pPlayer->m_fEffects( ) |= EF_NOINTERP;

	oStandardBlendingRules( pPlayer, hdr, pos, q, currentTime, boneMask );

	if( pPlayer->m_fEffects( ) & EF_NOINTERP )
		pPlayer->m_fEffects( ) &= ~EF_NOINTERP;
}

typedef bool( __thiscall* fnIsRenderableInPvs )( void*, IClientRenderable* );
fnIsRenderableInPvs oIsRenderableInPvs;
bool __fastcall hkIsRenderableInPVS( void* ecx, void* edx, IClientRenderable* pRenderable ) {
	if( !ecx || !pRenderable )
		return oIsRenderableInPvs( ecx, pRenderable );

	auto player = reinterpret_cast< C_CSPlayer* >( reinterpret_cast< uintptr_t >( pRenderable ) - 0x4 );

	if( player ) {
		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if( local ) {
			if( !player->IsTeammate( local ) )
				return true;
		}
	}

	return oIsRenderableInPvs( ecx, pRenderable );
}

typedef bool( __thiscall* fnAddRenderable )( void*, IClientRenderable*, bool, RenderableTranslucencyType_t, RenderableModelType_t, uint32 );
fnAddRenderable oAddRenderable;
void __fastcall hkAddRenderable( void* ecx, void* edx, IClientRenderable* pRenderable, bool bRenderWithViewModels, RenderableTranslucencyType_t nType, RenderableModelType_t nModelType, uint32 nSplitscreenEnabled ) {
	RenderableTranslucencyType_t type = nType;

	if( uintptr_t( pRenderable ) != 0x4 && uintptr_t( ( pRenderable - 0x4 + 0x64 ) - 1 ) <= 0x3F )
		type = RenderableTranslucencyType_t::RENDERABLE_IS_TRANSLUCENT;

	oAddRenderable( ecx, pRenderable, bRenderWithViewModels, type, nModelType, nSplitscreenEnabled );
}

//using PostNetworkDataReceived_t = void(__fastcall*)(/*std::uintptr_t, std::uintptr_t, */int);
//PostNetworkDataReceived_t oPostNetworkDataReceived;
//
//void __fastcall PostDataNetworkReceived(/*std::uintptr_t ecx, std::uintptr_t edx, */int commandsAck) {
//	if (!g_Vars.globals.HackIsReady)
//		return oPostNetworkDataReceived(/*ecx, edx, */commandsAck);
//
//	auto local = C_CSPlayer::GetLocalPlayer();
//	if (!local || local->IsDead())
//		return oPostNetworkDataReceived(/*ecx, edx, */commandsAck);
//
//	// this might be m_Split.m_nServerCommandsAck
//	auto slot = commandsAck;
//	if (!slot)
//		return oPostNetworkDataReceived(/*ecx, edx, */commandsAck);
//
//	auto predictionData = &Engine::Prediction::Instance().predictionData->m_Data[slot % 150];
//	if (predictionData) {
//
//		predictionData->m_aimPunchAngle = local->m_aimPunchAngle();
//		predictionData->m_aimPunchAngleVel = local->m_aimPunchAngleVel();
//		predictionData->m_vecViewOffset = local->m_vecViewOffset();
//		predictionData->m_vecBaseVelocity = local->m_vecBaseVelocity();
//		predictionData->m_flVelocityModifier = local->m_flVelocityModifier();
//		predictionData->m_flFallVelocity = local->m_flFallVelocity();
//		predictionData->m_vecVelocity = local->m_vecVelocity();
//		predictionData->m_nTickbase = local->m_nTickBase();
//
//		Engine::Prediction::Instance().m_bGetNetvarCompressionData = true;
//	}
//
//	oPostNetworkDataReceived(/*ecx, edx, */commandsAck);
//}

//using CPredictionCopyTransferData_t = int(__fastcall*)(void*, void*, const char*, int, datamap_t*);
//CPredictionCopyTransferData_t TransferDataOriginal;
//int __fastcall TransferData(void* edx, void* ecx, const char* name, int entIndex, datamap_t* map) {
//	//printf(XorStr("called CPredictionCopy::TransferData"));
//
//	//55 8B EC 8B 45 10 53 56 8B F1 57
//	static auto original = TransferDataOriginal;
//
//	// i cannot tell what chambers intended to do with this... maybe a return address check to this signature?
//	static auto to_post_network_data_received = reinterpret_cast<void*>(Memory::Scan(XorStr("client.dll"), XorStr("85 C0 8D 4C 24 3C")));
//	// what i can do is something ghetto aka seeing if PostNetworkDataRecieved was called or not, but something better to do may be to ask chambers what he meant with this.
//
//	Engine::Prediction::Instance()->OnFrameStageNotify();
//
//	return original(edx, ecx, name, entIndex, map);
//}

//using PhysicsSimulateFn = void( __thiscall* ) ( void* ecx );
//PhysicsSimulateFn oPhysicsSimulate;
//void __fastcall hkPhysicsSimulate( void* ecx, void* edx ) {
//	/*auto local = ( C_CSPlayer* )Interfaces::m_pEntList->GetClientEntity( Interfaces::m_pEngine->GetLocalPlayer( ) );*/
//	auto local = C_CSPlayer::GetLocalPlayer();
//	auto entity = (C_CSPlayer*)ecx;
//	if( !entity || !local || entity->EntIndex() == local->EntIndex() || local->IsDead())
//		return oPhysicsSimulate( ecx );
//
//	int nSimulationTick = *( int* )( uintptr_t( ecx ) + 0x2AC );
//	auto pCommandContext = ( C_CommandContext* )( uintptr_t( ecx ) + 0x34FC );
//
//	if( !pCommandContext || Interfaces::m_pGlobalVars->tickcount == nSimulationTick || !pCommandContext->needsprocessing )
//		return;
//
//	if( pCommandContext->cmd.tick_count >= ( g_Vars.globals.m_pCmd->tick_count + int( 1 / Interfaces::m_pGlobalVars->interval_per_tick ) + g_Vars.sv_max_usercmd_future_ticks->GetInt( ) ) ) {
//		nSimulationTick = Interfaces::m_pGlobalVars->tickcount;
//		pCommandContext->needsprocessing = false;
//
//		Engine::Prediction::Instance( )->StoreNetvarCompression( &pCommandContext->cmd );
//	}
//	else {
//		//Engine::Prediction::Instance( )->RestoreNetvarCompression( &pCommandContext->cmd );
//		oPhysicsSimulate( ecx );
//		Engine::Prediction::Instance( )->StoreNetvarCompression( &pCommandContext->cmd );
//	}
//}

typedef void( __thiscall* fnCalcViewBob ) ( C_BasePlayer*, Vector& );
fnCalcViewBob oCalcViewBob;
void __fastcall hkCalcViewBob( C_BasePlayer* player, void* edx, Vector& eyeOrigin ) {
	if( !g_Vars.esp.remove_bob )
		oCalcViewBob( player, eyeOrigin );
}


typedef bool( __thiscall* fnIsHltv )( IVEngineClient* );
fnIsHltv oIsHltv;
bool hkIsHltv( IVEngineClient* EngineClient, uint32_t ) {
	static const auto return_to_setup_velocity = Memory::Scan( XorStr( "client.dll" ), XorStr( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80" ) );
	if( _ReturnAddress( ) == ( void* )return_to_setup_velocity && g_Vars.globals.m_bUpdatingAnimations && Interfaces::m_pEngine->IsInGame( ) )
		return true;

	return oIsHltv( EngineClient );
}

typedef void( __thiscall* fnUpdateClientSideAnimation )( C_CSPlayer* player );
fnUpdateClientSideAnimation oUpdateClientSideAnimation;
void __fastcall hkUpdateClientSideAnimation( C_CSPlayer* player, void* edx ) {
	if( !player || player->IsDead( ) )
		return oUpdateClientSideAnimation( player );

	if( g_Vars.globals.m_bUpdatingAnimations )
		oUpdateClientSideAnimation( player );
}

typedef void( __thiscall* fnVertexBufferLock )( void* ecx, int max_vertex_count, bool append, void* unk );
fnVertexBufferLock oVertexBufferLock;
void __fastcall hkVertexBufferLock( void* ecx, void* edx, int max_vertex_count, bool append, void* unk ) {
	oVertexBufferLock( ecx, std::min( 32767, max_vertex_count ), append, unk );
}

typedef float( __thiscall* fnGetFloat )( void* );
fnGetFloat oRainAlphaGetFloat;
float __fastcall hkRainAlphaGetFloat( void* ecx, void* edx ) {

	if( g_Vars.esp.weather ) {
		return g_Vars.esp.weather_alpha * 0.01f;
	}

	return oRainAlphaGetFloat( ecx );
}

//typedef bool( __thiscall* fnGetBool )( void* );
//fnGetBool oSvCheatsGetBool;
//bool __fastcall sv_cheats_get_bool( void* pConVar, void* edx ) {
//	static auto ret_ard = ( uintptr_t )Memory::Scan( XorStr("client.dll"), XorStr("85 C0 75 30 38 86") );
//	if( reinterpret_cast< uintptr_t >( _ReturnAddress( ) ) == ret_ard )
//		return true;
//
//	return oSvCheatsGetBool( pConVar );
//}

typedef int(__thiscall* fnCreateMoveHltv)(void*, void*, Encrypted_t<CUserCmd2>);
fnCreateMoveHltv oCreateMoveHltv;
int __fastcall create_move_hltv(void* thisptr, void* edx, Encrypted_t<CUserCmd2> cmd) { // This was given to me by wanderer#4468 and i pasted that shit in :sunglasses: 
	if (!g_Vars.misc.bypass_mrx || !(g_Vars.misc.mind_trick && g_Vars.misc.mind_trick_bind.enabled))
		return oCreateMoveHltv(thisptr, edx, cmd);

	auto ret = oCreateMoveHltv(thisptr, edx, cmd);
	if (fabsf(cmd->move.Length()) < 21.f && fabsf(cmd->move.Length()) > FLT_EPSILON) {
		if (fabsf(cmd->move.Length()) > (21.f / 2.f)) {
			cmd->move *= 21.f / cmd->move.Length();
		}
		else
			cmd->move *= 0;
	}
	return ret;
}

namespace Interfaces
{
	Encrypted_t<IBaseClientDLL> m_pClient = nullptr;
	Encrypted_t<IClientEntityList> m_pEntList = nullptr;
	Encrypted_t<IGameMovement> m_pGameMovement = nullptr;
	Encrypted_t<IPrediction> m_pPrediction = nullptr;
	Encrypted_t<IMoveHelper> m_pMoveHelper = nullptr;
	Encrypted_t<IInput> m_pInput = nullptr;
	Encrypted_t<CGlobalVars>  m_pGlobalVars = nullptr;
	Encrypted_t<ISurface> m_pSurface = nullptr;
	Encrypted_t<IVEngineClient> m_pEngine = nullptr;
	Encrypted_t<IClientMode> m_pClientMode = nullptr;
	Encrypted_t<ICVar> m_pCvar = nullptr;
	Encrypted_t<IPanel> m_pPanel = nullptr;
	Encrypted_t<IGameEventManager> m_pGameEvent = nullptr;
	Encrypted_t<IVModelRender> m_pModelRender = nullptr;
	Encrypted_t<IMaterialSystem> m_pMatSystem = nullptr;
	Encrypted_t<ISteamClient> g_pSteamClient = nullptr;
	Encrypted_t<ISteamGameCoordinator> g_pSteamGameCoordinator = nullptr;
	Encrypted_t<ISteamMatchmaking> g_pSteamMatchmaking = nullptr;
	Encrypted_t<ISteamUser> g_pSteamUser = nullptr;
	Encrypted_t<ISteamFriends> g_pSteamFriends = nullptr;
	Encrypted_t<IPhysicsSurfaceProps> m_pPhysSurface = nullptr;
	Encrypted_t<IEngineTrace> m_pEngineTrace = nullptr;
	Encrypted_t<CGlowObjectManager> m_pGlowObjManager = nullptr;
	Encrypted_t<IVModelInfo> m_pModelInfo = nullptr;
	Encrypted_t<CClientState>  m_pClientState = nullptr;
	Encrypted_t<IVDebugOverlay> m_pDebugOverlay = nullptr;
	Encrypted_t<IEngineSound> m_pEngineSound = nullptr;
	Encrypted_t<IMemAlloc> m_pMemAlloc = nullptr;
	Encrypted_t<IViewRenderBeams> m_pRenderBeams = nullptr;
	Encrypted_t<ILocalize> m_pLocalize = nullptr;
	Encrypted_t<IStudioRender> m_pStudioRender = nullptr;
	Encrypted_t<CSPlayerResource*> m_pPlayerResource = nullptr;
	Encrypted_t<ICenterPrint> m_pCenterPrint = nullptr;
	Encrypted_t<IVRenderView> m_pRenderView = nullptr;
	Encrypted_t<IClientLeafSystem> m_pClientLeafSystem = nullptr;
	Encrypted_t<IMDLCache> m_pMDLCache = nullptr;
	Encrypted_t<IViewRender> m_pViewRender = nullptr;
	Encrypted_t<IInputSystem> m_pInputSystem = nullptr;
	Encrypted_t<INetGraphPanel> m_pNetGraphPanel = nullptr;
	Encrypted_t<CHud> m_pHud = nullptr;
	Encrypted_t<SFHudDeathNoticeAndBotStatus> g_pDeathNotices = nullptr;
	Encrypted_t<CNetworkStringTableContainer> g_pClientStringTableContainer = nullptr;
	Encrypted_t<IVEffects> g_IVEffects = nullptr;
	Encrypted_t<IEffects> g_IEffects = nullptr;


	WNDPROC oldWindowProc;
	HWND hWindow = nullptr;

	RecvPropHook::Shared m_pDidSmokeEffectSwap = nullptr;
	RecvPropHook::Shared m_pFlAbsYawSwap = nullptr;
	RecvPropHook::Shared m_pPlaybackRateSwap = nullptr;
	RecvPropHook::Shared m_bClientSideAnimationSwap = nullptr;
	RecvPropHook::Shared m_Body_proxy_swap = nullptr;
	RecvPropHook::Shared m_sequence_proxy = nullptr;


	void m_bDidSmokeEffect( CRecvProxyData* pData, void* pStruct, void* pOut ) {
		Interfaces::m_pDidSmokeEffectSwap->GetOriginalFunction( )( pData, pStruct, pOut );

		if( g_Vars.esp.remove_smoke )
			*( uintptr_t* )( ( uintptr_t )pOut ) = true;
	}

	bool Create( void* reserved ) {
		auto& pPropManager = Engine::PropManager::Instance( );

		m_pClient = ( IBaseClientDLL* )CreateInterface( XorStr( "client.dll" ), XorStr( "VClient018" ) );
		if( !m_pClient.IsValid( ) ) {
			return false;
		}
			
		g_pClientStringTableContainer = ( CNetworkStringTableContainer* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineClientStringTable001" ) );
		if( !g_pClientStringTableContainer.IsValid( ) ) {
			return false;
		}

		g_IVEffects = (IVEffects*)CreateInterface(XorStr("engine.dll"), XorStr("VEngineEffects001"));
		if (!g_IVEffects.IsValid()) {
			return false;
		}

		g_IEffects = (IEffects*)CreateInterface(XorStr("client.dll"), XorStr("IEffects001"));
		if (!g_IEffects.IsValid()) {
			return false;
		}

		if( !pPropManager->Create( m_pClient.Xor( ) ) ) {
			return false;
		}

		m_pEntList = ( IClientEntityList* )CreateInterface( XorStr( "client.dll" ), XorStr( "VClientEntityList003" ) );
		if( !m_pEntList.IsValid( ) ) {
			return false;
		}

		m_pGameMovement = ( IGameMovement* )CreateInterface( XorStr( "client.dll" ), XorStr( "GameMovement001" ) );
		if( !m_pGameMovement.IsValid( ) ) {
			return false;
		}

		m_pPrediction = ( IPrediction* )CreateInterface( XorStr( "client.dll" ), XorStr( "VClientPrediction001" ) );
		if( !m_pPrediction.IsValid( ) ) {
			return false;
		}

		m_pInput = *reinterpret_cast< IInput** > ( ( *reinterpret_cast< uintptr_t** >( m_pClient.Xor( ) ) )[ 15 ] + 0x1 );
		if( !m_pInput.IsValid( ) ) {
			return false;
		}

		m_pGlobalVars = **reinterpret_cast< CGlobalVars*** > ( ( *reinterpret_cast< uintptr_t** > ( m_pClient.Xor( ) ) )[ 0 ] + 0x1B );
		if( !m_pGlobalVars.IsValid( ) ) {
			return false;
		}

		m_pEngine = ( IVEngineClient* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineClient014" ) );
		if( !m_pEngine.IsValid( ) ) {
			return false;
		}

		m_pPanel = ( IPanel* )CreateInterface( XorStr( "vgui2.dll" ), XorStr( "VGUI_Panel009" ) );
		if( !m_pPanel.IsValid( ) ) {
			return false;
		}

		m_pSurface = ( ISurface* )CreateInterface( XorStr( "vguimatsurface.dll" ), XorStr( "VGUI_Surface031" ) );
		if( !m_pSurface.IsValid( ) ) {
			return false;
		}

		m_pClientMode = **( IClientMode*** )( ( *( DWORD** )m_pClient.Xor( ) )[ 10 ] + 0x5 );
		if( !m_pClientMode.IsValid( ) ) {
			return false;
		}

		m_pCvar = ( ICVar* )CreateInterface( XorStr( "vstdlib.dll" ), XorStr( "VEngineCvar007" ) );
		if( !m_pCvar.IsValid( ) ) {
			return false;
		}

		m_pGameEvent = ( IGameEventManager* )CreateInterface( XorStr( "engine.dll" ), XorStr( "GAMEEVENTSMANAGER002" ) );
		if( !m_pGameEvent.IsValid( ) ) {
			return false;
		}

		m_pModelRender = ( IVModelRender* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineModel016" ) );
		if( !m_pModelRender.IsValid( ) ) {
			return false;
		}

		m_pMatSystem = ( IMaterialSystem* )CreateInterface( XorStr( "materialsystem.dll" ), XorStr( "VMaterialSystem080" ) );
		if( !m_pMatSystem.IsValid( ) ) {
			return false;
		}

		m_pPhysSurface = ( IPhysicsSurfaceProps* )CreateInterface( XorStr( "vphysics.dll" ), XorStr( "VPhysicsSurfaceProps001" ) );
		if( !m_pPhysSurface.IsValid( ) ) {
			return false;
		}

		m_pEngineTrace = ( IEngineTrace* )CreateInterface( XorStr( "engine.dll" ), XorStr( "EngineTraceClient004" ) );
		if( !m_pEngineTrace.IsValid( ) ) {
			return false;
		}

		if( !Engine::CreateDisplacement( reserved ) ) {
			return false;
		}

		m_pMoveHelper = ( IMoveHelper* )( Engine::Displacement.Data.m_uMoveHelper );
		if( !m_pMoveHelper.IsValid( ) ) {
			return false;
		}

		m_pGlowObjManager = ( CGlowObjectManager* )Engine::Displacement.Data.m_uGlowObjectManager;
		if( !m_pGlowObjManager.IsValid( ) ) {
			return false;
		}

		m_pModelInfo = ( IVModelInfo* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VModelInfoClient004" ) );
		if( !m_pModelInfo.IsValid( ) ) {
			return false;
		}

		// A1 FC BC 58 10  mov eax, g_ClientState
		m_pClientState = Encrypted_t<CClientState>( **( CClientState*** )( ( *( std::uintptr_t** )m_pEngine.Xor( ) )[ 14 ] + 0x1 ) );
		if( !m_pClientState.IsValid( ) ) {
			return false;
		}

		m_pDebugOverlay = ( IVDebugOverlay* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VDebugOverlay004" ) );
		if( !m_pDebugOverlay.IsValid( ) ) {
			return false;
		}

		m_pMemAlloc = *( IMemAlloc** )( GetProcAddress( GetModuleHandle( XorStr( "tier0.dll" ) ), XorStr( "g_pMemAlloc" ) ) );
		if( !m_pMemAlloc.IsValid( ) ) {
			return false;
		}

		m_pEngineSound = ( IEngineSound* )CreateInterface( XorStr( "engine.dll" ), XorStr( "IEngineSoundClient003" ) );
		if( !m_pEngineSound.IsValid( ) ) {
			return false;
		}

		m_pRenderBeams = *( IViewRenderBeams** )( Engine::Displacement.Data.m_uRenderBeams );
		if( !m_pRenderBeams.IsValid( ) ) {
			return false;
		}

		m_pLocalize = ( ILocalize* )CreateInterface( XorStr( "localize.dll" ), XorStr( "Localize_001" ) );
		if( !m_pLocalize.IsValid( ) ) {
			return false;
		}

		m_pStudioRender = ( IStudioRender* )CreateInterface( XorStr( "studiorender.dll" ), XorStr( "VStudioRender026" ) );
		if( !m_pStudioRender.IsValid( ) ) {
			return false;
		}

		m_pCenterPrint = *( ICenterPrint** )( Engine::Displacement.Data.m_uCenterPrint );
		if( !m_pCenterPrint.IsValid( ) ) {
			return false;
		}

		m_pRenderView = ( IVRenderView* )CreateInterface( XorStr( "engine.dll" ), XorStr( "VEngineRenderView014" ) );
		if( !m_pRenderView.IsValid( ) ) {
			return false;
		}

		m_pClientLeafSystem = ( IClientLeafSystem* )CreateInterface( XorStr( "client.dll" ), XorStr( "ClientLeafSystem002" ) );
		if( !m_pClientLeafSystem.IsValid( ) ) {
			return false;
		}

		m_pMDLCache = ( IMDLCache* )CreateInterface( XorStr( "datacache.dll" ), XorStr( "MDLCache004" ) );
		if( !m_pMDLCache.IsValid( ) ) {
			return false;
		}

		m_pInputSystem = ( IInputSystem* )CreateInterface( XorStr( "inputsystem.dll" ), XorStr( "InputSystemVersion001" ) );
		if( !m_pInputSystem.IsValid( ) ) {
			return false;
		}

		m_pViewRender = **( IViewRender*** )( Memory::Scan( XorStr( "client.dll" ), XorStr( "FF 50 4C 8B 06 8D 4D F4" ) ) - 6 );
		if( !m_pViewRender.IsValid( ) ) {
			return false;
		}

		auto D3DDevice9 = **( IDirect3DDevice9*** )Engine::Displacement.Data.m_D3DDevice;
		if( !D3DDevice9 )
			return false;

		m_pHud = *( CHud** )( Memory::Scan( XorStr( "client.dll" ), XorStr( "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08" ) ) + 1 );
		if( !m_pHud.IsValid( ) )
			return false;

		g_pDeathNotices = m_pHud->FindHudElement< SFHudDeathNoticeAndBotStatus* >( XorStr( "SFHudDeathNoticeAndBotStatus" ) );
		if( !g_pDeathNotices.IsValid( ) )
			return false;

		// pMethod
		D3DDEVICE_CREATION_PARAMETERS params;
		D3DDevice9->GetCreationParameters( &params );
		hWindow = params.hFocusWindow;

		if( !InputSys::Get( )->Initialize( D3DDevice9 ) ) {
			return false;
		}

		g_pSteamClient = ( ( ISteamClient * ( __cdecl* )( void ) )GetProcAddress( GetModuleHandleA( XorStr( "steam_api.dll" ) ), XorStr( "SteamClient" ) ) )( );
		HSteamUser hSteamUser = reinterpret_cast< HSteamUser( __cdecl* ) ( void ) >( GetProcAddress( GetModuleHandle( XorStr( "steam_api.dll" ) ), XorStr( "SteamAPI_GetHSteamUser" ) ) )( );
		HSteamPipe hSteamPipe = reinterpret_cast< HSteamPipe( __cdecl* ) ( void ) >( GetProcAddress( GetModuleHandle( XorStr( "steam_api.dll" ) ), XorStr( "SteamAPI_GetHSteamPipe" ) ) )( );
		g_pSteamGameCoordinator = ( ISteamGameCoordinator* )g_pSteamClient->GetISteamGenericInterface( hSteamUser, hSteamPipe, XorStr( "SteamGameCoordinator001" ) );

		Render::Engine::Initialise( );
		Render::DirectX::init( D3DDevice9 );

		GameEvent::Get( )->Register( );

		initialize_kits( );
		
		//g_NewChams.init();
		//ISkinChanger::Get( )->Create( );

		for( auto clientclass = Interfaces::m_pClient->GetAllClasses( );
			clientclass != nullptr;
			clientclass = clientclass->m_pNext ) {
			if( !strcmp( clientclass->m_pNetworkName, XorStr( "CCSPlayer" ) ) ) {
				CCSPlayerClass = clientclass;
				oCreateCCSPlayer = ( CreateClientClassFn )clientclass->m_pCreateFn;

				clientclass->m_pCreateFn = Hooked::hkCreateCCSPlayer;
				break;
			}
		}

		if( Interfaces::m_pEngine->IsInGame( ) ) {
			for( int i = 1; i <= Interfaces::m_pGlobalVars->maxClients; ++i ) {
				auto entity = C_CSPlayer::GetPlayerByIndex( i );
				if( !entity || !entity->IsPlayer( ) )
					continue;

				auto& new_hook = Hooked::player_hooks[ i ];
				new_hook.clientHook.Create( entity );
				new_hook.renderableHook.Create( ( void* )( ( uintptr_t )entity + 0x4 ) );
				new_hook.networkableHook.Create( ( void* )( ( uintptr_t )entity + 0x8 ) );
				new_hook.SetHooks( );
			}
		}

		for( ClientClass* pClass = Interfaces::m_pClient->GetAllClasses( ); pClass; pClass = pClass->m_pNext ) {
			if( !strcmp( pClass->m_pNetworkName, XorStr( "CPlayerResource" ) ) ) {
				RecvTable* pClassTable = pClass->m_pRecvTable;

				for( int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++ ) {
					RecvProp* pProp = &pClassTable->m_pProps[ nIndex ];

					if( !pProp || strcmp( pProp->m_pVarName, XorStr( "m_iTeam" ) ) )
						continue;

					m_pPlayerResource = Encrypted_t<CSPlayerResource*>( *reinterpret_cast< CSPlayerResource*** >( std::uintptr_t( pProp->m_pDataTable->m_pProps->m_ProxyFn ) + 0x10 ) );
					break;
				}
				break;
			}
		}

		// init config system
		g_Vars.Create( );

		g_Vars.viewmodel_fov->fnChangeCallback.m_Size = 0;
		g_Vars.viewmodel_offset_x->fnChangeCallback.m_Size = 0;
		g_Vars.viewmodel_offset_y->fnChangeCallback.m_Size = 0;
		g_Vars.viewmodel_offset_z->fnChangeCallback.m_Size = 0;

		g_Vars.mat_ambient_light_r->fnChangeCallback.m_Size = 0;
		g_Vars.mat_ambient_light_g->fnChangeCallback.m_Size = 0;
		g_Vars.mat_ambient_light_b->fnChangeCallback.m_Size = 0;

		g_Vars.cl_extrapolate->SetValue( 0 );
		//g_Vars.cl_lagcompensation->SetValue( 0 );

#if defined(DEV) || defined(BETA_MODE) || defined(DEBUG_MODE)
		//	g_Vars.host_limitlocal->nFlags &= ~FCVAR_HIDDEN;
		//	g_Vars.host_limitlocal->nFlags &= ~FCVAR_CHEAT;
		//	g_Vars.host_limitlocal->fnChangeCallback.m_Size = 0;
		//	g_Vars.host_limitlocal->SetValueInt( 1 );
#endif

		RecvProp* prop = nullptr;
		pPropManager->GetProp( XorStr( "DT_SmokeGrenadeProjectile" ), XorStr( "m_bDidSmokeEffect" ), &prop );
		m_pDidSmokeEffectSwap = std::make_shared<RecvPropHook>( prop, &Hooked::m_nSmokeEffectTickBegin );

		pPropManager->GetProp( XorStr( "DT_CSRagdoll" ), XorStr( "m_flAbsYaw" ), &prop );
		m_pFlAbsYawSwap = std::make_shared<RecvPropHook>( prop, &Hooked::RecvProxy_m_flAbsYaw );

		pPropManager->GetProp( XorStr( "DT_BaseAnimating" ), XorStr( "bClientSideAnimation" ), &prop );
		m_bClientSideAnimationSwap = std::make_shared<RecvPropHook>( prop, &Hooked::m_bClientSideAnimation );

		pPropManager->GetProp(XorStr("DT_CSPlayer"), XorStr("m_flLowerBodyYawTarget"), &prop);
		m_Body_proxy_swap = std::make_shared<RecvPropHook>(prop, &Hooked::Body_proxy);

		pPropManager->GetProp(XorStr("DT_BaseViewModel"), XorStr("m_nSequence"), &prop);
		m_sequence_proxy = std::make_shared<RecvPropHook>(prop, &Hooked::sequence_proxy);

		//pPropManager->GetProp(XorStr("DT_CSPlayer"), XorStr("m_flLowerBodyYawTarget"), &prop);
		//m_Body_original = std::make_shared<RecvPropHook>(prop, &Hooked::RecvProxy_m_flLowerBodyYawTarget);

		//auto& database = pPropManager->database;
		//auto BaseOverlay = std::find( database.begin( ), database.end( ), XorStr( "DT_BaseAnimatingOverlay" ) );
		//
		//if( database.end( ) != BaseOverlay ) {
		//	auto OverlayVars = BaseOverlay->child_tables.front( );
		//	auto AnimOverlays = OverlayVars.child_tables.front( );
		//	auto anim_layer_6 = AnimOverlays.child_tables.at( 6 );
		//
		//	auto playback = anim_layer_6.child_props.at( 2 );
		//	m_pPlaybackRateSwap = std::make_shared< RecvPropHook >( playback, &Hooked::RecvProxy_PlaybackRate );
		//}

		MH_Initialize( );

		using namespace Hooked;

		// kinda clean LOL;
		RunSimulationDetor.m_nAddress = Engine::Displacement.Function.m_RunSimulation;
		if( RunSimulationDetor.m_nAddress ) {
			if( !RunSimulationDetor.Hook( ) ) {
				return false;
			}
		}

		PredictionUpdateDetor.m_nAddress = Engine::Displacement.Function.m_PredictionUpdate;
		if( PredictionUpdateDetor.m_nAddress ) {
			if( !PredictionUpdateDetor.Hook( ) ) {
				return false;
			}
		}

		Hooked::CL_FireEvents = reinterpret_cast< Hooked::CL_FireEventsFn >( Memory::Scan( XorStr( "engine.dll" ), XorStr( "55 8B EC 83 EC 08 53 8B 1D ?? ?? ?? ?? 56 57 83 BB ?? ?? 00 00 06" ) ) );

		if( !Hooked::CL_FireEvents ) {
			MessageBoxA( NULL, XorStr( "error!" ), XorStr( "" ), NULL );
			return false;
		}

		oGetScreenAspectRatio = Hooked::HooksManager.HookVirtual<decltype( oGetScreenAspectRatio )>( m_pEngine.Xor( ), &Hooked::hkGetScreenAspectRatio, Index::EngineClient::GetScreenAspectRatio );
		oFireEvents = Hooked::HooksManager.HookVirtual<decltype( oFireEvents )>( m_pEngine.Xor( ), &hkFireEvents, 59 );
		//oDispatchUserMessage = Hooked::HooksManager.HookVirtual<decltype( oDispatchUserMessage )>( m_pClient.Xor( ), &hkDispatchUserMessage, 38 );
		oIsConnected = Hooked::HooksManager.HookVirtual<decltype( oIsConnected )>( m_pEngine.Xor( ), &hkIsConnected, 27 );
		oIsBoxVisible = Hooked::HooksManager.HookVirtual<decltype( oIsBoxVisible )>( m_pEngine.Xor( ), &Hooked::hkIsBoxVisible, 32 );

		oCreateMove = Hooked::HooksManager.HookVirtual<decltype( oCreateMove )>( m_pClientMode.Xor( ), &Hooked::CreateMove, Index::CClientModeShared::CreateMove );
		oDoPostScreenEffects = Hooked::HooksManager.HookVirtual<decltype( oDoPostScreenEffects )>( m_pClientMode.Xor( ), &Hooked::DoPostScreenEffects, Index::CClientModeShared::DoPostScreenSpaceEffects );
		oOverrideView = Hooked::HooksManager.HookVirtual<decltype( oOverrideView )>( m_pClientMode.Xor( ), &Hooked::OverrideView, Index::CClientModeShared::OverrideView );
		//oRenderView = Hooked::HooksManager.HookVirtual<decltype( oRenderView )>( m_pViewRender.Xor( ), &Hooked::hkRenderView, 6 );
		//oEmitSound = Hooked::HooksManager.HookVirtual<decltype( oEmitSound )>( m_pEngineSound.Xor( ), &Hooked::hkEmitSound, 5 );

		auto SceneEnd_p = GetV(m_pRenderView.Xor(), 9);
		MH_CreateHook(SceneEnd_p, &hkSceneEnd, reinterpret_cast<void**>(&oSceneEnd));
		MH_EnableHook(SceneEnd_p);

		oFrameStageNotify = Hooked::HooksManager.HookVirtual<decltype( oFrameStageNotify )>( m_pClient.Xor( ), &Hooked::FrameStageNotify, Index::IBaseClientDLL::FrameStageNotify );
		oPreEntity = Hooked::HooksManager.HookVirtual<decltype(oPreEntity)>(m_pClient.Xor(), &Hooked::PreEntity, 5);
		//oView_Render = Hooked::HooksManager.HookVirtual<decltype( oView_Render )>( m_pClient, &Hooked::View_Render, 27 );

		oRunCommand = Hooked::HooksManager.HookVirtual<decltype( oRunCommand )>( m_pPrediction.Xor( ), &Hooked::RunCommand, Index::IPrediction::RunCommand );

		//oProcessMovement = Hooked::HooksManager.HookVirtual<decltype( oProcessMovement )>( m_pGameMovement.Xor( ), &hkProcessMovement, Index::IGameMovement::ProcessMovement );

		oDrawSetColor = Hooked::HooksManager.HookVirtual<decltype( oDrawSetColor )>( m_pSurface.Xor( ), &DrawSetColor, 60 / 4 );
		//  oEndScene = Hooked::HooksManager.HookVirtual<decltype( oEndScene )>( D3DDevice9, &Hooked::EndScene, Index::DirectX::EndScene );
		oPresent = Hooked::HooksManager.HookVirtual<decltype( oPresent )>( D3DDevice9, &Hooked::Present, Index::DirectX::Present );
		oReset = Hooked::HooksManager.HookVirtual<decltype( oReset )>( D3DDevice9, &Hooked::Reset, Index::DirectX::Reset );

		oLockCursor = Hooked::HooksManager.HookVirtual<decltype( oLockCursor )>( m_pSurface.Xor( ), &Hooked::LockCursor, Index::VguiSurface::LockCursor );

		oPaintTraverse = Hooked::HooksManager.HookVirtual<decltype( oPaintTraverse )>( m_pPanel.Xor( ), &Hooked::PaintTraverse, Index::IPanel::PaintTraverse );

		//oBeginFrame = Hooked::HooksManager.HookVirtual<decltype( oBeginFrame )>( m_pMatSystem.Xor( ), &Hooked::BeginFrame, Index::MatSystem::BeginFrame );
		oOverrideConfig = Hooked::HooksManager.HookVirtual<decltype( oOverrideConfig )>( m_pMatSystem.Xor( ), &OverrideConfig, 21 );

		oListLeavesInBox = Hooked::HooksManager.HookVirtual<decltype( oListLeavesInBox )>( Interfaces::m_pEngine->GetBSPTreeQuery( ), &Hooked::ListLeavesInBox, Index::BSPTreeQuery::ListLeavesInBox );

		static auto calc_view_bob = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9" ) );
		oCalcViewBob = Hooked::HooksManager.CreateHook<decltype( oCalcViewBob ) >( &hkCalcViewBob, ( void* )calc_view_bob );

		static auto particlesystem = Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 EC 18 56 8B F1 C7 45"));
		oRandomColor_InitNewParticlesScalar = Hooked::HooksManager.CreateHook<decltype(oRandomColor_InitNewParticlesScalar) >(&Hooked_RandomColor_InitNewParticlesScalar, (void*)particlesystem);

		static auto ShouldDrawViewModel = Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 51 57 E8"));
		oShouldDrawViewModel = Hooked::HooksManager.CreateHook<decltype(oShouldDrawViewModel) >(&hkShouldDrawViewModel, (void*)ShouldDrawViewModel);

		//oDrawModel = Hooked::HooksManager.HookVirtual<decltype( oDrawModel )>( m_pStudioRender, &Hooked::DrawModel, Index::StudioRender::DrawModel );
		oDrawModelExecute = Hooked::HooksManager.HookVirtual<decltype( oDrawModelExecute )>( m_pModelRender.Xor( ), &Hooked::DrawModelExecute, Index::ModelDraw::DrawModelExecute );

		oAddBoxOverlay = Hooked::HooksManager.HookVirtual<decltype( oAddBoxOverlay )>( m_pDebugOverlay.Xor( ), &hkAddBoxOverlay, 1 );
		
		//sv_cheats_get_bool
		//oSvCheatsGetBool = Hooked::HooksManager.HookVirtual<decltype( oSvCheatsGetBool )>( Interfaces::m_pCvar->FindVar(XorStr("sv_cheats")), &sv_cheats_get_bool, 13 );

		o_net_show_fragments = Hooked::HooksManager.HookVirtual<decltype( o_net_show_fragments )>( Interfaces::m_pCvar->FindVar(XorStr("net_showfragments")), &net_show_fragments, 13 );

		oWriteUsercmdDeltaToBuffer = Hooked::HooksManager.HookVirtual<decltype( oWriteUsercmdDeltaToBuffer )>( m_pClient.Xor( ), &WriteUsercmdDeltaToBuffer, 23 );

		//oTraceRay = Hooked::HooksManager.HookVirtual<decltype( oTraceRay )>( m_pEngineTrace.Xor( ), &TraceRay, 5 );

		auto meme = ( void* )( uintptr_t( m_pClientState.Xor( ) ) + 0x8 );
		oPacketStart = Hooked::HooksManager.HookVirtual<decltype( oPacketStart )>( meme, &Hooked::PacketStart, 5 );
		oPacketEnd = Hooked::HooksManager.HookVirtual<decltype( oPacketEnd )>( meme, &Hooked::PacketEnd, 6 );
		oProcessTempEntities = Hooked::HooksManager.HookVirtual<decltype( oProcessTempEntities )>( meme, &Hooked::ProcessTempEntities, 36 );
		oVoiceData = Hooked::HooksManager.HookVirtual<decltype(oVoiceData)>(meme, &Hooked::hkVoiceData, 24);

		//oIsPlayingDemo = Hooked::HooksManager.HookVirtual<decltype( oIsPlayingDemo )>( m_pEngine.Xor( ), &Hooked::hkIsPlayingDemo, Index::EngineClient::IsPlayingDemo );

		auto rel32_resolve = [ ] ( uintptr_t ptr ) {
			auto offset = *( uintptr_t* )( ptr + 0x1 );
			return ( uintptr_t* )( ptr + 5 + offset );
		};

		//0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F 
		//g_pSoundServices           = *( ISoundServices** )( utils::PatternScan( engine,            XorStr( "B9 ? ? ? ? 80 65 FC FE 6A 00" ) ) + 1 );
		auto soundservice = *( uintptr_t** )( Engine::Displacement.Data.m_SoundService + 1 );
		auto interpolate = Engine::Displacement.Data.m_InterpolateServerEntities;
		auto reset_contents_cache = Engine::Displacement.Data.m_ResetContentsCache;
		auto process_interpolated_list = Engine::Displacement.Data.m_ProcessInterpolatedList;
		auto cl_move = Memory::Scan( XorStr( "engine.dll" ), XorStr( "55 8B EC 81 EC ?? ?? ?? ?? 53 56 57 8B 3D ?? ?? ?? ?? 8A" ) );
		auto reporthit = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 8B 55 08 83 EC 1C F6 42 1C 01" ) );

		oModifyEyePoisition = Hooked::HooksManager.CreateHook<decltype( oModifyEyePoisition ) >( &hkModifyEyePosition, ( void* )Engine::Displacement.Data.m_ModifyEyePos );

		//55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18
		oSendNetMsg = Hooked::HooksManager.CreateHook<decltype( oSendNetMsg ) >( &Hooked::SendNetMsg, ( void* )Engine::Displacement.Data.m_SendNetMsg );
		oSendDatagram = Hooked::HooksManager.CreateHook<decltype( oSendDatagram ) >( &Hooked::SendDatagram, ( void* )Engine::Displacement.Data.SendDatagram );
		oProcessPacket = Hooked::HooksManager.CreateHook<decltype( oProcessPacket ) >( &Hooked::ProcessPacket, ( void* )Engine::Displacement.Data.ProcessPacket );
		oInterpolateServerEntities = Hooked::HooksManager.CreateHook<decltype( oInterpolateServerEntities ) >( &Hooked::InterpolateServerEntities, ( void* )interpolate );
		oProcessInterpolatedList = Hooked::HooksManager.CreateHook<decltype( oProcessInterpolatedList ) >( &hkProcessInterpolatedList, ( void* )process_interpolated_list );
		oReportHit = Hooked::HooksManager.CreateHook<decltype( oReportHit ) >( &ReportHit, ( void* )reporthit );
		oCL_Move = Hooked::HooksManager.CreateHook<decltype( oCL_Move ) >( &CL_Move, ( void* )cl_move );

		//auto physics_simulate_adr = rel32_resolve( Memory::Scan( XorStr( "client.dll" ), XorStr( "E8 ? ? ? ? 80 BE ? ? ? ? ? 0F 84 ? ? ? ? 8B 06" ) ) );
		//oPhysicsSimulate = Hooked::HooksManager.CreateHook<decltype( oPhysicsSimulate ) >( &hkPhysicsSimulate, ( void* )physics_simulate_adr );

		//static auto CPredictionCopyTranferDataHook = reinterpret_cast<void*>(Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 8B 45 10 53 56 8B F1 57")));
		//TransferDataOriginal = Hooked::HooksManager.CreateHook<decltype(TransferDataOriginal)>(&TransferData, CPredictionCopyTranferDataHook);

		//static auto ballshack123 = reinterpret_cast<void*>(Memory::Scan(XorStr("client.dll"), XorStr("E8 40 00 00 00 33 F6 6A 02")));
		//oPostNetworkDataReceived = Hooked::HooksManager.CreateHook<decltype(oPostNetworkDataReceived)>(&PostDataNetworkReceived, ballshack123);

		//oPostNetworkDataReceived = HooksManager.HookVirtual<decltype(oPostNetworkDataReceived)>(Interfaces::m_pPrediction.Xor(), &PostDataNetworkReceived, 7);

		auto standard_blending_rules_adr = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6" ) );
		oStandardBlendingRules = Hooked::HooksManager.CreateHook<decltype( oStandardBlendingRules ) >( &hkStandardBlendingRules, ( void* )standard_blending_rules_adr );

		auto build_transformations_adr = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C" ) );
		oBuildTransformations = Hooked::HooksManager.CreateHook<decltype( oBuildTransformations ) >( &hkBuildTransformations, ( void* )build_transformations_adr );

		auto debp_adr = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 53 56 8B F1 57 89 74 24 1C" ) );
		oDoExtraBonesProccesing = Hooked::HooksManager.CreateHook<decltype( oDoExtraBonesProccesing ) >( &DoExtraBonesProccesing, ( void* )debp_adr );

		oIsHltv = Hooked::HooksManager.HookVirtual<decltype( oIsHltv )>( m_pEngine.Xor( ), &hkIsHltv, Index::EngineClient::IsHltv );

		auto IsUsingStaticPropDebugModeAddr = rel32_resolve( Memory::Scan( XorStr( "engine.dll" ), XorStr( "E8 ?? ?? ?? ?? 84 C0 8B 45 08" ) ) );
		oIsUsingStaticPropDebugMode = Hooked::HooksManager.CreateHook<decltype( oIsUsingStaticPropDebugMode ) >( &IsUsingStaticPropDebugMode, ( void* )IsUsingStaticPropDebugModeAddr );

		static auto update_client_side_animation = Memory::Scan( XorStr( "client.dll" ), XorStr( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36" ) );
		oUpdateClientSideAnimation = Hooked::HooksManager.CreateHook<decltype( oUpdateClientSideAnimation ) >( &hkUpdateClientSideAnimation, ( void* )update_client_side_animation );

		static auto createmove_hltv = Memory::Scan(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B 45 08 89 0C 24"));
		oCreateMoveHltv = Hooked::HooksManager.CreateHook<decltype(oCreateMoveHltv) >(&create_move_hltv, (void*)createmove_hltv);

		Interfaces::MusicPlayer::Instance()->init();
		Interfaces::m_pEngine->ClientCmd_Unrestricted(XorStr("snd_updateaudiocache")); // update audio cache so shit works.

		Hooked::HooksManager.Enable( );
		g_lua.initialize();

		return true;
	}

	void Destroy( ) {
		Hooked::HooksManager.Restore( );

		CCSPlayerClass->m_pCreateFn = oCreateCCSPlayer;
		Hooked::player_hooks.clear( );

		GameEvent::Get( )->Shutdown( );
		GlowOutline::Get( )->Shutdown( );
		//ISkinChanger::Get( )->Destroy( );
		InputSys::Get( )->Destroy( );

		MH_Uninitialize( );

		Interfaces::m_pInputSystem->EnableInput( true );
		Interfaces::m_pClientState->m_nDeltaTick( ) = -1;
	}

	void* CreateInterface( const std::string& image_name, const std::string& name ) {
		auto image = GetModuleHandleA(image_name.c_str());
		if (!image)
			return nullptr;

		auto fn = (CreateInterfaceFn)(GetProcAddress(image, XorStr("CreateInterface")));
		if (!fn)
			return nullptr;

		return fn(name.c_str(), nullptr);
	}
}