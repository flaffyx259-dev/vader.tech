#include "../Hooked.hpp"
#include "../../Features/Visuals/CChams.hpp"
void __fastcall Hooked::hkSceneEnd(void* thisptr, void* edx)
{
	oSceneEnd();
	//g_NewChams.SceneEnd();
}