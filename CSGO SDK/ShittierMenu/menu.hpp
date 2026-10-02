#pragma once
#include "../SDK/sdk.hpp"
#include "IMGAY/imgui.h"

#include <string>

struct IDirect3DDevice9;

extern ImFont* gravity;
extern ImFont* gravityBold;

extern IDirect3DTexture9* logo_nuts;

#define InsertSliderInt(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderInt(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertSliderFloat(x1,x2,x3,x4,x5) ImGui::NewLine(); ImGui::PushItemWidth(159.f); ImGui::SliderFloat(x1, x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertCombo(x1,x2,x3) ImGui::NewLine(); ImGui::PushItemWidth(159.f); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0); ImGui::Combo(x1, x2, x3, IM_ARRAYSIZE(x3)); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0); ImGui::PopItemWidth();
#define InsertMultiCombo(x1,x2) ImGui::NewLine(); ImGui::PushItemWidth(159.f); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0); ImGui::MultiCombo(x1, x2); style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0); ImGui::PopItemWidth();
#define InsertCheckbox(x1, x2, x3) static CustomCheckbox x1; x1.Draw(x2,x3);
#define InsertEmpty() static bool Placeholder = true; static CustomCheckbox emptyc; emptyc.Draw("##", &Placeholder);

class IMGUIMenu
{
public:
    bool Initialize(IDirect3DDevice9* pDevice);
    void Shutdown();

    void OnDeviceLost();
    void OnDeviceReset();
    void CustomCheckbox(std::string title, bool* value);

    //IDirect3DTexture9* menuLogo;;

    void Loading();

    void Keybinds_Spectators();

    void Render();

    void Toggle();

    bool IsVisible() const { return _visible; }

    bool Opened = false, Initialized = false, Loaded = false;

    ImVec2 menu_position;
private:
    void CreateStyle();

    ImGuiStyle        _style;
    bool              _visible;

};

class CustomCheckbox
{
public:
    void Draw(std::string title, bool* value);
protected:
    int AnimStep;
    int LastStep;
    bool FirstDraw;
};

inline IMGUIMenu g_IMGUIMenu;