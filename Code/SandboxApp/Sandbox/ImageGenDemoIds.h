#pragma once

#include "Pigeon/Core/UUID.h"

namespace sbx
{
	// Shared UUIDs for the text-to-image demo. The model/skeleton/input-image IDs match the Sandbox
	// ResourcesManifest (Assets/Sandbox/ImageGeneration); the four result-texture IDs are caller-assigned
	// (not in the manifest): the pipeline (ADR 0011) registers its restyled background, OpenPose pose
	// hint, final composite, and optional integration pass under them, and SceneSetupSystem shows a
	// sprite bound to each.
	inline const pg::UUID k_DiffusionCheckpointID("d8000000-0000-4000-8000-000000000001");
	inline const pg::UUID k_DiffusionLoraID("d8000000-0000-4000-8000-000000000002");
	inline const pg::UUID k_DiffusionControlNetID("d8000000-0000-4000-8000-000000000003");
	inline const pg::UUID k_DiffusionSkeletonID("d8000000-0000-4000-8000-000000000004");
	inline const pg::UUID k_LivingRoomImageID("d8000000-0000-4000-8000-000000000006");
	inline const pg::UUID k_BackgroundTextureID("d8000000-0000-4000-8000-0000000000fa");
	inline const pg::UUID k_HintTextureID("d8000000-0000-4000-8000-0000000000fb");
	inline const pg::UUID k_CompositeTextureID("d8000000-0000-4000-8000-0000000000fc");
	inline const pg::UUID k_IntegratedTextureID("d8000000-0000-4000-8000-0000000000fd");
}
