#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace pg
{
	// Single aggregated transform request produced by the SandboxApp resolve tier and applied by the
	// engine TransformResolveSystem. At most one per target entity per frame.
	struct ResolvedTransformRequestOneFrameComponent
	{
		ResolvedTransformRequestOneFrameComponent() = default;
		ResolvedTransformRequestOneFrameComponent(const ResolvedTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
