#pragma once

#include <unordered_map>

#include "Pigeon/Audio/AudioDevice.h"
#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Core/UUID.h"

namespace pg
{
	// No-op audio device for the Testing build: tracks voice bookkeeping in memory (category, base and
	// effective volume, paused flag) so tests can assert playback state without real audio hardware.
	class TestingAudioDevice : public AudioDevice
	{
	public:
		TestingAudioDevice() = default;
		~TestingAudioDevice() override = default;

		void Play(const pg::UUID& voiceId, const pg::S_Ptr<pg::SoundClip>& clip, pg::EAudioCategory category, bool loop, float baseVolume) override;
		void Pause(const pg::UUID& voiceId) override;
		void Resume(const pg::UUID& voiceId) override;
		void Stop(const pg::UUID& voiceId) override;
		void SetCategoryVolumes(float master, float sound, float music) override;
		void Update() override;

		bool IsVoiceActive(const pg::UUID& voiceId) const override;
		bool IsVoicePaused(const pg::UUID& voiceId) const override;
		float GetVoiceVolume(const pg::UUID& voiceId) const override;
		std::size_t GetActiveVoiceCount() const override;

	private:
		struct Voice
		{
			pg::EAudioCategory m_Category = pg::EAudioCategory::eSound;
			float m_BaseVolume = 1.0f;
			float m_EffectiveVolume = 1.0f;
			bool m_Paused = false;
		};

		std::unordered_map<pg::UUID, Voice> m_Voices;
		float m_Master = 1.0f;
		float m_Sound = 1.0f;
		float m_Music = 1.0f;
	};
}
