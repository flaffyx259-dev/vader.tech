#pragma once

// https://www.unknowncheats.me/forum/counterstrike-global-offensive/391922-grenade-tracers.html

#include <deque>
#include <vector>
#include <set>
#include <unordered_map>

#include "ESP.hpp"

struct Grenade_t
{
	C_BaseEntity* entity;
	std::vector<Vector> positions;
	float addTime;
};

class CGrenade
{
public:
	CGrenade();

	bool checkGrenades(C_BaseEntity* ent);
	void addGrenade(Grenade_t grenade);
	void updatePosition(C_BaseEntity* ent, Vector position);
	void draw();

private:

	std::vector<Grenade_t> grenades;
	int findGrenade(C_BaseEntity* ent);

};

