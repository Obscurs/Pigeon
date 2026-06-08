#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace pg
{
	// Engine-origin transform request emitted by CameraSystem for camera panning. Applied by the engine
	// TransformResolveSystem alongside the aggregated app request.
	struct CameraTransformRequestOneFrameComponent
	{
		CameraTransformRequestOneFrameComponent() = default;
		CameraTransformRequestOneFrameComponent(const CameraTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
