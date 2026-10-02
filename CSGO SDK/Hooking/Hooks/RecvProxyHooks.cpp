#include "../Hooked.hpp"
#include "../../SDK/Displacement.hpp"
#include "../../SDK/sdk.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Valve/CBaseHandle.hpp"
#include "../../Features/Rage/LagCompensation.hpp"
#include "../../Features/Rage/Resolver.hpp"
#include "../../Utils/address.h"
#include "../../Utils/stack.h"
#include "../../Utils/modules.h"
#include "../../Utils/pattern.h"

inline int RandomSequence(int low, int high)
{
	return rand() % (high - low + 1) + low;
}

enum ESequence
{
	SEQUENCE_DEFAULT_DRAW = 0,
	SEQUENCE_DEFAULT_IDLE1 = 1,
	SEQUENCE_DEFAULT_IDLE2 = 2,
	SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
	SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
	SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
	SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
	SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
	SEQUENCE_DEFAULT_LOOKAT01 = 12,

	SEQUENCE_BUTTERFLY_DRAW = 0,
	SEQUENCE_BUTTERFLY_DRAW2 = 1,
	SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
	SEQUENCE_BUTTERFLY_LOOKAT03 = 15,

	SEQUENCE_FALCHION_IDLE1 = 1,
	SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
	SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
	SEQUENCE_FALCHION_LOOKAT01 = 12,
	SEQUENCE_FALCHION_LOOKAT02 = 13,

	SEQUENCE_DAGGERS_IDLE1 = 1,
	SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
	SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
	SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
	SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,

	SEQUENCE_BOWIE_IDLE1 = 1,
};


const static std::unordered_map<std::string, int(*)(int)> animation_fix_map
{
	{ XorStr("models/weapons/v_knife_butterfly.mdl"), [](int sequence) -> int
{
	switch (sequence)
	{
		case SEQUENCE_DEFAULT_DRAW:
			return RandomSequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomSequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
		default:
			return sequence + 1;
	}
} },
{ XorStr("models/weapons/v_knife_falchion_advanced.mdl"), [](int sequence) -> int
{
	switch (sequence)
	{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_FALCHION_IDLE1;
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return RandomSequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomSequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence - 1;
	}
} },
{ XorStr("models/weapons/v_knife_push.mdl"), [](int sequence) -> int
{
	switch (sequence)
	{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_DAGGERS_IDLE1;
		case SEQUENCE_DEFAULT_LIGHT_MISS1:
		case SEQUENCE_DEFAULT_LIGHT_MISS2:
			return RandomSequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return RandomSequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
		case SEQUENCE_DEFAULT_HEAVY_HIT1:
		case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		case SEQUENCE_DEFAULT_LOOKAT01:
			return sequence + 3;
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		default:
			return sequence + 2;
	}
} },
{ XorStr("models/weapons/v_knife_survival_bowie.mdl"), [](int sequence) -> int
{
	switch (sequence)
	{
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return sequence;
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_BOWIE_IDLE1;
		default:
			return sequence - 1;
	}
} }
};

struct StackFrame
{
	StackFrame* Next;
	DWORD Return;
};

__forceinline DWORD GetReturnAddress(int depth = 0)
{
	StackFrame* fp;

	_asm mov fp, ebp;

	for (int i = 0; i < depth; i++)
	{
		if (!fp)
			break;

		fp = fp->Next;
	}

	return fp ? fp->Return : 0;
}

