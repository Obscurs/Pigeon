#pragma once
#include <glm/glm.hpp>

namespace pg
{
	// Resolved world placement, composed from Position/Rotation/Scale by TransformComposeSystem and
	// read by the render bridges. m_SortKey is the world-space Y of the local bottom edge; the renderer
	// draws lower keys behind higher ones. Added and written only by TransformComposeSystem.
	struct WorldTransformComponent
	{
		WorldTransformComponent() = default;
		WorldTransformComponent(const WorldTransformComponent&) = default;

		glm::mat4 m_Matrix{ 1.f };
		float m_SortKey{ 0.f };
	};
}
