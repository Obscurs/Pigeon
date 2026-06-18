#pragma once

#include "Pigeon/Core/UUID.h"

namespace pg
{
	// App-emitted request (engine-typed, so the engine DiffusionSystem can read it) to rasterize an
	// OpenPose Skeleton's canonical hint and register it as a Generated Texture under m_TargetTextureID
	// (caller-assigned, like an audio Voice Handle), so the pose can be shown on screen rather than only
	// fed internally to a ControlNet job (ADR 0011). Synchronous — not a Diffusion Job. m_Width/m_Height
	// at 0 fall back to the engine Generation Config defaults.
	struct RasterizeOpenPoseHintRequestOneFrameComponent
	{
		pg::UUID m_SkeletonID;
		pg::UUID m_TargetTextureID;
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
	};
}
