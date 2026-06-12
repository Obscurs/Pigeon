#pragma once
#include "Pigeon/Core/UUID.h"

namespace pg
{
	// References a loaded 3D model resource by UUID, placing it in a world-space scene via the
	// transform components (exactly as SpriteComponent references a texture). The forthcoming 3D
	// render pass resolves m_ModelID against the resource map's model map to draw the geometry.
	struct ModelComponent
	{
		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;

		pg::UUID m_ModelID;
	};
}
