#pragma once

#include <cstdint>
#include <string>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// App-emitted request (engine-typed, so the engine TextGenSystem can read it) to generate one text
	// completion. The result is published into the TextGenResultSingletonComponent under m_TargetTextID
	// (caller-assigned, like an audio Voice Handle). Text Generation Config fields left at their
	// zero/empty sentinel fall back to the engine defaults; prompt/system prompt/seed are always honoured.
	struct GenerateTextRequestOneFrameComponent
	{
		pg::UUID m_TargetTextID;
		std::string m_Prompt;
		std::string m_SystemPrompt;
		int64_t m_Seed = -1;

		// Unset (<= 0) fields fall back to the engine Text Generation Config defaults.
		int m_MaxTokens = 0;
		float m_Temperature = 0.f;
		float m_TopP = 0.f;
	};
}
