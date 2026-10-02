#include "../Hooked.hpp"
#include <intrin.h>
#include "../../Utils/InputSys.hpp"
#include "../../Features/Visuals/Hitmarker.hpp"
#include "../../Renderer/Render.hpp"
#include "../../Features/Visuals/ESP.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Classes/player.hpp"
#include "../../ShittierMenu/MenuNew.h"
#include "../../ShittierMenu/IMGAY/imgui.h"
#include "../../ShittierMenu/IMGAY/imgui_internal.h"
#include "../../ShittierMenu/IMGAY/impl/imgui_impl_dx9.h"
#include "../../ShittierMenu/IMGAY/impl/imgui_impl_win32.h"
#include "../../ShittierMenu/ImGuiRenderer.h"

DWORD dwOld_D3DRS_COLORWRITEENABLE;
IDirect3DVertexDeclaration9* vertDec;
IDirect3DVertexShader9* vertShader;

HRESULT __stdcall Hooked::Present( LPDIRECT3DDEVICE9 pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion ) {
	g_Vars.globals.szLastHookCalled = XorStr( "27" );
	g_Vars.globals.m_pD3D9Device = pDevice;
	if (GetForegroundWindow() == FindWindowA("Valve001", NULL) && InputSys::Get()->WasKeyPressed(VK_INSERT, true)) Menu::opened = !Menu::opened;

	if( Render::DirectX::initialized ) {
		InputHelper::Update( );

		//Render::DirectX::begin( );
		{
			//GUI::ctx->animation = g_Vars.globals.menuOpen ? ( GUI::ctx->animation + ( 1.0f / 0.2f ) * Interfaces::m_pGlobalVars->frametime )
			//	: ( ( GUI::ctx->animation - ( 1.0f / 0.2f ) * Interfaces::m_pGlobalVars->frametime ) );

			//if( !g_Vars.globals.menuOpen )
			//	GUI::ctx->ColorPickerInfo.HashedID = 0;

			//GUI::ctx->animation = std::clamp<float>( GUI::ctx->animation, 0.f, 1.0f );

			//Hitmarkers::RenderHitmarkers( );
			//Menu::Draw( );
		}
		//Render::DirectX::end( );

		if( g_Vars.misc.instant_stop_key.key != g_Vars.misc.slow_walk_bind.key ) {
			g_Vars.misc.instant_stop_key.key = g_Vars.misc.slow_walk_bind.key;
		}

		// chat isn't open && console isn't open
		if( !Interfaces::m_pClient->IsChatRaised( ) && !Interfaces::m_pEngine->Con_IsVisible( ) && !Menu::opened ) {
			// we aren't tabbed out
				// shit compiler, that's why no ternary operators
			if( InputHelper::Pressed( g_Vars.antiaim.manual_left_bind.key ) ) {
				if( g_Vars.globals.manual_aa == 0 )
					g_Vars.globals.manual_aa = -1;
				else {
					g_Vars.globals.manual_aa = 0;
				}
			}

			if( InputHelper::Pressed( g_Vars.antiaim.manual_right_bind.key ) ) {
				if( g_Vars.globals.manual_aa == 2 )
					g_Vars.globals.manual_aa = -1;
				else {
					g_Vars.globals.manual_aa = 2;
				}
			}

			if( InputHelper::Pressed( g_Vars.antiaim.manual_back_bind.key ) ) {
				if( g_Vars.globals.manual_aa == 1 )
					g_Vars.globals.manual_aa = -1;
				else {
					g_Vars.globals.manual_aa = 1;
				}
			}


			for( auto& keybind : g_keybinds ) {
				if( keybind == &g_Vars.misc.third_person_bind )
					continue;

				// hold
				if( keybind->cond == KeyBindType::HOLD ) {
					keybind->enabled = InputHelper::Down( keybind->key );
				}

				// toggle
				else if( keybind->cond == KeyBindType::TOGGLE ) {
					if( InputHelper::Pressed( keybind->key ) )
						keybind->enabled = !keybind->enabled;
				}

				// always on
				else if( keybind->cond == KeyBindType::ALWAYS_ON ) {
					keybind->enabled = true;
				}

				// off hold
				else if ( keybind->cond == KeyBindType::OFFHOLD ) {
					keybind->enabled = !InputHelper::Down( keybind->key );
				}
			}
		}

		// handle thirdperson keybinds just for destiny
		// #STFU!
		if( !Interfaces::m_pClient->IsChatRaised( ) && !Interfaces::m_pEngine->Con_IsVisible( ) ) {
			// hold
			if( g_Vars.misc.third_person_bind.cond == KeyBindType::HOLD ) {
				g_Vars.misc.third_person_bind.enabled = InputHelper::Down( g_Vars.misc.third_person_bind.key );
			}

			// toggle
			else if( g_Vars.misc.third_person_bind.cond == KeyBindType::TOGGLE ) {
				if( InputHelper::Pressed( g_Vars.misc.third_person_bind.key ) )
					g_Vars.misc.third_person_bind.enabled = !g_Vars.misc.third_person_bind.enabled;
			}

			// always on
			else if( g_Vars.misc.third_person_bind.cond == KeyBindType::ALWAYS_ON ) {
				g_Vars.misc.third_person_bind.enabled = true;
			}
		}

		InputSys::Get( )->SetScrollMouse( 0.f );
	}

	if (!Menu::Loaded && Menu::Initialize(pDevice)) {
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vertDec);
		pDevice->GetVertexShader(&vertShader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		{
			Menu::Loading();
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vertDec);
		pDevice->SetVertexShader(vertShader);
	}
	else if (Menu::Loaded && Menu::Initialize(pDevice))
	{
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vertDec);
		pDevice->GetVertexShader(&vertShader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		//PostProcessing::setDevice(pDevice);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		/*render stuff*/
		{
			g_ImGuiRender->BeginScene();
			g_ImGuiRender->EndScene();
			Menu::Render();
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vertDec);
		pDevice->SetVertexShader(vertShader);
	}

	return oPresent( pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}