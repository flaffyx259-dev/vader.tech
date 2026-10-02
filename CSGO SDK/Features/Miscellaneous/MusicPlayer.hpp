#pragma once
#include "../../source.hpp"
#include <deque>
#include "../../SDK/sdk.hpp"
namespace Interfaces
{
	class MusicPlayer : public Core::Singleton<MusicPlayer> {
		//using sound_files_t = std::vector< std::string >;
	public:
		virtual void init(void);

		virtual void play(std::size_t file_idx);
		virtual void play(std::string file, float delay_time = 0.6f);

		virtual void stop(void);

		virtual void run(void);

		virtual void load_sound_files(void);

		//sound_files_t	m_sound_files;
		bool			m_playing = false;
		float time_to_reset_sound = 0.f;
	private:
		//std::string		m_sound_files_path;
		ConVar* m_voice_loopback = nullptr;
	};
}

