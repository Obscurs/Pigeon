#include "pch.h"

#include "Platform/Testing/TestingAudioDevice.h"

#include "Pigeon/Audio/AudioVolume.h"

void pg::TestingAudioDevice::Play(const pg::UUID& voiceId, const pg::S_Ptr<pg::SoundClip>& clip, pg::EAudioCategory category, bool loop, float baseVolume)
{
	Voice voice;
	voice.m_Category = category;
	voice.m_BaseVolume = baseVolume;
	voice.m_EffectiveVolume = pg::ComputeVoiceVolume(category, m_Master, m_Sound, m_Music, baseVolume);
	voice.m_Paused = false;
	m_Voices[voiceId] = voice;
}

void pg::TestingAudioDevice::Pause(const pg::UUID& voiceId)
{
	const std::unordered_map<pg::UUID, Voice>::iterator it = m_Voices.find(voiceId);
	if (it != m_Voices.end())
	{
		it->second.m_Paused = true;
	}
}

void pg::TestingAudioDevice::Resume(const pg::UUID& voiceId)
{
	const std::unordered_map<pg::UUID, Voice>::iterator it = m_Voices.find(voiceId);
	if (it != m_Voices.end())
	{
		it->second.m_Paused = false;
	}
}

void pg::TestingAudioDevice::Stop(const pg::UUID& voiceId)
{
	m_Voices.erase(voiceId);
}

void pg::TestingAudioDevice::SetCategoryVolumes(float master, float sound, float music)
{
	m_Master = master;
	m_Sound = sound;
	m_Music = music;
	for (std::pair<const pg::UUID, Voice>& entry : m_Voices)
	{
		entry.second.m_EffectiveVolume = pg::ComputeVoiceVolume(entry.second.m_Category, m_Master, m_Sound, m_Music, entry.second.m_BaseVolume);
	}
}

void pg::TestingAudioDevice::Update()
{
	// No real playback: voices remain active until explicitly stopped.
}

bool pg::TestingAudioDevice::IsVoiceActive(const pg::UUID& voiceId) const
{
	return m_Voices.find(voiceId) != m_Voices.end();
}

bool pg::TestingAudioDevice::IsVoicePaused(const pg::UUID& voiceId) const
{
	const std::unordered_map<pg::UUID, Voice>::const_iterator it = m_Voices.find(voiceId);
	return it != m_Voices.end() && it->second.m_Paused;
}

float pg::TestingAudioDevice::GetVoiceVolume(const pg::UUID& voiceId) const
{
	const std::unordered_map<pg::UUID, Voice>::const_iterator it = m_Voices.find(voiceId);
	return it != m_Voices.end() ? it->second.m_EffectiveVolume : 0.0f;
}

std::size_t pg::TestingAudioDevice::GetActiveVoiceCount() const
{
	return m_Voices.size();
}
