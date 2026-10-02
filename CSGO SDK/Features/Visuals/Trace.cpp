#include "Trace.h"
#include "../../SDK/Includes.hpp"
#include "../../SDK/Classes/entity.hpp"
#include "../../source.hpp"

// https://www.unknowncheats.me/forum/counterstrike-global-offensive/391922-grenade-tracers.html

CGrenade::CGrenade()
{

}

bool CGrenade::checkGrenades(C_BaseEntity* ent) //to check if we already added this grenade
{
	for (Grenade_t grenade : grenades)
		if (grenade.entity == ent)
			return false;

	return true;
}

void CGrenade::addGrenade(Grenade_t grenade)
{
	grenades.push_back(grenade);
}

void CGrenade::updatePosition(C_BaseEntity* ent, Vector position)
{
	grenades.at(findGrenade(ent)).positions.push_back(position);
}

void CGrenade::draw()
{
	//bool bInit = false; // incase of fps drops uncomment this, untested - exon


	for (size_t i = 0; i < grenades.size(); i++)
	{

		if (grenades.at(i).addTime + 2.5f < Interfaces::m_pGlobalVars->realtime)
		{
			if (grenades.at(i).positions.size() < 1) continue;

			grenades.at(i).positions.erase(grenades.at(i).positions.begin());
		}

		for (size_t j = 1; j < grenades.at(i).positions.size(); j++)
		{
				Vector2D sPosition;
				Vector2D sLastPosition;
				Vector ToDo = grenades.at(i).positions.at(j);
				if (Render::Engine::WorldToScreen(ToDo, sPosition) && Render::Engine::WorldToScreen(grenades.at(i).positions.at(j - 1), sLastPosition))
				{
					//if (bInit)
					Render::Engine::Line(sPosition.x, sPosition.y, sLastPosition.x, sLastPosition.y, g_Vars.esp.nade_tracer_color.ToRegularColor());

					//bInit = true;
				}
		}
		//bInit = false;

	}
}

int CGrenade::findGrenade(C_BaseEntity* ent)
{
	for (size_t i = 0; i < grenades.size(); i++)
		if (grenades.at(i).entity == ent)
			return i;

	return 0;
}
