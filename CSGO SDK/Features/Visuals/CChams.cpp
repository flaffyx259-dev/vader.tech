#include "CChams.hpp"
#include "../../source.hpp"
#include <fstream>
#include "../../SDK/Classes/Player.hpp"
#include "../../SDK/Classes/IMaterialSystem.hpp"
#include "../../SDK/CVariables.hpp"
#include "../Rage/LagCompensation.hpp"
#include "../Game/SetupBones.hpp"
#include "../../Hooking/hooked.hpp"
#include "../../SDK/displacement.hpp"
#include "../Rage/ShotInformation.hpp"
#include "../Game/Prediction.hpp"
#include "../Rage/Resolver.hpp"

extern C_AnimationLayer FakeAnimLayers[ 13 ];

#pragma optimize( "", off )



model_type_t C_NewChams::GetModelType(const ModelRenderInfo_t& info) {
	// model name.
	const char* cmdl = info.pModel->szName;

	std::string mdl{ info.pModel->szName };

	static auto int_from_chars = [cmdl](size_t index) {
		return *(int*)(cmdl + index);
	};

	// little endian.
	if (int_from_chars(7) == 'paew') { // weap
		if (int_from_chars(15) == 'om_v' && int_from_chars(19) == 'sled')
			return model_type_t::arms;

		if (cmdl[15] == 'v')
			return model_type_t::view_weapon;
	}

	//else if( int_from_chars( 7 ) == 'yalp' ) // play
	//	return model_type_t::player;

	if (mdl.find("player") != std::string::npos && info.entity_index >= 1 && info.entity_index <= 64)
		return model_type_t::player;

	return model_type_t::invalid;
}

bool C_NewChams::IsInViewPlane(const Vector& world) {
	float w;

	const VMatrix& matrix = Interfaces::m_pEngine->WorldToScreenMatrix();

	w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];

	return w > 0.001f;
}

void C_NewChams::SetColor(FloatColor col, IMaterial* mat) {
	if (mat)
		mat->ColorModulate(col.r, col.g, col.b);

	else
		Interfaces::m_pRenderView->SetColorModulation(col);
}

void C_NewChams::SetAlpha(float alpha, IMaterial* mat) {
	if (mat)
		mat->AlphaModulate(alpha);

	else
		Interfaces::m_pRenderView->SetBlend(alpha);
}

void C_NewChams::SetupMaterial(IMaterial* mat, FloatColor col, bool z_flag) {
	SetColor(col);

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, z_flag);
	mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, z_flag);
	mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, z_flag);

	Interfaces::m_pStudioRender->ForcedMaterialOverride(mat);
}

void C_NewChams::init() {
	std::ofstream("csgo\\materials\\simple_flat.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\simple_ignorez_reflective.vmt") << R"#("VertexLitGeneric"
{

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast"             "1"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	std::ofstream("csgo\\materials\\simple_regular_reflective.vmt") << R"#("VertexLitGeneric"
{

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       "env_cubemap"
  "$normalmapalphaenvmapmask"  "1"
  "$envmapcontrast"             "1"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";


	std::ofstream("csgo/materials/chams.vmt") << R"#("VertexLitGeneric"
{
	  "$basetexture" "vgui/white_additive"
	  "$ignorez" "0"
	  "$additive" "0"
	  "$envmap"  "models/effects/cube_white"
	  "$normalmapalphaenvmapmask" "1"
	  "$envmaptint" "[0.37 0.68 0.89]"
	  "$envmapfresnel" "1"
	  "$envmapfresnelminmaxexp" "[0 1 2]"

	  "$envmapcontrast" "1"
	  "$nofog" "1"
	  "$model" "1"
	  "$nocull" "0"
	  "$selfillum" "1"
	  "$halflambert" "1"
	  "$znearer" "0"
	  "$flat" "1"
}
)#";

	std::ofstream("csgo/materials/Overlay.vmt") << R"#(" VertexLitGeneric "{

			"$additive" "1"
	        "$envmap"  "models/effects/cube_white"
			"$envmaptint" "[0 0 0]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[0 16 12]"
			"$alpha" "0.8"
})#";

	std::ofstream("csgo/materials/glowOverlay.vmt") << R"#("VertexLitGeneric" {
				"$additive" "1"
				"$envmap" "models/effects/cube_white"
				"$envmaptint" "[0 0.1 0.2]"
				"$envmapfresnel" "1"
				"$envmapfresnelminmaxexp" "[0 1 2]"
				"$ignorez" "1"
				"$alpha" "1"
			})#";

	std::ofstream("csgo/materials/ShotChams.vmt") << R"#("VertexLitGeneric" {
				"$additive" "1"
				"$envmap" "models/effects/cube_white"
				"$envmaptint" "[0 0.1 0.2]"
				"$envmapfresnel" "1"
				"$envmapfresnelminmaxexp" "[0 1 2]"
				"$ignorez" "1"
				"$alpha" "1"
			})#";

	std::ofstream("csgo/materials/cubemap.vmt") << R"#("VertexLitGeneric" { 

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$envmaptint"   "[0.6 0.6 0.6]"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
	})#";

	std::ofstream("csgo/materials/outline2.vmt") << R"#("VertexLitGeneric" {
 
		"$additive" "1"
		"$envmap" "models/effects/cube_white"
		"$envmaptint" "[1 1 1]"
		"$envmapfresnel" "1"
		"$envmapfresnelminmaxexp" "[0 1 2]"
		"$alpha" "0.8"
	})#";

	materialMetall = Interfaces::m_pMatSystem->FindMaterial("simple_ignorez_reflective", "Model textures");
	materialMetall->IncrementReferenceCount();

	materialMetallnZ = Interfaces::m_pMatSystem->FindMaterial("simple_regular_reflective", "Model textures");
	materialMetallnZ->IncrementReferenceCount();

	// find stupid materials.
	debugambientcube = Interfaces::m_pMatSystem->FindMaterial("debug/debugambientcube", "Model textures");
	debugambientcube->IncrementReferenceCount();

	debugdrawflat = Interfaces::m_pMatSystem->FindMaterial("debug/debugdrawflat", "Model textures");
	debugdrawflat->IncrementReferenceCount();

	materialMetall3 = Interfaces::m_pMatSystem->FindMaterial("cubemap", "Model textures");
	materialMetall3->IncrementReferenceCount();

	skeet = Interfaces::m_pMatSystem->FindMaterial("chams", "Model textures");
	skeet->IncrementReferenceCount();

	onetap = Interfaces::m_pMatSystem->FindMaterial("Overlay", "Model textures");
	onetap->IncrementReferenceCount();

	aimware = Interfaces::m_pMatSystem->FindMaterial("GlowOverlay", "Model textures");
	aimware->IncrementReferenceCount();

	shotchams = Interfaces::m_pMatSystem->FindMaterial("ShotChams", "Model textures");
	shotchams->IncrementReferenceCount();

	blinking = Interfaces::m_pMatSystem->FindMaterial("models/inventory_items/dogtags/dogtags_outline", "Model textures");
	blinking->IncrementReferenceCount();

	// Armsrace
	debugglow = Interfaces::m_pMatSystem->FindMaterial("dev/glow_armsrace", nullptr);
	debugglow->IncrementReferenceCount();

	// bubbly?
	debugbubbly = Interfaces::m_pMatSystem->FindMaterial("outline2", nullptr);
	debugbubbly->IncrementReferenceCount();

	crystalblue = Interfaces::m_pMatSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_blue", nullptr);
	crystalblue->IncrementReferenceCount();

	italy = Interfaces::m_pMatSystem->FindMaterial("decals/italy_flag", nullptr);
	italy->IncrementReferenceCount();

}

