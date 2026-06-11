#pragma once
#include "Pigeon/Transform/TransformRequestData.h"

namespace sbx
{
	// Transform change request emitted by CharacterControlSystem while the showcase character moves.
	// Aggregated by the SandboxApp TransformResolveSystem like the other app transform requests.
	struct CharacterTransformRequestOneFrameComponent
	{
		CharacterTransformRequestOneFrameComponent() = default;
		CharacterTransformRequestOneFrameComponent(const CharacterTransformRequestOneFrameComponent&) = default;

		pg::TransformRequestData m_Data;
	};
}
