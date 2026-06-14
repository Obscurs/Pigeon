#pragma once

#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Shared UUIDs for the text-to-image demo. The model/skeleton IDs match the App ResourcesManifest
	// (Assets/App/ImageGeneration); the generated-texture ID is caller-assigned (not in the manifest):
	// SceneSetupSystem shows a sprite bound to it, and ImageGenDemoSystem requests generation into it.
	inline const pg::UUID k_DiffusionCheckpointID("d8000000-0000-4000-8000-000000000001");
	inline const pg::UUID k_DiffusionLoraID("d8000000-0000-4000-8000-000000000002");
	inline const pg::UUID k_DiffusionControlNetID("d8000000-0000-4000-8000-000000000003");
	inline const pg::UUID k_DiffusionSkeletonID("d8000000-0000-4000-8000-000000000004");
	inline const pg::UUID k_LivingRoomImageID("d8000000-0000-4000-8000-000000000006");
	inline const pg::UUID k_GeneratedTextureID("d8000000-0000-4000-8000-0000000000ff");
}
