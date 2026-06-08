#pragma once

#include <cstddef>

#include "Pigeon/Audio/EAudioCategory.h"
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"

namespace pg
{
	class SoundClip;

	// Platform-abstracted mixer/output device. Owns the active voices, each addressed by a
	// caller-assigned UUID handle. The concrete backend (miniaudio or the Testing no-op) is selected by
	// Create() using the same renderer-API switch as the other platform resources.
	class AudioDevice
	{
	public:
		virtual ~AudioDevice() = default;

		virtual void Play(const pg::UUID& voiceId, const pg::S_Ptr<pg::SoundClip>& clip, pg::EAudioCategory category, bool loop, float baseVolume) = 0;
		virtual void Pause(const pg::UUID& voiceId) = 0;
		virtual void Resume(const pg::UUID& voiceId) = 0;
		virtual void Stop(const pg::UUID& voiceId) = 0;

		// Recompute and apply the effective volume of every active voice from the current volumes.
		virtual void SetCategoryVolumes(float master, float sound, float music) = 0;

		// Reap finished non-looping voices so they stop counting as active.
		virtual void Update() = 0;

		virtual bool IsVoiceActive(const pg::UUID& voiceId) const = 0;
		virtual bool IsVoicePaused(const pg::UUID& voiceId) const = 0;
		virtual float GetVoiceVolume(const pg::UUID& voiceId) const = 0;
		virtual std::size_t GetActiveVoiceCount() const = 0;

		static pg::S_Ptr<AudioDevice> Create();
	};
}
