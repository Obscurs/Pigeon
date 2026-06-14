#pragma once

#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Shared UUIDs for the text-generation demo. The model ID matches the App ResourcesManifest
	// (Assets/App/TextGeneration); the target-text ID is caller-assigned (not in the manifest):
	// TextGenDemoSystem requests generation into it and reads the result back under the same ID.
	inline const pg::UUID k_LanguageModelID("d8000000-0000-4000-8000-000000000007");
	inline const pg::UUID k_TextGenTargetID("d8000000-0000-4000-8000-0000000000fe");
}
