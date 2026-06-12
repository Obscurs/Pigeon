#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace sbx
{
	// Transform change request emitted by ModelSpinSystem each frame to drive a 3D model's animated
	// Y-axis rotation (and hold its anchor position). Aggregated by the SandboxApp TransformResolveSystem.
	struct ModelSpinTransformRequestOneFrameComponent
	{
		ModelSpinTransformRequestOneFrameComponent() = default;
		ModelSpinTransformRequestOneFrameComponent(const ModelSpinTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
