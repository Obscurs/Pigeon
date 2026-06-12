#pragma once
#include <glm/glm.hpp>

namespace sbx
{
	// Per-3D-model spin parameters consumed by ModelSpinSystem to rotate the model about the world Y
	// axis each frame (emitted as a transform request). A model with zero rotation speed holds its
	// initial orientation at its anchor.
	struct ModelSpinComponent
	{
		ModelSpinComponent() = default;
		ModelSpinComponent(const ModelSpinComponent&) = default;

		glm::vec3 m_Anchor{ 0.f, 0.f, 0.f }; // fixed world position the model spins in place at
		float m_RotationSpeed{ 0.f };        // radians per second about the world Y axis
		float m_Elapsed{ 0.f };              // accumulated animated seconds, advanced each frame
	};
}