namespace Hooked
{
	void m_nSmokeEffectTickBegin(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("28");

		if (!pData || !pStruct || !pOut)
			return;

		Interfaces::m_pDidSmokeEffectSwap->GetOriginalFunction()(pData, pStruct, pOut);
		if (g_Vars.esp.remove_smoke) {
			*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(pOut) + 0x1) = true;
		}
	}

	//void __cdecl RecvProxy_m_flLowerBodyYawTarget(CRecvProxyData* data, void* ptr, void* out) {

	//	if (!data || !ptr || !out)
	//		return;

	//	g_Vars.globals.szLastHookCalled = XorStr("48");

	//	static DWORD fnCopyNewEntity = Memory::Scan(XorStr("engine.dll"), XorStr("EB 3F FF 77 10"));

	//	if (fnCopyNewEntity != GetReturnAddress(2))
	//		Engine::g_Resolver.on_lby_proxy((C_CSPlayer*)ptr, &data->m_Value.m_Float);

	//	// BALLS

	//	return Interfaces::m_Body_original->GetOriginalFunction()(data, ptr, out);
	//}

	void RecvProxy_m_flAbsYaw(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("29");

		if (!pData || !pStruct || !pOut)
			return;

		Interfaces::m_pFlAbsYawSwap->GetOriginalFunction()(pData, pStruct, pOut);

		if (Interfaces::m_pEngine->IsConnected() && Interfaces::m_pEngine->IsInGame()) {
			CBaseHandle handle = *(CBaseHandle*)((uintptr_t)pStruct + Engine::Displacement.DT_CSRagdoll.m_hPlayer);
			if (handle.IsValid()) {
				auto player = (C_CSPlayer*)handle.Get();

				if (player) {
					auto lag_data = Engine::LagCompensation::Get()->GetLagData(player->EntIndex()).Xor();
					if (lag_data && lag_data->m_History.size()) {
						lag_data->m_bGotAbsYaw = true;
						lag_data->m_flAbsYawHandled = pData->m_Value.m_Float;
					}
				}
			}

			Interfaces::m_pFlAbsYawSwap->GetOriginalFunction()(pData, pStruct, pOut);
		}
	}

	void m_bClientSideAnimation(CRecvProxyData* pData, void* pStruct, void* pOut) {
		auto local = C_CSPlayer::GetLocalPlayer();
		if (!local || local->IsDead())
			return Interfaces::m_bClientSideAnimationSwap->GetOriginalFunction()(pData, pStruct, pOut);

		auto player = (C_CSPlayer*)pStruct;

		if (player && player->IsPlayer() && !player->IsTeammate(local))
			*(int*)pOut = (g_Vars.globals.m_bUpdatingAnimations ? 1 : 0);
	}

	void RecvProxy_PlaybackRate(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("47");
		// PlaybackRate
		Interfaces::m_pPlaybackRateSwap->GetOriginalFunction()(pData, pStruct, pOut);

		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer)
			return;

		auto pAnimOverlay = (C_AnimationLayer*)pStruct;
		if (pAnimOverlay) {
			auto player = (C_BasePlayer*)pAnimOverlay->m_pOwner;
			if (!player || player == LocalPlayer)
				return;

			auto& lag_data = Engine::LagCompensation::Get()->GetLagData(player->EntIndex());
			if (lag_data.Xor()) {
				lag_data->m_flRate = pAnimOverlay->m_flPlaybackRate;
				lag_data->m_bRateCheck = true;
			}
		}
	}

	struct stack_frame {
		stack_frame* next;
		DWORD ret;
	};

	__forceinline DWORD get_ret_addr(int depth = 0) {
		stack_frame* fp;

		_asm mov fp, ebp;

		for (int i = 0; i < depth; i++) {
			if (!fp)
				break;

			fp = fp->next;
		}

		return fp ? fp->ret : 0;
	}

	void Body_proxy(CRecvProxyData* pData, void* pStruct, void* pOut) {
		g_Vars.globals.szLastHookCalled = XorStr("51");

		static DWORD RecvTable_Decode = Engine::Displacement.DT_CSPlayer.m_flLowerBodyYawTargetProxy;

		const auto player = reinterpret_cast<C_CSPlayer*>(pStruct);

		// skip "unwanted" lowerbody updates originating from CopyNewEntity.
	    // happens on entity creation + pvs re-enter.

		// call from entity going into pvs.
		if (RecvTable_Decode != get_ret_addr(2)) {
			//printf("called\n");

			// store data about the update.
			Engine::g_Resolver.OnBodyUpdate(player, pData->m_Value.m_Float);
		}

		// call original proxy.
		Interfaces::m_Body_proxy_swap->GetOriginalFunction()(pData, pStruct, pOut);
	}

	void _cdecl sequence_proxy(CRecvProxyData* data_const, void* p_struct, void* p_out) {
		C_CSPlayer* LocalPlayer = C_CSPlayer::GetLocalPlayer();

		if (!LocalPlayer || !LocalPlayer->IsAlive())
			return Interfaces::m_sequence_proxy->GetOriginalFunction()(data_const, p_struct, p_out);

		const auto data = const_cast<CRecvProxyData*>(data_const);
		const auto view_model = static_cast<C_BaseViewModel*>(p_struct);
		if (view_model && data && LocalPlayer && view_model->m_hOwner() && view_model->m_hOwner().IsValid())
		{

			auto pOwner = static_cast<C_BasePlayer*>(Interfaces::m_pEntList->GetClientEntityFromHandle(view_model->m_hOwner()));

			if (pOwner == LocalPlayer) 
			{
				auto knife_model = Interfaces::m_pModelInfo->GetModel(view_model->m_nModelIndex());
				auto model_name = Interfaces::m_pModelInfo->GetModelName(knife_model);

				if (animation_fix_map.count(model_name))
				{
					data->m_Value.m_Int = animation_fix_map.at(model_name)(data->m_Value.m_Int);
				}
			}
		}

		// Call the original function with our edited data.
		Interfaces::m_sequence_proxy->GetOriginalFunction()(data, p_struct, p_out);
	}
}