bool C_NewChams::OverridePlayer(int index) {
	C_CSPlayer* player = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(index);
	if (!player)
		return false;


	if (!C_CSPlayer::GetLocalPlayer()) return false;
	auto private_localPlayer = C_CSPlayer::GetLocalPlayer();

	// always skip the local player in DrawModelExecute.
	// this is because if we want to make the local player have less alpha
	// the static props are drawn after the players and it looks like aids.
	// therefore always process the local player in scene end.
	if (player == private_localPlayer)
		return true;

	// see if this player is an enemy to us.
	bool enemy = !player->IsTeammate(private_localPlayer);


	// we have chams on enemies.
	if (enemy && (g_Vars.esp.new_chams_enemy || g_Vars.esp.new_chams_enemy_overlay))
		return true;

	return false;
}

bool C_NewChams::DrawModel(const ModelRenderInfo_t& info) {
	// store and validate model type.
	model_type_t type = GetModelType(info);
	if (type == model_type_t::invalid)
		return true;

	// is a valid player.
	if (type == model_type_t::player) {
		// do not cancel out our own calls from SceneEnd
		// also do not cancel out calls from the glow.
		if (!m_running && !Interfaces::m_pStudioRender->m_pForcedMaterial && OverridePlayer(info.entity_index))
			return false;
	}

	//if (type == model_type_t::view_weapon && !strstr(info.m_model->m_name, "arms") && g_cl.m_local->alive()) {
	//	RenderWeapon(ecx, ctx, state, info, bone);
	//	if (!m_weapon_running && !g_csgo.m_studio_render->m_pForcedMaterial)
	//		return false;
	//}

	return true;
}

void C_NewChams::SceneEnd() {
	if (!C_CSPlayer::GetLocalPlayer()) return;
	localPlayer = C_CSPlayer::GetLocalPlayer();

	// store and sort ents by distance.
	if (SortPlayers()) {
		// iterate each player and render them.
		for (const auto& p : m_players)
			RenderPlayer(p);
	}

	// restore.
	Interfaces::m_pStudioRender->ForcedMaterialOverride(nullptr);
	Interfaces::m_pRenderView->SetColorModulation(Color(255, 255, 255, 255));
	Interfaces::m_pRenderView->SetBlend(1.f);
}

IMaterial* EnemyXQZMat = nullptr, * EnemyOverlayXQZMat = nullptr, * EnemyMat = nullptr, * LocalMat = nullptr, * LocalOverlayMat = nullptr;
void C_NewChams::RenderPlayer(C_CSPlayer* player) {
	if (!localPlayer && localPlayer->IsDormant()) return;
	// prevent recruisive model cancelation.
	m_running = true;

	// restore.
	Interfaces::m_pStudioRender->ForcedMaterialOverride(nullptr);
	Interfaces::m_pRenderView->SetColorModulation(Color::White());
	Interfaces::m_pRenderView->SetBlend(1.f);

	// this is the local player.
	// we always draw the local player manually in drawmodel.
	if (player == localPlayer) {

		if (g_Vars.esp.new_chams_local_original_model || g_Vars.esp.new_chams_local == 0 && g_Vars.esp.new_chams_local_overlay == 0) { // profit
			player->DrawModel();
		}

		/*draw normal chams*/
		if (player->m_bIsScoped() && g_Vars.esp.new_chams_local_scoped_enabled) {
			SetAlpha(g_Vars.esp.new_chams_local_scoped_alpha);
		}

		else {
			SetAlpha(g_Vars.esp.new_chams_local_color.a);
		}

		// set material and color.
		switch (g_Vars.esp.new_chams_local)
		{
		case 0:
		{
			LocalMat = nullptr;
			break;
		}
		case 1:
		{
			LocalMat = debugambientcube;
			break;
		}
		case 2:
		{
			LocalMat = debugdrawflat;
			break;
		}
		case 3:
		{
			LocalMat = materialMetall3;
			break;
		}
		}
		if (LocalMat)
		{
			bool bFound = false;
			auto pVar = LocalMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(g_Vars.esp.new_chams_local_color.r, g_Vars.esp.new_chams_local_color.g, g_Vars.esp.new_chams_local_color.b);

			SetupMaterial(LocalMat, g_Vars.esp.new_chams_local_color, false);
			player->DrawModel();
		}

		/*draw chams over normal chams :)*/
		SetAlpha(g_Vars.esp.new_chams_local_overlay_color.a);
		switch (g_Vars.esp.new_chams_local_overlay)
		{
		case 0:
		{
			LocalOverlayMat = nullptr;
			break;
		}
		case 1:
		{
			LocalOverlayMat = aimware;
			break;
		}
		case 2:
		{
			LocalOverlayMat = blinking;
			break;
		}
		case 3:
		{
			LocalOverlayMat = onetap;
			break;
		}
		case 4:
		{
			LocalOverlayMat = debugglow;
			break;
		}
		}
		if (LocalOverlayMat)
		{
			bool bFound = false;
			auto pVar = LocalOverlayMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(g_Vars.esp.new_chams_local_overlay_color.r, g_Vars.esp.new_chams_local_overlay_color.g, g_Vars.esp.new_chams_local_overlay_color.b);

			SetupMaterial(LocalOverlayMat, g_Vars.esp.new_chams_local_overlay_color, false);
			player->DrawModel();
		}

	}

	// check if is an enemy.
	bool enemy = !player->IsTeammate(localPlayer);

	//if (enemy && g_cfg::c_bool["chams_enemy_backtrack"]) {
	//	RenderHistoryChams(player->EntIndex());
	//}

	if (enemy && !player->m_bGunGameImmunity()) {
		if ((g_Vars.esp.new_chams_enemy_overlay || g_Vars.esp.new_chams_enemy_xqz) && g_Vars.esp.new_chams_enemy == 0) player->DrawModel(); // draw original model if overlay only is on, otherwise unneeded because of the normal chams

		/*draw overlay xqz chams*/
		SetAlpha(g_Vars.esp.new_chams_enemy_xqz_color.a);
		switch (g_Vars.esp.new_chams_enemy_xqz)
		{
			case 0:
			{
				EnemyXQZMat = nullptr;
				break;
			}
			case 1:
			{
				EnemyXQZMat = debugambientcube;
				break;
			}
			case 2:
			{
				EnemyXQZMat = debugdrawflat;
				break;
			}
			case 3:
			{
				EnemyXQZMat = materialMetall3;
				break;
			}
		}
		if (EnemyXQZMat)
		{
			bool bFound = false;
			auto pVar = EnemyXQZMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(g_Vars.esp.new_chams_enemy_xqz_color.r, g_Vars.esp.new_chams_enemy_xqz_color.g, g_Vars.esp.new_chams_enemy_xqz_color.b);

			SetupMaterial(EnemyXQZMat, g_Vars.esp.new_chams_enemy_xqz_color, true);
			player->DrawModel();
		}

		/*draw normal xqz chams*/
		SetAlpha(g_Vars.esp.new_chams_enemy_xqz_overlay_color.a);
		switch (g_Vars.esp.new_chams_enemy_xqz_overlay)
		{
			case 0:
			{
				EnemyOverlayXQZMat = nullptr;
				break;
			}
			case 1:
			{
				EnemyOverlayXQZMat = aimware;
				break;
			}
			case 2:
			{
				EnemyOverlayXQZMat = blinking;
				break;
			}
			case 3:
			{
				EnemyOverlayXQZMat = onetap;
				break;
			}
			case 4:
			{
				EnemyOverlayXQZMat = debugglow;
				break;
			}
		}
		if (EnemyOverlayXQZMat)
		{
			auto chams_color = g_Vars.esp.new_chams_enemy_xqz_overlay_color;
			bool bFound = false;
			auto pVar = EnemyOverlayXQZMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(chams_color.r, chams_color.g, chams_color.b);

			SetupMaterial(EnemyOverlayXQZMat, chams_color, true);
			player->DrawModel();
		}

		/*draw normal chams*/
		SetAlpha(g_Vars.esp.new_chams_enemy_color.a);
		switch (g_Vars.esp.new_chams_enemy)
		{
		case 0:
		{
			EnemyMat = nullptr;
			break;
		}
		case 1:
		{
			EnemyMat = debugambientcube;
			break;
		}
		case 2:
		{
			EnemyMat = debugdrawflat;
			break;
		}
		case 3:
		{
			EnemyMat = materialMetall3;
			break;
		}
		}
		if (EnemyMat)
		{
			auto chams_color = g_Vars.esp.new_chams_enemy_color;
			bool bFound = false;
			auto pVar = EnemyMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(chams_color.r, chams_color.g, chams_color.b);

			SetupMaterial(EnemyMat, chams_color, false);
			player->DrawModel();
		}

		/*draw chams over normal chams :)*/
		SetAlpha(g_Vars.esp.new_chams_enemy_overlay_color.a);
		IMaterial* EnemyOverlayMat = nullptr;
		switch (g_Vars.esp.new_chams_enemy_overlay)
		{
		case 0:
		{
			EnemyOverlayMat = nullptr;
			break;
		}
		case 1:
		{
			EnemyOverlayMat = aimware;
			break;
		}
		case 2:
		{
			EnemyOverlayMat = blinking;
			break;
		}
		case 3:
		{
			EnemyOverlayMat = onetap;
			break;
		}
		case 4:
		{
			EnemyOverlayMat = debugglow;
			break;
		}
		}
		if (EnemyOverlayMat)
		{
			bool bFound = false;
			auto pVar = EnemyOverlayMat->FindVar("$envmaptint", &bFound);
			pVar->SetVecValue(g_Vars.esp.new_chams_enemy_overlay_color.r, g_Vars.esp.new_chams_enemy_overlay_color.g, g_Vars.esp.new_chams_enemy_overlay_color.b);

			SetupMaterial(EnemyOverlayMat, g_Vars.esp.new_chams_enemy_overlay_color, false);
			player->DrawModel();
		}
	}

	if (player == localPlayer) {
		SetAlpha(g_Vars.esp.new_chams_local_fake_color.a);
		IMaterial* LocalFakeMat = nullptr;
		switch (g_Vars.esp.new_chams_local_fake)
		{
		case 0:
		{
			LocalFakeMat = nullptr;
			break;
		}
		case 1:
		{
			LocalFakeMat = debugambientcube;
			break;
		}
		case 2:
		{
			LocalFakeMat = debugdrawflat;
			break;
		}
		case 3:
		{
			LocalFakeMat = materialMetall3;
			break;
		}
		}
		if (LocalFakeMat)
		{
			SetupMaterial(LocalFakeMat, g_Vars.esp.new_chams_local_fake_color, false);
			//g_cl.SetAngles2(ang_t(0.f, g_cl.m_radar.y, 0.f));
			player->DrawModel();
		}
	}

	m_running = false;
}

