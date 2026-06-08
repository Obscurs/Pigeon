#pragma once
#include <glm/glm.hpp>

namespace pg
{
	// World-space translation of a non-UI entity. Added and written only by TransformResolveSystem.
	struct PositionComponent
	{
		PositionComponent() = default;
		PositionComponent(const PositionComponent&) = default;

		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
	};
}
