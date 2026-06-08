#pragma once

namespace pg
{
	struct SetAudioVolumeRequestOneFrameComponent
	{
		SetAudioVolumeRequestOneFrameComponent() = default;
		SetAudioVolumeRequestOneFrameComponent(const SetAudioVolumeRequestOneFrameComponent&) = default;

		float m_MasterVolume = 1.0f;
		float m_SoundVolume = 1.0f;
		float m_MusicVolume = 1.0f;
	};
}
