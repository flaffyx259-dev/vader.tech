#include <ShlObj_core.h>
#include "walkbot.h"
#include "../../SDK/Classes/entity.hpp"
#include "../../SDK/Includes.hpp"
#include "../../SDK/Classes/Player.hpp"
#include "../Rage/Ragebot.hpp"

Color colorDefault = Color(0, 255, 0);
Color colorclose = Color(255, 0, 0);
Color colorselected = Color(0, 0, 255);
Color colortarget = Color(255, 255, 0);

float distancemeters(Vector v1, Vector v2) {
	float dist = sqrt(v1.DistanceSquared(v2));
	return dist * 0.01905f;
}
bool file_exists(LPCTSTR fname)
{
	return::GetFileAttributes(fname) != DWORD(-1);
}
void walkbot::move(Encrypted_t<CUserCmd> cmd) {
	if (!g_Vars.misc.walkbot_enable)
		return;

	if (dots.size() < 2)
		return;

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();

	if (!pLocal || !pLocal->IsAlive())
		return;

	if (marker > dots.size() - 1) {
		if (g_Vars.misc.walkbot_cyclewalk) {
			prishel = true;
			marker = dots.size() - 1;
		}
		else marker = 0;
	}

	Vector target = dots.at(marker);

	Vector lookat = target - pLocal->GetEyePosition();
	lookat.Normalize();
	lookat.ToEulerAngles().Clamp();

	if (!g_Vars.globals.RageBotTargetting) {
		cmd->viewangles = lookat.ToEulerAngles();
		cmd->forwardmove = 450;
	}
	//m_cvar()->ConsolePrintf("set to vector(%s, %s, %s), cmd->viewangldâes = vector(%s, %s, %s) \n", std::to_string(lookat.x), std::to_string(lookat.y), std::to_string(lookat.z), std::to_string(cmd->m_viewangles.x), std::to_string(cmd->m_viewangles.y), std::to_string(cmd->m_viewangles.z));

	//if (pLocal->m_vecVelocity().Length2D() < 25 && !(marker == 0 || marker == dots.size() - 1))
	//	cmd->buttons |= IN_JUMP;

	float dist3D = distancemeters(pLocal->m_vecOrigin(), target);
	if (dist3D <= 0.5f) {
		if (g_Vars.misc.walkbot_soundalert) Interfaces::m_pSurface->PlaySound_(XorStr("play buttons\\light_power_on_switch_01"));
		if (g_Vars.misc.walkbot_cyclewalk)
			prishel ? marker-- : marker++;
		else
			marker++;

		if (g_Vars.misc.walkbot_cyclewalk)
			if (marker == 0)
				prishel = false;
	}
}

