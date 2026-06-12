#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Pigeon/Core/Core.h"

#include <glm/glm.hpp>

namespace pg
{
	// One interleaved vertex of a loaded 3D model: object-space position, normal, and texture
	// coordinate. The attributes a future 3D render pass uploads to the GPU.
	struct ModelVertex
	{
		glm::vec3 m_Position{ 0.f, 0.f, 0.f };
		glm::vec3 m_Normal{ 0.f, 0.f, 0.f };
		glm::vec2 m_TexCoord{ 0.f, 0.f };
	};

	// Loadable 3D model resource parsed from a Wavefront .obj file. Holds CPU-side triangle
	// geometry (an interleaved vertex list plus a triangle index list); polygon faces are
	// fan-triangulated at load. Mirrors how SoundClip/Font are loaded into the resource map; a
	// later 3D renderer uploads these buffers to the GPU.
	class Model
	{
	public:
		Model(std::vector<ModelVertex> vertices, std::vector<uint32_t> indices)
			: m_Vertices(std::move(vertices)), m_Indices(std::move(indices))
		{
		}
		~Model() = default;

		const std::vector<ModelVertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		static pg::S_Ptr<Model> Create(const std::string& path);

	private:
		std::vector<ModelVertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
}
