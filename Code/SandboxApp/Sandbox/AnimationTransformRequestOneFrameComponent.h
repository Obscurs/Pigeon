#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace sbx
{
	// Transform change request emitted by QuadAnimationSystem each frame to drive a quad's animated
	// position and rotation. Aggregated by the SandboxApp TransformResolveSystem.
	struct AnimationTransformRequestOneFrameComponent
	{
		AnimationTransformRequestOneFrameComponent() = default;
		AnimationTransformRequestOneFrameComponent(const AnimationTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
