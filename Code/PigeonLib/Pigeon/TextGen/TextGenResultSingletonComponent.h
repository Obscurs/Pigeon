#pragma once

#include <string>
#include <unordered_map>

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// The live store of finished text completions, keyed by the caller-assigned target text UUID. Written
	// solely by TextGenSystem when a Text Gen Job completes; the app reads it by the target UUID to
	// display the Generated Text. Text is not a renderer resource, so it does not route through the
	// resource map's texture map the way a Generated Texture does (ADR 0009).
	struct TextGenResultSingletonComponent
	{
		std::unordered_map<pg::UUID, std::string> m_Results;
	};
}