void C_NewChams::OnPostScreenEffects() {
	auto pLocal = C_CSPlayer::GetLocalPlayer();

	if (!g_Vars.globals.HackIsReady || !Interfaces::m_pEngine->IsConnected() || !Interfaces::m_pEngine->IsInGame() || !pLocal) {
		m_Hitmatrix.clear();
		return;
	}

	if (m_Hitmatrix.empty())
		return;

	if (!Interfaces::m_pStudioRender.IsValid())
		return;

	auto ctx = Interfaces::m_pMatSystem->GetRenderContext();

	if (!ctx)
		return;

	auto DrawModelRebuild = [&](C_HitMatrixEntry it) -> void {
		if (!g_Vars.r_drawmodelstatsoverlay)
			return;

		DrawModelResults_t results;
		DrawModelInfo_t info;
		ColorMeshInfo_t* pColorMeshes = NULL;
		info.m_bStaticLighting = false;
		info.m_pStudioHdr = it.state.m_pStudioHdr;
		info.m_pHardwareData = it.state.m_pStudioHWData;
		info.m_Skin = it.info.skin;
		info.m_Body = it.info.body;
		info.m_HitboxSet = it.info.hitboxset;
		info.m_pClientEntity = (IClientRenderable*)it.state.m_pRenderable;
		info.m_Lod = it.state.m_lod;
		info.m_pColorMeshes = pColorMeshes;

		bool bShadowDepth = (it.info.flags & STUDIO_SHADOWDEPTHTEXTURE) != 0;

		// Don't do decals if shadow depth mapping...
		info.m_Decals = bShadowDepth ? STUDIORENDER_DECAL_INVALID : it.state.m_decals;

		// Sets up flexes
		float* pFlexWeights = NULL;
		float* pFlexDelayedWeights = NULL;

		int overlayVal = g_Vars.r_drawmodelstatsoverlay->GetInt();
		int drawFlags = it.state.m_drawFlags;

		if (bShadowDepth) {
			drawFlags |= STUDIORENDER_DRAW_OPAQUE_ONLY;
			drawFlags |= STUDIORENDER_SHADOWDEPTHTEXTURE;
		}

		if (overlayVal && !bShadowDepth) {
			drawFlags |= STUDIORENDER_DRAW_GET_PERF_STATS;
		}

		Interfaces::m_pStudioRender->DrawModel(&results, &info, it.pBoneToWorld, pFlexWeights, pFlexDelayedWeights, it.info.origin, drawFlags);
		Interfaces::m_pStudioRender->m_pForcedMaterial = nullptr;
		Interfaces::m_pStudioRender->m_nForcedMaterialType = 0;
	};

	auto it = m_Hitmatrix.begin();
	while (it != m_Hitmatrix.end()) {
		if (!(&it->state) || !it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
			++it;
			continue;
		}

		auto alpha = 1.0f;
		auto delta = Interfaces::m_pGlobalVars->realtime - it->time;
		if (delta > 0.0f) {
			alpha -= delta;
			if (delta > 1.0f) {
				it = m_Hitmatrix.erase(it);
				continue;
			}
		}

		FloatColor color = g_Vars.esp.hitmatrix_color;
		color.a *= alpha;

		bool bFound = false;
		auto pVar = shotchams->FindVar("$envmaptint", &bFound);
		pVar->SetVecValue(color.r, color.g, color.b);


		SetupMaterial(shotchams, color, true);

		DrawModelRebuild(*it);

		//if (g_Vars.esp.chams_hitmatrix_outline) {
		//	SetupMaterial(aimware, color, true);

		//	DrawModelRebuild(*it);
		//}

		++it;
	}
}

void C_NewChams::AddHitmatrix(C_CSPlayer* player, matrix3x4_t* bones) {
	if (!player || !bones)
		return;

	auto& hit = m_Hitmatrix.emplace_back();

	std::memcpy(hit.pBoneToWorld, bones, player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
	hit.time = Interfaces::m_pGlobalVars->realtime + g_Vars.esp.hitmatrix_time;

	static int m_nSkin = SDK::Memory::FindInDataMap(player->GetPredDescMap(), XorStr("m_nSkin"));
	static int m_nBody = SDK::Memory::FindInDataMap(player->GetPredDescMap(), XorStr("m_nBody"));

	hit.info.origin = player->GetAbsOrigin();
	hit.info.angles = player->GetAbsAngles();

	auto renderable = player->GetClientRenderable();

	if (!renderable)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto hdr = *(studiohdr_t**)(player->m_pStudioHdr());

	if (!hdr)
		return;

	hit.state.m_pStudioHdr = hdr;
	hit.state.m_pStudioHWData = Interfaces::m_pMDLCache->GetHardwareData(model->studio);
	hit.state.m_pRenderable = renderable;
	hit.state.m_drawFlags = 0;

	hit.info.pRenderable = renderable;
	hit.info.pModel = model;
	hit.info.pLightingOffset = nullptr;
	hit.info.pLightingOrigin = nullptr;
	hit.info.hitboxset = player->m_nHitboxSet();
	hit.info.skin = *(int*)(uintptr_t(player) + m_nSkin);
	hit.info.body = *(int*)(uintptr_t(player) + m_nBody);
	hit.info.entity_index = player->m_entIndex;
	hit.info.instance = Memory::VCall<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30u)(renderable);
	hit.info.flags = 0x1;

	hit.info.pModelToWorld = &hit.model_to_world;
	hit.state.m_pModelToWorld = &hit.model_to_world;

	hit.model_to_world.AngleMatrix(hit.info.angles, hit.info.origin);
}

