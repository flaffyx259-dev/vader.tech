#pragma once
#include "../../SDK/sdk.hpp"

struct angle_data {
	float angle;
	float thickness;
	angle_data(const float angle, const float thickness) : angle(angle), thickness(thickness) {}
};

class freestanding
{
public:
	bool override(float& yaw) const;
	bool get_real(float& yaw);
	void get_targets();
	void update_hotkeys(ClientFrameStage_t stage);
	static float trace_freestanding(C_CSPlayer* player, const Vector point);
	int get_mode() const { return direction; }
	float last_fs_time = 0.f;
private:
	int direction = -1;
	std::vector<C_CSPlayer*> players;
};

class AdaptiveAngle {
public:
	float m_yaw;
	float m_dist;

public:
	// ctor.
	__forceinline AdaptiveAngle(float yaw, float penalty = 0.f) {
		// set yaw.
		m_yaw = Math::NormalizedAngle(yaw);

		// init distance.
		m_dist = 0.f;

		// remove penalty.
		m_dist -= penalty;
	}
};

namespace Interfaces
{

   class __declspec( novtable ) AntiAimbot : public NonCopyable {
   public:
	  static Encrypted_t<AntiAimbot> Get( );
	  virtual void Main( bool* bSendPacket, bool* bFinalPacket, Encrypted_t<CUserCmd> cmd, bool ragebot ) = 0;
	  virtual void PrePrediction( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) = 0;
	  virtual void ImposterBreaker( bool* bSendPacket, Encrypted_t<CUserCmd> cmd ) = 0;
	  virtual void fake_duck(bool* bSendPacket, Encrypted_t<CUserCmd> cmd) = 0;
   protected:
	  AntiAimbot( ) { };
	  virtual ~AntiAimbot( ) { };
   };
}
