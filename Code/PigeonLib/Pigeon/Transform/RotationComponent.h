#pragma once
#include <glm/gtc/quaternion.hpp>

namespace pg
{
	// World-space orientation of a non-UI entity, as a quaternion (full 3D). Default is identity.
	// Added and written only by TransformResolveSystem.
	struct RotationComponent
	{
		RotationComponent() = default;
		RotationComponent(const RotationComponent&) = default;

		glm::quat m_Rotation{ 1.f, 0.f, 0.f, 0.f };
	};
}
