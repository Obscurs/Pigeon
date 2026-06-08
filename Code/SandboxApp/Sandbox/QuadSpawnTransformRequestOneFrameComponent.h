#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace sbx
{
	// Transform change request emitted by QuadSpawnSystem to place newly spawned quads.
	// Aggregated by the SandboxApp TransformResolveSystem.
	struct QuadSpawnTransformRequestOneFrameComponent
	{
		QuadSpawnTransformRequestOneFrameComponent() = default;
		QuadSpawnTransformRequestOneFrameComponent(const QuadSpawnTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
