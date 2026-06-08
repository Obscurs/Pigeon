#pragma once

#include "Pigeon/Audio/AudioDevice.h"
#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"

namespace pg
{
	// miniaudio-backed mixer used by the real build. Owns a miniaudio engine and one miniaudio sound per
	// active voice. All miniaudio types are confined to the .cpp via the Impl PIMPL so the heavy
	// single-header library never leaks into engine code.
	class MiniaudioAudioDevice : public AudioDevice
	{
	public:
		MiniaudioAudioDevice();
		~MiniaudioAudioDevice() override;

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
		struct Impl;
		pg::U_Ptr<Impl> m_Impl;
	};
}
