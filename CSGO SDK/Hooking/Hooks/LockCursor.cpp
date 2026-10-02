#include "../Hooked.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../../ShittierMenu/MenuNew.h"

void __stdcall Hooked::LockCursor() {
	g_Vars.globals.szLastHookCalled = XorStr("16");

	oLockCursor((void*)Interfaces::m_pSurface.Xor());

	auto pLocal = C_CSPlayer::GetLocalPlayer();

	bool state = true;
	if (!Interfaces::m_pEngine->IsInGame() || (pLocal && pLocal->IsDead()) || (Menu::initialized && ImGui::GetIO().WantTextInput)) {
		state = !Menu::opened;
	}

	Interfaces::m_pInputSystem->EnableInput(state);

	if (Menu::opened)
		Interfaces::m_pSurface->UnlockCursor();
}
