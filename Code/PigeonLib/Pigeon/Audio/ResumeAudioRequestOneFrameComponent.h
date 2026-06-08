#pragma once

#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct ResumeAudioRequestOneFrameComponent
	{
		ResumeAudioRequestOneFrameComponent() = default;
		ResumeAudioRequestOneFrameComponent(const ResumeAudioRequestOneFrameComponent&) = default;

		pg::UUID m_VoiceID;
	};
}
