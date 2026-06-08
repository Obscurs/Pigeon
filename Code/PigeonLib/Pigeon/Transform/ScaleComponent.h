#pragma once
#include <glm/glm.hpp>

namespace pg
{
	// World-space scale of a non-UI entity. Default is unit scale. Added and written only by
	// TransformResolveSystem.
	struct ScaleComponent
	{
		ScaleComponent() = default;
		ScaleComponent(const ScaleComponent&) = default;

		glm::vec3 m_Scale{ 1.f, 1.f, 1.f };
	};
}
