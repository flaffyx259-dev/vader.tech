#pragma once
#include "../../SDK/sdk.hpp"
enum model_type_t : uint32_t {
	invalid = 0,
	player,
	weapon,
	arms,
	view_weapon
};

class BoneArray : public matrix3x4_t {
public:
	bool GetBone(Vector& out, int bone = 0) {
		if (bone < 0 || bone >= 128)
			return false;

		matrix3x4_t* bone_matrix = &this[bone];

		if (!bone_matrix)
			return false;

		out = { bone_matrix->m[0][3], bone_matrix->m[1][3], bone_matrix->m[2][3] };

		return true;
	}
};

class C_NewChams {
public:
	//struct C_HitMatrixEntry {
	//	int ent_index;
	//	ModelRenderInfo_t info;
	//	DrawModelState_t state;
	//	matrix3x4_t pBoneToWorld[128] = {};
	//	float time;
	//	matrix3x4_t model_to_world;
	//} m_hit_matrix;

	//std::vector<C_HitMatrixEntry> m_Hitmatrix;
private:
	C_CSPlayer* localPlayer;
public:

	model_type_t GetModelType(const ModelRenderInfo_t& info);
	bool IsInViewPlane(const Vector& world);

	void SetColor(FloatColor col, IMaterial* mat = nullptr);
	void SetAlpha(float alpha, IMaterial* mat = nullptr);
	void SetupMaterial(IMaterial* mat, FloatColor col, bool z_flag);

	void init();

	bool OverridePlayer(int index);
	bool GenerateLerpedMatrix(int index, BoneArray* out);
	void RenderHistoryChams(int index);
	//void add_matrix(int index, matrix3x4_t* bones);
	//void draw_hit_matrix(void* ecx, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone);
	bool DrawModel(const ModelRenderInfo_t& info);
	void SceneEnd();

	void RenderPlayer(C_CSPlayer* player);
	void OnPostScreenEffects();
	void RenderWeapon(void* ecx, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone);
	void AddHitmatrix(C_CSPlayer* player, matrix3x4_t* bones);
	bool SortPlayers();

public:

	struct C_HitMatrixEntry {
		int ent_index;
		ModelRenderInfo_t info;
		DrawModelState_t state;
		matrix3x4_t pBoneToWorld[128] = { };
		float time;
		matrix3x4_t model_to_world;
	};

	std::vector< C_CSPlayer* > m_players;
	int m_weapon_index;
	bool m_running, m_weapon_running, m_viewmodel_running;
	IMaterial* debugambientcube;
	IMaterial* debugdrawflat;
	IMaterial* materialMetall;
	IMaterial* materialMetall2;
	IMaterial* materialMetall3;
	IMaterial* materialMetallnZ;
	IMaterial* skeet;
	IMaterial* onetap;
	IMaterial* shaded;
	IMaterial* aimware;
	IMaterial* shotchams;
	IMaterial* blinking;
	IMaterial* debugglow;
	IMaterial* debugbubbly;
	IMaterial* crystalblue;
	IMaterial* italy;
	std::vector<C_HitMatrixEntry> m_Hitmatrix;
};
namespace Interfaces
{

   class __declspec( novtable ) IChams : public NonCopyable {
   public:
	  static Encrypted_t<IChams> Get( );
	  virtual void OnDrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) = 0;
	  virtual void DrawModel( void* ECX, IMatRenderContext* MatRenderContext, DrawModelState_t& DrawModelState, ModelRenderInfo_t& RenderInfo, matrix3x4_t* pCustomBoneToWorld ) = 0;
	  virtual bool CreateMaterials( ) = 0;
	  virtual void OnPostScreenEffects() = 0;
	  virtual void AddHitmatrix(C_CSPlayer* player, matrix3x4_t* bones) = 0;

   protected:
	  IChams( ) { };
	  virtual ~IChams( ) { };
   };
}

inline C_NewChams g_NewChams;