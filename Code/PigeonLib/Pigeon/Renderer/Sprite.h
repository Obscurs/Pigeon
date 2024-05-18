#pragma once

#include "Pigeon/Core/UUID.h"
#include <glm/glm.hpp>

namespace pig
{
	class Sprite
	{
	public:
		struct Data
		{
			glm::vec4 m_TexCoordsRect = glm::vec4(0.f, 0.f, 0.f, 0.f);;
			pig::UUID m_TextureID{};
			glm::vec2 m_TextureSize = glm::vec2(0.f, 0.f);
			glm::mat4 m_Transform = glm::mat4(1.f);
			glm::vec3 m_Origin = glm::vec3(0.f, 0.f, 0.f);
		};

		Sprite(const glm::mat4 transform, const glm::vec4 texCoords, const pig::UUID& textureID, const glm::vec3 origin = glm::vec3(0.f, 1.f, 0.f));
		~Sprite() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void SetTransform(const glm::mat4& transform);
		void SetTexCoords(const glm::vec4& texCoords);
		void SetOrigin(const glm::vec3& origin);

		const glm::mat4& GetTransform() const { return m_Data.m_Transform; }
		const glm::vec4& GetTexCoordsRect() const { return m_Data.m_TexCoordsRect; }
		const glm::vec3& GetOrigin() const { return m_Data.m_Origin; }
		const pig::UUID& GetTextureID() const { return m_Data.m_TextureID; }
		const glm::vec2& GetTextureSize() const { return m_Data.m_TextureSize; }

	private:
		Data m_Data;
	};
}