void walkbot::update(bool showdebug) {

	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();

	if (g_Vars.misc.walkbot_bind.enabled) {
		button_clicked = false;
		button_down = true;
	}
	else if (!g_Vars.misc.walkbot_bind.enabled && button_down) {
		button_clicked = true;
		button_down = false;
	}
	else {
		button_clicked = false;
		button_down = false;
	}

	std::string oldmapname = mapname;
	mapname = Interfaces::m_pEngine->GetLevelName();
	if (oldmapname != mapname) {
		//refresh(mapname, dots, g_cfg.esp.walksave);
		file("_def", file_load);
		return;
	}

	int oldteamnum = teamnum;
	teamnum = pLocal->m_iTeamNum();
	if (oldteamnum != teamnum) {
		//refresh(mapname, dots, g_cfg.esp.walksave);
		marker = 0;
		prishel = false;
		return;
	}

	if (!pLocal) return;

	int w, h, cX, cY;
	Interfaces::m_pEngine->GetScreenSize(w, h);
	cX = w / 2;
	cY = h / 2;

	bool spoterased = false, spotfound = false;

	for (int i = 0; i < dots.size(); i++) {
		Vector spot = dots.at(i);
		Color c = colorDefault;

		float dist3D = distancemeters(pLocal->GetAbsOrigin(), spot);
		if (dist3D <= 1) c = colorclose;

		Vector2D pos2d;
		if (Render::Engine::WorldToScreen(spot, pos2d))
		{
			float dist2D = pos2d.Dot(Vector2D(cX, cY));
			if (dist2D <= 15 && !spotfound)
			{
				c = colorselected;
				spotfound = true;

				if (button_clicked && !spoterased)
				{
					dots.erase(dots.begin() + i);
					if (g_Vars.misc.walkbot_soundalert && g_Vars.misc.walkbot_developermode)
						Interfaces::m_pSurface->PlaySound_(XorStr("play error"));

					spoterased = true;
					continue;
				}
			}
		}

		Ray_t ray; CGameTrace tr; CTraceFilter filter;
		ray.Init(pLocal->GetEyePosition(), spot);
		filter.pSkip = pLocal;
		Interfaces::m_pEngineTrace->TraceRay(ray, 0x46004003, &filter, &tr);

		bool isvisible = tr.IsVisible();
		if (isvisible)
		{
			if (i == marker)
				if (c == colorselected)
					Interfaces::m_pDebugOverlay->AddSphereOverlay(spot, 7, 20, 20, c.r(), c.g(), c.b(), 255, 0.025f);
				else
					Interfaces::m_pDebugOverlay->AddSphereOverlay(spot, 7, 20, 20, colortarget.r(), colortarget.g(), colortarget.b(), 255, 0.025f);
			else
				Interfaces::m_pDebugOverlay->AddSphereOverlay(spot, 7, 20, 20, c.r(), c.g(), c.b(), 255, 0.025f);

			if (i - 1 >= 0)
				Interfaces::m_pDebugOverlay->AddLineOverlay(dots.at(i - 1), spot, 255, 255, 255, 255, 0.025f);
		}
		//else
		//{
		//	char dist_to[32];
		//	sprintf_s(dist_to, "%.0fm", dist3D);

		//	//if (i != marker)
		//	//	render::get().text(fonts[LOGS], pos2d.x, pos2d.y, Color(255, 255, 255, 255), HFONT_CENTERED_Y | HFONT_CENTERED_X, dist_to);
		//}

		//if (i == marker)
		//	render::get().text(fonts[LOGS], pos2d.x, pos2d.y, Color(255, 0, 0, 255), HFONT_CENTERED_Y | HFONT_CENTERED_X, crypt_str("> target <"));

		//Render::Engine::esp.string(pos2d.x, pos2d.y - 5, Color(255, 255, 255, 220), (std::string("index = " + std::to_string(i)).c_str()));
		//Render::Engine::esp.string(pos2d.x, pos2d.y + 10, Color(255, 255, 255, 220), (std::string("x = " + std::to_string(spot.x)).c_str()));
		//Render::Engine::esp.string(pos2d.x, pos2d.y + 25, Color(255, 255, 255, 220), (std::string("y = " + std::to_string(spot.y)).c_str()));
		//Render::Engine::esp.string(pos2d.x, pos2d.y + 40, Color(255, 255, 255, 220), (std::string("z = " + std::to_string(spot.z)).c_str()));
		//Render::Engine::esp.string(100, 100, Color(255, 255, 0, 220), (std::string("velocity: " + std::to_string(pLocal->m_vecVelocity().Length2D())).c_str()));
	}

	if (button_clicked && !spoterased)
	{
		Vector tracestart, traceend;
		QAngle tuda;

		Interfaces::m_pEngine->GetViewAngles(tuda);
		Math::angle_vectors(tuda, &traceend);

		tracestart = pLocal->GetEyePosition();
		traceend = tracestart + (traceend * 8192.0f);

		Ray_t ray; CGameTrace tr; CTraceFilter filter;
		ray.Init(tracestart, traceend);
		filter.pSkip = pLocal;

		Interfaces::m_pEngineTrace->TraceRay(ray, 0x46004003, &filter, &tr);

		Vector spot = tr.endpos;
		dots.push_back(spot);

		if (g_Vars.misc.walkbot_soundalert && g_Vars.misc.walkbot_developermode)
			Interfaces::m_pSurface->PlaySound_(XorStr("play resource\\warning"));
	}
}

