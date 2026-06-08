#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace pg
{
	// Absolute transform values plus per-channel dirty flags. Shared payload embedded in transform
	// change requests; the resolver applies only the channels whose flag is set. Requests are absolute
	// (they set values); a mover that wants relative motion reads the current value first.
	struct TransformRequestData
	{
		bool m_SetPosition = false;
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };

		bool m_SetRotation = false;
		glm::quat m_Rotation{ 1.f, 0.f, 0.f, 0.f };

		bool m_SetScale = false;
		glm::vec3 m_Scale{ 1.f, 1.f, 1.f };

		bool m_SetBounds = false;
		glm::vec3 m_BoundsMin{ 0.f, 0.f, 0.f };
		glm::vec3 m_BoundsMax{ 1.f, 1.f, 0.f };
	};
}
