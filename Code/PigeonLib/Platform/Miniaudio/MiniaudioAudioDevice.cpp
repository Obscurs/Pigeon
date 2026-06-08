#include "pch.h"

#include "Platform/Miniaudio/MiniaudioAudioDevice.h"

#include <unordered_map>

#include "Pigeon/Audio/AudioVolume.h"
#include "Pigeon/Audio/SoundClip.h"

#include "vendor/miniaudio/miniaudio.h"

struct pg::MiniaudioAudioDevice::Impl
{
	struct Voice
	{
		ma_sound* m_Sound = nullptr;
		pg::EAudioCategory m_Category = pg::EAudioCategory::eSound;
		float m_BaseVolume = 1.0f;
		bool m_Paused = false;
	};

	ma_engine m_Engine{};
	bool m_EngineReady = false;
	std::unordered_map<pg::UUID, Voice> m_Voices;
	float m_Master = 1.0f;
	float m_Sound = 1.0f;
	float m_Music = 1.0f;

	void DestroyVoice(Voice& voice)
	{
		if (voice.m_Sound != nullptr)
		{
			ma_sound_uninit(voice.m_Sound);
			delete voice.m_Sound;
			voice.m_Sound = nullptr;
		}
	}
};

pg::MiniaudioAudioDevice::MiniaudioAudioDevice()
	: m_Impl(std::make_unique<Impl>())
{
	const ma_result result = ma_engine_init(nullptr, &m_Impl->m_Engine);
	m_Impl->m_EngineReady = (result == MA_SUCCESS);
	PG_CORE_ASSERT(m_Impl->m_EngineReady, "Failed to initialise miniaudio engine");
}

pg::MiniaudioAudioDevice::~MiniaudioAudioDevice()
{
	for (std::pair<const pg::UUID, Impl::Voice>& entry : m_Impl->m_Voices)
	{
		m_Impl->DestroyVoice(entry.second);
	}
	m_Impl->m_Voices.clear();

	if (m_Impl->m_EngineReady)
	{
		ma_engine_uninit(&m_Impl->m_Engine);
		m_Impl->m_EngineReady = false;
	}
}

void pg::MiniaudioAudioDevice::Play(const pg::UUID& voiceId, const pg::S_Ptr<pg::SoundClip>& clip, pg::EAudioCategory category, bool loop, float baseVolume)
{
	if (!m_Impl->m_EngineReady || clip == nullptr)
	{
		return;
	}

	// Replacing an existing voice under the same handle: tear the old one down first.
	const std::unordered_map<pg::UUID, Impl::Voice>::iterator existing = m_Impl->m_Voices.find(voiceId);
	if (existing != m_Impl->m_Voices.end())
	{
		m_Impl->DestroyVoice(existing->second);
		m_Impl->m_Voices.erase(existing);
	}

	ma_sound* sound = new ma_sound();
	const ma_result result = ma_sound_init_from_file(&m_Impl->m_Engine, clip->GetPath().c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr, sound);
	if (result != MA_SUCCESS)
	{
		delete sound;
		PG_CORE_ASSERT(false, "Failed to load sound from file");
		return;
	}

	ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
	ma_sound_set_volume(sound, pg::ComputeVoiceVolume(category, m_Impl->m_Master, m_Impl->m_Sound, m_Impl->m_Music, baseVolume));
	ma_sound_start(sound);

	Impl::Voice voice;
	voice.m_Sound = sound;
	voice.m_Category = category;
	voice.m_BaseVolume = baseVolume;
	voice.m_Paused = false;
	m_Impl->m_Voices[voiceId] = voice;
}

void pg::MiniaudioAudioDevice::Pause(const pg::UUID& voiceId)
{
	const std::unordered_map<pg::UUID, Impl::Voice>::iterator it = m_Impl->m_Voices.find(voiceId);
	if (it != m_Impl->m_Voices.end() && it->second.m_Sound != nullptr)
	{
		ma_sound_stop(it->second.m_Sound);
		it->second.m_Paused = true;
	}
}

void pg::MiniaudioAudioDevice::Resume(const pg::UUID& voiceId)
{
	const std::unordered_map<pg::UUID, Impl::Voice>::iterator it = m_Impl->m_Voices.find(voiceId);
	if (it != m_Impl->m_Voices.end() && it->second.m_Sound != nullptr)
	{
		ma_sound_start(it->second.m_Sound);
		it->second.m_Paused = false;
	}
}

void pg::MiniaudioAudioDevice::Stop(const pg::UUID& voiceId)
{
	const std::unordered_map<pg::UUID, Impl::Voice>::iterator it = m_Impl->m_Voices.find(voiceId);
	if (it != m_Impl->m_Voices.end())
	{
		m_Impl->DestroyVoice(it->second);
		m_Impl->m_Voices.erase(it);
	}
}

void pg::MiniaudioAudioDevice::SetCategoryVolumes(float master, float sound, float music)
{
	m_Impl->m_Master = master;
	m_Impl->m_Sound = sound;
	m_Impl->m_Music = music;
	for (std::pair<const pg::UUID, Impl::Voice>& entry : m_Impl->m_Voices)
	{
		if (entry.second.m_Sound != nullptr)
		{
			ma_sound_set_volume(entry.second.m_Sound, pg::ComputeVoiceVolume(entry.second.m_Category, master, sound, music, entry.second.m_BaseVolume));
		}
	}
}

void pg::MiniaudioAudioDevice::Update()
{
	// Reap finished non-looping voices so they stop counting as active.
	for (std::unordered_map<pg::UUID, Impl::Voice>::iterator it = m_Impl->m_Voices.begin(); it != m_Impl->m_Voices.end();)
	{
		ma_sound* sound = it->second.m_Sound;
		if (sound != nullptr && !it->second.m_Paused && ma_sound_at_end(sound) == MA_TRUE)
		{
			m_Impl->DestroyVoice(it->second);
			it = m_Impl->m_Voices.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool pg::MiniaudioAudioDevice::IsVoiceActive(const pg::UUID& voiceId) const
{
	return m_Impl->m_Voices.find(voiceId) != m_Impl->m_Voices.end();
}

bool pg::MiniaudioAudioDevice::IsVoicePaused(const pg::UUID& voiceId) const
{
	const std::unordered_map<pg::UUID, Impl::Voice>::const_iterator it = m_Impl->m_Voices.find(voiceId);
	return it != m_Impl->m_Voices.end() && it->second.m_Paused;
}

float pg::MiniaudioAudioDevice::GetVoiceVolume(const pg::UUID& voiceId) const
{
	const std::unordered_map<pg::UUID, Impl::Voice>::const_iterator it = m_Impl->m_Voices.find(voiceId);
	if (it == m_Impl->m_Voices.end() || it->second.m_Sound == nullptr)
	{
		return 0.0f;
	}
	return ma_sound_get_volume(it->second.m_Sound);
}

std::size_t pg::MiniaudioAudioDevice::GetActiveVoiceCount() const
{
	return m_Impl->m_Voices.size();
}