bool C_NewChams::SortPlayers() {
	if (!localPlayer && localPlayer->IsDormant()) return false;
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = [](C_CSPlayer* a, C_CSPlayer* b) {
		Vector local = g_NewChams.localPlayer->GetAbsOrigin();

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		float len1 = (a->GetAbsOrigin() - local).LengthSquared();
		float len2 = (b->GetAbsOrigin() - local).LengthSquared();

		return len1 < len2;
	};

	// reset player container.
	m_players.clear();

	// find all players that should be rendered.
	for (int i{ 1 }; i <= Interfaces::m_pGlobalVars->maxClients; ++i) {
		// get player ptr by idx.
		C_CSPlayer* player = (C_CSPlayer*)Interfaces::m_pEntList->GetClientEntity(i);

		// validate.
		if (!player || !player->IsPlayer() || player->IsDead())
			continue;

		// do not draw players occluded by view plane.
		if (!IsInViewPlane(player->WorldSpaceCenter()))
			continue;

		// this player was not skipped to draw later.
		// so do not add it to our render list.
		if (!OverridePlayer(i))
			continue;

		m_players.push_back(player);
	}

	// any players?
	if (m_players.empty())
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort(m_players.begin(), m_players.end(), distance_predicate);

	return true;
}

namespace Interfaces
{
	enum ChamsMaterials {
		MATERIAL_OFF = 0,
		MATERIAL_FLAT = 1,
		MATERIAL_REGULAR,
		MATERIAL_SHINY,
		MATERIAL_GLOW,
		MATERIAL_BLINKING,
		MATERIAL_WIREFRAME,
		MATERIAL_OUTLINE,
		MATERIAL_ANIMATEDLINE,
	};

	struct C_HitMatrixEntry {
		int ent_index;
		ModelRenderInfo_t info;
		DrawModelState_t state;
		matrix3x4_t pBoneToWorld[ 128 ] = { };
		float time;
		matrix3x4_t model_to_world;
	};