void walkbot::file(std::string addictive_name, int todo)
{
	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();

	if (!pLocal)
		return;

	std::string levelNameFixed;
	for (size_t i = 0; i < mapname.size(); ++i)
		if (mapname[i] != '\\') levelNameFixed += mapname[i];

	if (pLocal->m_iTeamNum() > 0)
		levelNameFixed += std::to_string(pLocal->m_iTeamNum());

	levelNameFixed += addictive_name;
	static char path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path))) {
		char szCmd[256];
		sprintf(szCmd, XorStr("\\walkbot\\%s.ini"), levelNameFixed);
		folder = std::string(path) + XorStr("\\walkbot\\");
		file = std::string(path) + szCmd;
	}

	CreateDirectoryA(folder.c_str(), NULL);

	switch (todo)
	{
	case file_overwrite:
		DeleteFileA(file.c_str());
		for (int i = 0; i < dots.size(); i++) {
			Vector spot = dots.at(i);
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("x"), std::to_string(spot.x).c_str(), file.c_str());
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("y"), std::to_string(spot.y).c_str(), file.c_str());
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("z"), std::to_string(spot.z).c_str(), file.c_str());
		}

		break;

	case file_load:
		if (file_exists(file.c_str())) {
			char value[32] = { '\0' };
			dots.clear();
			int i = 0;
			bool limitreached = false;
			while (!limitreached) {
				GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("x"), "", value, 32, file.c_str());
				float x = atof(value);
				GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("y"), "", value, 32, file.c_str());
				float y = atof(value);
				GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("z"), "", value, 32, file.c_str());
				float z = atof(value);

				if (x == 0 && y == 0 && z == 0) {
					limitreached = true;
					continue;
				}
				Vector spot = Vector(x, y, z);
				dots.push_back(spot);
				i++;
			}
			marker = 0;
			prishel = false;
		}
		break;
	}

}

void walkbot::refresh(std::string levelName, std::vector<Vector> spots, bool save)
{
	C_CSPlayer* pLocal = C_CSPlayer::GetLocalPlayer();

	if (!pLocal)
		return;

	std::string levelNameFixed;
	for (size_t i = 0; i < mapname.size(); ++i)
		if (levelName[i] != '\\') levelNameFixed += levelName[i];

	if (pLocal->m_iTeamNum() > 0)
		levelNameFixed += std::to_string(pLocal->m_iTeamNum());

	static char path[MAX_PATH];
	std::string folder, file;
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path))) {
		char szCmd[256];
		sprintf(szCmd, XorStr("\\walkbot\\%s.ini"), levelNameFixed);
		folder = std::string(path) + XorStr("\\walkbot\\");
		file = std::string(path) + szCmd;
	}

	CreateDirectoryA(folder.c_str(), NULL);

	if (save) {
		DeleteFileA(file.c_str());
		for (int i = 0; i < spots.size(); i++) {
			Vector spot = spots.at(i);
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("x"), std::to_string(spot.x).c_str(), file.c_str());
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("y"), std::to_string(spot.y).c_str(), file.c_str());
			WritePrivateProfileStringA(std::to_string(i).c_str(), XorStr("z"), std::to_string(spot.z).c_str(), file.c_str());
		}
	}
	else {
		char value[32] = { '\0' };
		dots.clear();

		int i = 0;
		bool limitreached = false;

		while (!limitreached) {
			GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("x"), "", value, 32, file.c_str());
			float x = atof(value);
			GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("y"), "", value, 32, file.c_str());
			float y = atof(value);
			GetPrivateProfileStringA(std::to_string(i).c_str(), XorStr("z"), "", value, 32, file.c_str());
			float z = atof(value);

			if (x == 0 && y == 0 && z == 0) {
				limitreached = true;
				continue;
			}
			Vector spot = Vector(x, y, z);
			dots.push_back(spot);
			i++;
		}
		//refresh(mapname, dots, false);
	}
}

void walkbot::clear_dots() {
	dots.clear();
	marker = 0;
	prishel = false;
}
