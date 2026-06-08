#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace sbx
{
	// Transform change request emitted by SceneSetupSystem to place the camera, sprite, and labels.
	// Aggregated by the SandboxApp TransformResolveSystem.
	struct SceneTransformRequestOneFrameComponent
	{
		SceneTransformRequestOneFrameComponent() = default;
		SceneTransformRequestOneFrameComponent(const SceneTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
