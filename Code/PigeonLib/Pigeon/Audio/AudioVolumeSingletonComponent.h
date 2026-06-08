#pragma once

namespace pg
{
	struct AudioVolumeSingletonComponent
	{
		AudioVolumeSingletonComponent() = default;
		AudioVolumeSingletonComponent(const AudioVolumeSingletonComponent&) = default;

		float m_MasterVolume = 1.0f;
		float m_SoundVolume = 1.0f;
		float m_MusicVolume = 1.0f;
	};
}
