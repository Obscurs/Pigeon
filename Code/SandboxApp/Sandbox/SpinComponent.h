#pragma once
#include <glm/glm.hpp>

namespace sbx
{
	// Per-quad animation parameters consumed by QuadAnimationSystem to compute the quad's position,
	// rotation, and colour each frame (emitted as a transform request). A quad whose speeds are all
	// zero renders static at its anchor.
	struct SpinComponent
	{
		SpinComponent() = default;
		SpinComponent(const SpinComponent&) = default;

		glm::vec3 m_Anchor{ 0.f, 0.f, 0.f }; // base world position (render order comes from world Y)
		glm::vec3 m_Scale{ 1.f, 1.f, 1.f };
		glm::vec3 m_BaseColor{ 1.f, 1.f, 1.f };

		float m_RotationSpeed{ 0.f };   // radians per second about the z axis
		float m_OrbitRadius{ 0.f };     // distance orbited around the anchor
		float m_OrbitSpeed{ 0.f };      // radians per second around the anchor
		float m_ColorCycleSpeed{ 0.f }; // colour animation speed; 0 => constant base colour

		float m_Elapsed{ 0.f };         // accumulated animated seconds, advanced each frame
	};
}
