#pragma once
#include <glm/glm.hpp>

namespace pg
{
	// Axis-aligned extent of an entity's geometry in its own local space, as min/max corners. Default
	// spans the unit quad (0,0,0)-(1,1,0). The bottom edge (m_Min) transformed to world space feeds the
	// render order. Added and written only by TransformResolveSystem.
	struct LocalBoundsComponent
	{
		LocalBoundsComponent() = default;
		LocalBoundsComponent(const LocalBoundsComponent&) = default;

		glm::vec3 m_Min{ 0.f, 0.f, 0.f };
		glm::vec3 m_Max{ 1.f, 1.f, 0.f };
	};
}
