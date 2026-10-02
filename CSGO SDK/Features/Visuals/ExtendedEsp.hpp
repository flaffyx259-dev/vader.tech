#pragma once
#include "../../SDK/sdk.hpp"

class __declspec( novtable ) IExtendedEsp : public NonCopyable {
public:
  static Encrypted_t<IExtendedEsp> Get( );
  virtual void Start() = NULL;
  virtual void Finish() = NULL;

  struct SoundPlayer {
	  void Override(SndInfo_t& sound) {
		  m_iIndex = sound.m_nSoundSource;
		  //m_vecOrigin = *sound.m_pOrigin;
		  m_iReceiveTime = GetTickCount();
	  }

	  int m_iIndex = 0;
	  int m_iReceiveTime = 0;
	  Vector m_vecOrigin = Vector(0, 0, 0);
	  Vector m_vecLastOrigin = Vector(0, 0, 0);

	  /* Restore data */
	  int m_nFlags = 0;
	  int playerindex = 0;
	  Vector m_vecAbsOrigin = Vector(0, 0, 0);
	  bool m_bDormant = false;
  } m_cSoundPlayers[65];

protected:
  IExtendedEsp( ) {

  }

  virtual ~IExtendedEsp( ) {

  }
};