	class CChams : public IChams {
	public:
		void OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) override;
		void DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) override;
		bool GetBacktrackMatrix( C_CSPlayer* player, matrix3x4_t* out );
		void OnPostScreenEffects() override;
		bool IsVisibleScan(C_CSPlayer* player);	  virtual void AddHitmatrix(C_CSPlayer* player, matrix3x4_t* bones);


		CChams( );
		~CChams( );

		virtual bool CreateMaterials( ) {
			if( m_bInit )
				return true;

			m_matRegular = Interfaces::m_pMatSystem->FindMaterial( ( "debug/debugambientcube" ), nullptr );
			m_matFlat = Interfaces::m_pMatSystem->FindMaterial( ( "debug/debugdrawflat" ), nullptr );
			m_matGlow = Interfaces::m_pMatSystem->FindMaterial( ( "dev/glow_armsrace" ), nullptr );

			std::ofstream( "csgo\\materials\\vader_custom.vmt" ) << R"#("VertexLitGeneric"
			{
					"$basetexture" "vgui/white_additive"
					"$ignorez"      "0"
					"$phong"        "1"
					"$BasemapAlphaPhongMask"        "1"
					"$phongexponent" "15"
					"$normalmapalphaenvmask" "1"
					"$envmap"       "env_cubemap"
					"$envmaptint"   "[0.6 0.6 0.6]"
					"$phongboost"   "[0.6 0.6 0.6]"
					"phongfresnelranges"   "[0.5 0.5 1.0]"
					"$nofog"        "1"
					"$model"        "1"
					"$nocull"       "0"
					"$selfillum"    "1"
					"$halflambert"  "1"
					"$znearer"      "0"
					"$flat"         "1"
			}
			)#";

			std::ofstream("csgo/materials/animated_wireframe_vader.vmt") << R"#("VertexLitGeneric" {
				"$basetexture"	"models/inventory_items/dogtags/dogtags_lightray"
				"$additive"		"1"
				"$vertexcolor"	"1"
				"$vertexalpha"	"1"
				"$translucent"	"1"
				proxies 
				{ 
					texturescroll 
					{ 
						"texturescrollvar"		"$basetexturetransform" 
						"texturescrollrate"		"0.8" 
						"texturescrollangle"	"130"
					}
				}
            }
			)#";

			std::ofstream("csgo/materials/glowOverlay.vmt") << R"#("VertexLitGeneric" {
				"$additive" "1"
				"$envmap" "models/effects/cube_white"
				"$envmaptint" "[0 0.1 0.2]"
				"$envmapfresnel" "1"
				"$envmapfresnelminmaxexp" "[0 1 2]"
				"$ignorez" "1"
				"$alpha" "1"
			}
			)#";


			std::ofstream("csgo/materials/LineOverlay.vmt") << R"#("VertexLitGeneric" {
                "$basetexture"                "particle\confetti\confetti"
                "$additive"                    "1"
                "$envmap"                    "editor/cube_vertigo"
                "$envmaptint"                "[0 0.5 0.55]"
                "$envmapfresnel"            "1"
                "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
                "$alpha"                    "1"
 
                      Proxies
                {
                    TextureScroll
                    {
                        "texturescrollvar"            "$baseTextureTransform"
                        "texturescrollrate"            "0.30"
                        "texturescrollangle"        "270"
                    }
                }
			}
			)#";

			m_matWireframe = Interfaces::m_pMatSystem->FindMaterial( XorStr( "animated_wireframe_vader" ), TEXTURE_GROUP_MODEL );

			m_matAimware = Interfaces::m_pMatSystem->FindMaterial( XorStr( "glowOverlay" ), TEXTURE_GROUP_MODEL );

			m_matShiny = Interfaces::m_pMatSystem->FindMaterial( XorStr( "vader_custom" ), TEXTURE_GROUP_MODEL );

			m_matAnimatedLine = Interfaces::m_pMatSystem->FindMaterial(XorStr("LineOverlay"), TEXTURE_GROUP_MODEL);

			if( !m_matRegular || m_matRegular == nullptr || m_matRegular->IsErrorMaterial( ) )
				return false;

			if( !m_matFlat || m_matFlat == nullptr || m_matFlat->IsErrorMaterial( ) )
				return false;

			if( !m_matGlow || m_matGlow == nullptr || m_matGlow->IsErrorMaterial( ) )
				return false;

			if( !m_matShiny || m_matShiny == nullptr || m_matShiny->IsErrorMaterial( ) )
				return false;

			if ( !m_matWireframe || m_matWireframe == nullptr || m_matWireframe->IsErrorMaterial( ) )
				return false;

			if ( !m_matAimware || m_matAimware == nullptr || m_matAimware->IsErrorMaterial( ) )
				return false;

			if (!m_matAnimatedLine || m_matAnimatedLine == nullptr || m_matAnimatedLine->IsErrorMaterial())
				return false;

			m_bInit = true;
			return true;
		}

	private:
		void OverrideMaterial( bool ignoreZ, int type, const FloatColor& rgba, float glow_mod = 0, bool wf = false, const FloatColor& pearlescence_clr = { }, float pearlescence = 1.f, float shine = 1.f );

		bool m_bInit = false;
		IMaterial* m_matFlat = nullptr;
		IMaterial* m_matRegular = nullptr;
		IMaterial* m_matGlow = nullptr;
		IMaterial* m_matShiny = nullptr;
		IMaterial* m_matWireframe = nullptr;
		IMaterial* m_matAimware = nullptr;
		IMaterial* m_matAnimatedLine = nullptr;

		std::vector<C_HitMatrixEntry> m_Hitmatrix;
	};

	Encrypted_t<IChams> IChams::Get() {
		static CChams instance;
		return &instance;
	}


	CChams::CChams( ) {

	}

	CChams::~CChams( ) {

	}

	void CChams::OnPostScreenEffects() {
		auto pLocal = C_CSPlayer::GetLocalPlayer();

		if (!g_Vars.globals.HackIsReady || !Interfaces::m_pEngine->IsConnected() || !Interfaces::m_pEngine->IsInGame() || !pLocal) {
			m_Hitmatrix.clear();
			return;
		}

		if (m_Hitmatrix.empty())
			return;

		if (!Interfaces::m_pStudioRender.IsValid())
			return;

		auto ctx = Interfaces::m_pMatSystem->GetRenderContext();

		if (!ctx)
			return;

		auto DrawModelRebuild = [&](C_HitMatrixEntry it) -> void {
			if (!g_Vars.r_drawmodelstatsoverlay)
				return;

			DrawModelResults_t results;
			DrawModelInfo_t info;
			ColorMeshInfo_t* pColorMeshes = NULL;
			info.m_bStaticLighting = false;
			info.m_pStudioHdr = it.state.m_pStudioHdr;
			info.m_pHardwareData = it.state.m_pStudioHWData;
			info.m_Skin = it.info.skin;
			info.m_Body = it.info.body;
			info.m_HitboxSet = it.info.hitboxset;
			info.m_pClientEntity = (IClientRenderable*)it.state.m_pRenderable;
			info.m_Lod = it.state.m_lod;
			info.m_pColorMeshes = pColorMeshes;

			bool bShadowDepth = (it.info.flags & STUDIO_SHADOWDEPTHTEXTURE) != 0;

			// Don't do decals if shadow depth mapping...
			info.m_Decals = bShadowDepth ? STUDIORENDER_DECAL_INVALID : it.state.m_decals;

			// Sets up flexes
			float* pFlexWeights = NULL;
			float* pFlexDelayedWeights = NULL;

			int overlayVal = g_Vars.r_drawmodelstatsoverlay->GetInt();
			int drawFlags = it.state.m_drawFlags;

			if (bShadowDepth) {
				drawFlags |= STUDIORENDER_DRAW_OPAQUE_ONLY;
				drawFlags |= STUDIORENDER_SHADOWDEPTHTEXTURE;
			}

			if (overlayVal && !bShadowDepth) {
				drawFlags |= STUDIORENDER_DRAW_GET_PERF_STATS;
			}

			//Interfaces::m_pStudioRender->DrawModel(&results, &info, it.pBoneToWorld, pFlexWeights, pFlexDelayedWeights, it.info.origin, drawFlags);
			Hooked::oDrawModelExecute(Interfaces::m_pModelRender.Xor(), Memory::VCall< IMatRenderContext* (__thiscall*)(void*) >(Interfaces::m_pMatSystem.Xor(), 115)(Interfaces::m_pMatSystem.Xor()), it.state, it.info, it.pBoneToWorld);
			Interfaces::m_pStudioRender->m_pForcedMaterial = nullptr;
			Interfaces::m_pStudioRender->m_nForcedMaterialType = 0;
		};

		for (int i = 0; i < m_Hitmatrix.size(); i++)
		{
			if (Interfaces::m_pGlobalVars->realtime - m_Hitmatrix[i].time < 0.0f && Interfaces::m_pEntList->GetClientEntity(m_Hitmatrix[i].info.entity_index))
				continue;

			m_Hitmatrix.erase(m_Hitmatrix.begin() + i);
		}

		for (auto it = m_Hitmatrix.begin(); it != m_Hitmatrix.end(); it++) 
		{
			if (!(&it->state) || !it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
				!it->info.pRenderable || !it->info.pModelToWorld || !it->info.pModel) {
				++it;
				continue;
			}

			float_t flProgress = (it->time - Interfaces::m_pGlobalVars->realtime) / 0.4f;
			if (it->time - Interfaces::m_pGlobalVars->realtime > 0.4f)
				flProgress = 1.0f;

			//auto alpha = 1.0f;
			//auto delta = Interfaces::m_pGlobalVars->realtime - it->time;
			//if (delta > 0.0f) {
			//	alpha -= delta;
			//	if (delta > 1.0f) {
			//		it = m_Hitmatrix.erase(it);
			//		continue;
			//	}
			//}

			auto color = g_Vars.esp.hitmatrix_color;
			color.a *= flProgress;

			int material; // la premium

			switch (g_Vars.esp.new_chams_onshot_mat) {
			case 0:
			{
				material = 4;
				break;
			}
			case 1:
			{
				material = 2;
				break;
			}

			}

			OverrideMaterial(true, material, color, std::clamp<float>((100.0f - g_Vars.esp.new_chams_onshot_mat_glow_value) * 0.2f, 1.f, 20.f));

			Interfaces::m_pRenderView->SetBlend(color.a);

			DrawModelRebuild(*it);

			Interfaces::m_pModelRender->ForcedMaterialOverride(nullptr);
		}
	}

	void CChams::AddHitmatrix(C_CSPlayer* player, matrix3x4_t* bones) {
		if (!player || !bones)
			return;

		static int m_nSkin = SDK::Memory::FindInDataMap(player->GetPredDescMap(), XorStr("m_nSkin"));
		static int m_nBody = SDK::Memory::FindInDataMap(player->GetPredDescMap(), XorStr("m_nBody"));

		auto renderable = player->GetClientRenderable();

		if (!renderable)
			return;

		auto model = player->GetModel();

		if (!model)
			return;

		auto hdr = *(studiohdr_t**)(player->m_pStudioHdr());

		if (!hdr)
			return;

		auto& hit = m_Hitmatrix.emplace_back();

		std::memcpy(hit.pBoneToWorld, bones, player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
		hit.time = Interfaces::m_pGlobalVars->realtime + g_Vars.esp.hitmatrix_time;

		hit.info.origin = player->GetAbsOrigin();
		hit.info.angles = player->GetAbsAngles();

		hit.state.m_pStudioHdr = hdr;
		hit.state.m_pStudioHWData = Interfaces::m_pMDLCache->GetHardwareData(model->studio);
		hit.state.m_pRenderable = renderable;
		hit.state.m_drawFlags = 0;

		hit.info.pRenderable = renderable;
		hit.info.pModel = model;
		hit.info.pLightingOffset = nullptr;
		hit.info.pLightingOrigin = nullptr;
		hit.info.hitboxset = player->m_nHitboxSet();
		hit.info.skin = *(int*)(uintptr_t(player) + m_nSkin);
		hit.info.body = *(int*)(uintptr_t(player) + m_nBody);
		hit.info.entity_index = player->m_entIndex;
		hit.info.instance = Memory::VCall<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30u)(renderable);
		hit.info.flags = 0x1;

		hit.info.pModelToWorld = &hit.model_to_world;
		hit.state.m_pModelToWorld = &hit.model_to_world;

		hit.model_to_world.AngleMatrix(hit.info.angles, hit.info.origin);
	}


	void CChams::OverrideMaterial( bool ignoreZ, int type, const FloatColor& rgba, float glow_mod, bool wf, const FloatColor& pearlescence_clr, float pearlescence, float shine ) {
		IMaterial* material = nullptr;

		switch( type ) {
		case MATERIAL_OFF: break;
		case MATERIAL_FLAT:
			material = m_matRegular; break;
		case MATERIAL_REGULAR:
			material = m_matFlat; break;
		case MATERIAL_SHINY:
			material = m_matShiny; break;
		case MATERIAL_GLOW:
			material = m_matAimware; break;
		case MATERIAL_WIREFRAME:
			material = m_matWireframe; break;
		case MATERIAL_OUTLINE:
			material = m_matGlow; break;
		case MATERIAL_ANIMATEDLINE:
			material = m_matAnimatedLine; break;
		}

		if( !material ) {
			Interfaces::m_pStudioRender->m_pForcedMaterial = nullptr;
			Interfaces::m_pStudioRender->m_nForcedMaterialType = 0;
			return;
		}

		// apparently we have to do this, otherwise SetMaterialVarFlag can cause crashes (I crashed once here when loading a different cfg)
		material->IncrementReferenceCount( );

		material->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, ignoreZ );
		material->SetMaterialVarFlag( MATERIAL_VAR_WIREFRAME, wf );

		if( type == MATERIAL_GLOW ) {
			auto tint = material->FindVar( XorStr( "$envmaptint" ), nullptr );
			if( tint )
				tint->SetVecValue( rgba.r,
					rgba.g,
					rgba.b );

			auto alpha = material->FindVar( XorStr( "$alpha" ), nullptr );
			if( alpha )
				alpha->SetFloatValue( rgba.a );

			auto envmap = material->FindVar( XorStr( "$envmapfresnelminmaxexp" ), nullptr );
			if( envmap )
				envmap->SetVecValue( 0.f, 1.f, glow_mod );
		}
		else if( type == MATERIAL_OUTLINE ) {
			auto tint = material->FindVar( XorStr( "$envmaptint" ), nullptr );
			if( tint )
				tint->SetVecValue( rgba.r,
					rgba.g,
					rgba.b );

			auto alpha = material->FindVar( XorStr( "$alpha" ), nullptr );
			if( alpha )
				alpha->SetFloatValue( rgba.a );

			auto envmap = material->FindVar( XorStr( "$envmapfresnelminmaxexp" ), nullptr );
			if( envmap )
				envmap->SetVecValue( 0.f, 1.f, 8.0f );
		}
		else if( type == MATERIAL_SHINY ) {
			material->AlphaModulate( rgba.a );
			material->ColorModulate( rgba.r, rgba.g, rgba.b );
			
			auto tint = material->FindVar( XorStr( "$envmaptint" ), nullptr );
			if( tint )
				tint->SetVecValue( pearlescence_clr.r * ( pearlescence / 100.f ),
					pearlescence_clr.g * ( pearlescence / 100.f ),
					pearlescence_clr.b * ( pearlescence / 100.f ) );

			auto envmap = material->FindVar( XorStr( "$phongboost" ), nullptr );
			if( envmap )
				envmap->SetVecValue( shine / 100.f, shine / 100.f, shine / 100.f );
		}
		else {
			material->AlphaModulate(
				rgba.a );

			material->ColorModulate(
				rgba.r,
				rgba.g,
				rgba.b );
		}

		Interfaces::m_pStudioRender->m_pForcedMaterial = material;
		Interfaces::m_pStudioRender->m_nForcedMaterialType = 0;
	}

	void CChams::DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) {
		if( !Interfaces::m_pStudioRender.IsValid( ) )
			goto end;

		if( !CreateMaterials( ) )
			goto end;

		//if( !g_Vars.esp.chams_enabled )
		//	goto end_func;

		C_CSPlayer* local = C_CSPlayer::GetLocalPlayer( );
		if( !local || !Interfaces::m_pEngine->IsInGame( ) )
			goto end;

		static float pulse_alpha = 0.f;
		static bool change_alpha = false;

		if( pulse_alpha <= 0.f )
			change_alpha = true;
		else if( pulse_alpha >= 255.f )
			change_alpha = false;

		pulse_alpha = change_alpha ? pulse_alpha + 0.05f : pulse_alpha - 0.05f;

		// STUDIO_SHADOWDEPTHTEXTURE(used for shadows)
		if( RenderInfo.flags & 0x40000000
			|| !RenderInfo.pRenderable
			|| !RenderInfo.pRenderable->GetIClientUnknown( ) ) {
			goto end;
		}

		// already have forced material ( glow outline ) 
		if( g_Vars.globals.m_bInPostScreenEffects
			|| !pBoneToWorld
			|| !RenderInfo.pModel
			//|| RenderInfo.flags == 0x40100001 
			) {
			goto end;
		}

		auto entity = ( C_CSPlayer* )( RenderInfo.pRenderable->GetIClientUnknown( )->GetBaseEntity( ) );
		if( !entity )
			goto end;

		auto client_class = entity->GetClientClass( );
		if( !client_class )
			goto end;

		if( !client_class->m_ClassID )
			goto end;

		if( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
			if( g_Vars.esp.remove_sleeves && strstr( DrawModelState.m_pStudioHdr->szName, XorStr( "sleeve" ) ) != nullptr )
				return;
		}

		auto InvalidateMaterial = [ & ] ( ) -> void {
			Interfaces::m_pStudioRender->m_pForcedMaterial = nullptr;
			Interfaces::m_pStudioRender->m_nForcedMaterialType = 0;
		};

		if( g_Vars.esp.chams_attachments ) {
			if( client_class->m_ClassID == ClassId_t::CBaseWeaponWorldModel || client_class->m_ClassID == ClassId_t::CBreakableProp ) {
				if( !entity )
					goto end;

				// ???
				if( entity->m_nModelIndex( ) >= 67000000 )
					goto end;

				if( client_class->m_ClassID == ClassId_t::CBreakableProp ) {
					if( !entity->moveparent( ).IsValid( ) )
						goto end;
				}

				if( client_class->m_ClassID == ClassId_t::CBaseWeaponWorldModel ) {
					if( !entity->m_hCombatWeaponParent( ).IsValid( ) )
						goto end;
				}

				C_BaseEntity* owner = nullptr;
				if( client_class->m_ClassID == ClassId_t::CBaseWeaponWorldModel )
					owner = ( C_CSPlayer* )( ( ( C_CSPlayer* )entity->m_hCombatWeaponParent( ).Get( ) )->m_hOwnerEntity( ).Get( ) );
				else
					owner = ( C_CSPlayer* )entity->moveparent( ).Get( );

				if( owner ) {
					if( g_Vars.esp.chams_attachments ) {
						if( owner->EntIndex( ) != local->EntIndex( ) )
							goto end;
					}
				}

				if (g_Vars.esp.new_chams_attachments_original_model)
					Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

				//set attachments chams
				OverrideMaterial(false, g_Vars.esp.attachments_chams_mat, g_Vars.esp.attachments_chams_color, 0.f, false,
					g_Vars.esp.chams_attachments_pearlescence_color, g_Vars.esp.chams_attachments_pearlescence, g_Vars.esp.chams_attachments_shine);

				Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);


				if (g_Vars.esp.new_chams_attachments_overlay > 0) {

					int material; // la premium

					switch (g_Vars.esp.new_chams_attachments_overlay) {
					case 0:
					{
						material = 0;
						break;
					}
					case 1:
					{
						material = 4;
						break;
					}
					case 2:
					{
						material = 6;
						break;
					}

					}

					OverrideMaterial(false, material, g_Vars.esp.new_chams_attachments_overlay_color,
						std::clamp<float>((100.0f - g_Vars.esp.chams_attachments_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_attachments_outline_wireframe);

					Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
				}

				InvalidateMaterial( );
				return;
			}
		}

		if( client_class->m_ClassID == ClassId_t::CPredictedViewModel ) {
			if( !g_Vars.esp.chams_weapon )
				goto end;

			if (g_Vars.esp.new_chams_weapon_original_model)
				Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);


			//set weapon chams
			OverrideMaterial(false, g_Vars.esp.weapon_chams_mat, g_Vars.esp.weapon_chams_color, 0.f, false,
				g_Vars.esp.chams_weapon_pearlescence_color, g_Vars.esp.chams_weapon_pearlescence, g_Vars.esp.chams_weapon_shine);

			Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);


			if (g_Vars.esp.new_chams_weapon_overlay > 0) {

				int material; // la premium

				switch (g_Vars.esp.new_chams_weapon_overlay) {
				case 0:
				{
					material = 0;
					break;
				}
				case 1:
				{
					material = 4;
					break;
				}
				case 2:
				{
					material = 6;
					break;
				}

				}

				OverrideMaterial(false, material, g_Vars.esp.new_chams_weapon_overlay_color,
					std::clamp<float>((100.0f - g_Vars.esp.chams_weapon_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_weapon_outline_wireframe);

				Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
			}



			InvalidateMaterial( );
			return;
		}
		else if( client_class->m_ClassID == ClassId_t::CBaseAnimating ) {
			if( !g_Vars.esp.chams_hands )
				goto end;

			if (g_Vars.esp.new_chams_hands_original_model)
				Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);


			//set hands chams
			OverrideMaterial(false, g_Vars.esp.hands_chams_mat, g_Vars.esp.hands_chams_color, 0.f, false,
				g_Vars.esp.chams_hands_pearlescence_color, g_Vars.esp.chams_hands_pearlescence, g_Vars.esp.chams_hands_shine);

			Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
			

			if (g_Vars.esp.new_chams_hands_overlay > 0) {

				int material; // la premium

				switch (g_Vars.esp.new_chams_hands_overlay) {
				case 0:
				{
					material = 0;
					break;
				}
				case 1:
				{
					material = 4;
					break;
				}
				case 2:
				{
					material = 6;
					break;
				}

				}

				OverrideMaterial(false, material, g_Vars.esp.new_chams_hands_overlay_color,
					std::clamp<float>((100.0f - g_Vars.esp.chams_hands_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_hands_outline_wireframe);

				Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
			}

			InvalidateMaterial( );
			return;
		}
		else if( entity && entity->IsPlayer( ) && !entity->IsDormant( ) && !entity->IsDead( ) && entity->m_entIndex >= 1 && entity->m_entIndex <= Interfaces::m_pGlobalVars->maxClients || entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll ) {
			bool is_local_player = false, is_enemy = false, is_teammate = false;
			if( entity->EntIndex( ) == local->EntIndex( ) )
				is_local_player = true;
			else if( entity->m_iTeamNum( ) != local->m_iTeamNum( ) )
				is_enemy = true;
			else
				is_teammate = true;

			if( entity == local ) {
				if( g_Vars.esp.blur_in_scoped && Interfaces::m_pInput->CAM_IsThirdPerson( ) ) {
					if( local && !local->IsDead( ) && local->m_bIsScoped( ) ) {
						Interfaces::m_pRenderView->SetBlend( g_Vars.esp.blur_in_scoped_value / 100.f );
					}
				}
			}

			bool ragdoll = entity->GetClientClass( )->m_ClassID == ClassId_t::CCSRagdoll;

			auto vis = is_teammate ? g_Vars.esp.team_chams_color_vis : g_Vars.esp.enemy_chams_color_vis;
			auto xqz = is_teammate ? g_Vars.esp.team_chams_color_xqz : g_Vars.esp.enemy_chams_color_xqz;
			bool should_xqz = is_teammate ? g_Vars.esp.team_chams_xqz : g_Vars.esp.enemy_chams_xqz;
			bool should_vis = is_teammate ? g_Vars.esp.team_chams_vis : g_Vars.esp.enemy_chams_vis;
			if( is_local_player ) {
				//set local player ghost chams
				static auto g_GameRules = *( uintptr_t** )( Engine::Displacement.Data.m_GameRules );
				bool invalid = g_GameRules && *( bool* )( *( uintptr_t* )g_GameRules + 0x20 ) || ( entity->m_fFlags( ) & ( 1 << 6 ) );

				if (g_Vars.fakelag.vis_lag && local->m_vecVelocity().Length() > 0.1f) {
					OverrideMaterial(false, MATERIAL_REGULAR, g_Vars.fakelag.vis_lag_color, 0.f, false);
					Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, g_Vars.globals.LagPosition);
					InvalidateMaterial( );
				}

				if (g_Vars.esp.break_bones) {
					pBoneToWorld[0].SetOrigin(Vector(pBoneToWorld[0].GetOrigin().x, pBoneToWorld[0].GetOrigin().y, pBoneToWorld[0].GetOrigin().z - 5));
					pBoneToWorld[3].SetOrigin(Vector(pBoneToWorld[3].GetOrigin().x, pBoneToWorld[3].GetOrigin().y, pBoneToWorld[3].GetOrigin().z - 5));
					pBoneToWorld[4].SetOrigin(Vector(pBoneToWorld[4].GetOrigin().x, pBoneToWorld[4].GetOrigin().y, pBoneToWorld[4].GetOrigin().z - 5));
					pBoneToWorld[5].SetOrigin(Vector(pBoneToWorld[5].GetOrigin().x, pBoneToWorld[5].GetOrigin().y, pBoneToWorld[5].GetOrigin().z - 5));
					pBoneToWorld[6].SetOrigin(Vector(pBoneToWorld[6].GetOrigin().x, pBoneToWorld[6].GetOrigin().y, pBoneToWorld[6].GetOrigin().z - 5));
					pBoneToWorld[7].SetOrigin(Vector(pBoneToWorld[7].GetOrigin().x, pBoneToWorld[7].GetOrigin().y, pBoneToWorld[7].GetOrigin().z - 5));
					pBoneToWorld[8].SetOrigin(Vector(pBoneToWorld[8].GetOrigin().x, pBoneToWorld[8].GetOrigin().y, pBoneToWorld[8].GetOrigin().z - 5));
				}

				if (g_Vars.esp.chams_local) {

					if (g_Vars.esp.new_chams_local_original_model)
						Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

					//set local player chams
					if (g_Vars.esp.new_chams_local > 0) {

						OverrideMaterial(false, g_Vars.esp.new_chams_local, g_Vars.esp.chams_local_color, 0.f, false,
							g_Vars.esp.chams_local_pearlescence_color, g_Vars.esp.chams_local_pearlescence, g_Vars.esp.chams_local_shine);

						Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
					}

					if (g_Vars.esp.new_chams_local_overlay > 0) {

						int material; // la premium

						switch (g_Vars.esp.new_chams_local_overlay) {
						case 0:
						{
							material = 0;
							break;
						}
						case 1:
						{
							material = 4;
							break;
						}
						case 2:
						{
							material = 6;
							break;
						}

						}

						OverrideMaterial(false, material, g_Vars.esp.new_chams_local_overlay_color,
							std::clamp<float>((100.0f - g_Vars.esp.chams_local_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_local_outline_wireframe);

						Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
					}
				}
				else {
					OverrideMaterial(false, 0, FloatColor(1, 1, 1, 1));
					Interfaces::m_pRenderView->SetColorModulation(FloatColor(255, 255, 255, 255));
					Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);
				}

				InvalidateMaterial( );
				return;
			}
			else if( is_enemy ) {
				auto data = Engine::LagCompensation::Get( )->GetLagData( entity->m_entIndex );
				if( data.IsValid( ) ) {
					if( g_Vars.esp.chams_history ) {
						// start from begin
						matrix3x4_t out[ 128 ];
						if( CChams::GetBacktrackMatrix( entity, out ) ) {

							int material; // la premium

							switch (g_Vars.esp.chams_history_material) {
							case 0:
							{
								material = MATERIAL_REGULAR;
								break;
							}
							case 1:
							{
								material = MATERIAL_FLAT;
								break;
							}
							case 2:
							{
								material = MATERIAL_GLOW;
								break;
							}

							}

							OverrideMaterial( true, material, g_Vars.esp.chams_history_color, std::clamp<float>((100.0f - g_Vars.esp.chams_history_outline_value) * 0.2f, 1.f, 20.f));
							Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, out );

							InvalidateMaterial( );
						}
					}
				}
				//set enemy chams
				if( g_Vars.esp.chams_enemy ) {
					if( !ragdoll ) {

						if( should_xqz ) {

							//set enemy player chams
							if (g_Vars.esp.new_chams_enemy_xqz > 0) {

								OverrideMaterial(true, g_Vars.esp.new_chams_enemy_xqz, g_Vars.esp.new_chams_enemy_xqz_color, 0.f, false,
									g_Vars.esp.chams_enemy_xqz_pearlescence_color, g_Vars.esp.chams_enemy_xqz_pearlescence, g_Vars.esp.chams_enemy_xqz_shine);

								Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

							}

							if (g_Vars.esp.new_chams_enemy_xqz_overlay > 0) {

								int material; // la premium

								switch (g_Vars.esp.new_chams_enemy_xqz_overlay) {
								case 0:
								{
									material = 0;
									break;
								}
								case 1:
								{
									material = 4;
									break;
								}

								}

								OverrideMaterial(true, material, g_Vars.esp.new_chams_enemy_xqz_overlay_color,
									std::clamp<float>((100.0f - g_Vars.esp.chams_enemy_xqz_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_enemy_outline_xqz_wireframe);

								Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

							}

							InvalidateMaterial( );
						}

						if( should_vis ) {

							//set enemy player chams
							if (g_Vars.esp.new_chams_enemy > 0) {

								OverrideMaterial(false, g_Vars.esp.new_chams_enemy, g_Vars.esp.new_chams_enemy_color, 0.f, false,
									g_Vars.esp.chams_enemy_pearlescence_color, g_Vars.esp.chams_enemy_pearlescence, g_Vars.esp.chams_enemy_shine);

								Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

							}

							if (g_Vars.esp.new_chams_enemy_overlay > 0) {

								int material; // la premium

								switch (g_Vars.esp.new_chams_enemy_overlay) {
								case 0:
								{
									material = 0;
									break;
								}
								case 1:
								{
									material = 4;
									break;
								}

								}

								OverrideMaterial(false, material, g_Vars.esp.new_chams_enemy_overlay_color,
									std::clamp<float>((100.0f - g_Vars.esp.chams_enemy_outline_value) * 0.2f, 1.f, 20.f), g_Vars.esp.chams_enemy_outline_wireframe);

								Hooked::oDrawModelExecute(ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld);

							}

							InvalidateMaterial( );
						}

					}
				}
				else {
					if( !ragdoll ) {
						InvalidateMaterial( );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					}
				}

				InvalidateMaterial( );
				return;
			}
			else if( is_teammate ) {
				//set teammate chams
				/*if( g_Vars.esp.chams_death_teammate ) {
					if( ragdoll ) {
						int material = g_Vars.esp.team_chams_death_mat;
						if( g_Vars.esp.chams_teammate_death_pulse ) {
							if( should_xqz_death ) {
								FloatColor clr = xqz_death;
								clr.a = pulse_alpha / 255.f;
								OverrideMaterial( true, material, clr );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}

							if( should_vis_death ) {
								FloatColor clr = vis_death;
								clr.a = pulse_alpha / 255.f;
								OverrideMaterial( false, material, clr );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}
						}
						else {
							if( should_xqz_death ) {
								OverrideMaterial( true, material, xqz_death );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}

							if( should_vis_death ) {
								OverrideMaterial( false, material, vis_death );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}
						}

						if( g_Vars.esp.chams_teammate_death_outline ) {
							OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_teammate_death_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_teammate_death_outline_value ) * 0.2f, 1.f, 20.f ) );
							Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
						}
					}
				}
				else {
					if( ragdoll ) {
						InvalidateMaterial( );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					}
				}*/

				/*//set teammate chams
				if( g_Vars.esp.chams_teammate ) {
					if( !ragdoll ) {
						int material = g_Vars.esp.team_chams_mat;
						if( g_Vars.esp.chams_teammate_pulse ) {
							if( should_xqz ) {
								FloatColor clr = xqz;
								clr.a = pulse_alpha;
								OverrideMaterial( true, material, clr );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}

							if( should_vis ) {
								FloatColor clr = vis;
								clr.a = pulse_alpha;
								OverrideMaterial( false, material, clr );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}
						}
						else {
							if( should_xqz ) {
								OverrideMaterial( true, material, xqz );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}

							if( should_vis ) {
								OverrideMaterial( false, material, vis );
								Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
							}
						}

						if( g_Vars.esp.chams_teammate_outline ) {
							OverrideMaterial( true, MATERIAL_GLOW, g_Vars.esp.chams_teammate_outline_color, std::clamp<float>( ( 100.0f - g_Vars.esp.chams_teammate_outline_value ) * 0.2f, 1.f, 20.f ) );
							Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
						}
					}
				}
				else {
					if( !ragdoll ) {
						InvalidateMaterial( );
						Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
					}
				}*/

				if( !ragdoll ) {
					InvalidateMaterial( );
					Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
				}
				return;
			}
		}
	end:
		local = C_CSPlayer::GetLocalPlayer( );

		if( !Interfaces::m_pEngine->IsInGame( ) || !Interfaces::m_pEngine->IsConnected( ) || !local )
			return;
		else {
			Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
			return;
		}

	end_func:
		Hooked::oDrawModelExecute( ECX, MatRenderContext, DrawModelState, RenderInfo, pBoneToWorld );
		return;

	}

	void CChams::OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pBoneToWorld ) {

	}

	bool CChams::GetBacktrackMatrix( C_CSPlayer* entity, matrix3x4_t* out ) {
		if( !entity )
			return false;

		auto data = Engine::LagCompensation::Get( )->GetLagData( entity->m_entIndex );
		if( data.IsValid( ) ) {
			// start from begin
			for( auto it = data->m_History.begin( ); it != data->m_History.end( ); ++it ) {
				if( it->player != entity )
					break;

				std::pair< Engine::C_LagRecord*, Engine::C_LagRecord* > last;
				if( Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *it, 0.2f, -1, false ) && it + 1 != data->m_History.end( ) && !Engine::LagCompensation::Get( )->IsRecordOutOfBounds( *( it + 1 ), 0.2f, -1, false ) )
					last = std::make_pair( &*( it + 1 ), &*it );

				if( !last.first || !last.second )
					continue;

				if( !Interfaces::m_pPrediction->GetUnpredictedGlobals( ) )
					continue;

				const auto& FirstInvalid = last.first;
				const auto& LastInvalid = last.second;

				if( !LastInvalid || !FirstInvalid )
					continue;

				if( LastInvalid->m_flSimulationTime - FirstInvalid->m_flSimulationTime > 0.5f )
					continue;

				if( ( FirstInvalid->m_vecOrigin - entity->m_vecOrigin( ) ).Length( ) < 2.5f && FirstInvalid->m_iResolverMode != Engine::RModes::FLICK )
					return false;

				const auto NextOrigin = LastInvalid->m_vecOrigin;
				const auto curtime = Interfaces::m_pPrediction->GetUnpredictedGlobals( )->curtime;

				auto flDelta = 1.f - ( curtime - LastInvalid->m_flInterpolateTime ) / ( LastInvalid->m_flSimulationTime - FirstInvalid->m_flSimulationTime );
				if( flDelta < 0.f || flDelta > 1.f )
					LastInvalid->m_flInterpolateTime = curtime;

				flDelta = 1.f - ( curtime - LastInvalid->m_flInterpolateTime ) / ( LastInvalid->m_flSimulationTime - FirstInvalid->m_flSimulationTime );

				const auto lerp = Math::Interpolate( NextOrigin, FirstInvalid->m_vecOrigin, std::clamp( flDelta, 0.f, 1.f ) );

				matrix3x4_t ret[ 128 ];
				memcpy( ret, FirstInvalid->m_BoneMatrix, sizeof( matrix3x4_t[ 128 ] ) );

				for( size_t i{ }; i < 128; ++i ) {
					const auto matrix_delta = Math::MatrixGetOrigin( FirstInvalid->m_BoneMatrix[ i ] ) - FirstInvalid->m_vecOrigin;
					Math::MatrixSetOrigin( matrix_delta + lerp, ret[ i ] );
				}

				memcpy( out, ret, sizeof( matrix3x4_t[ 128 ] ) );
				return true;
			}
		}
	}
}

#pragma optimize( "", on )