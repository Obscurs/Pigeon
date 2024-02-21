#pragma once

#include <glm/glm.hpp>

namespace pig
{
	class Sprite
	{
	public:
		struct Data
		{
			glm::vec3 m_Position;
			glm::vec2 m_Scale;
			glm::vec4 m_TexCoordsRect;
			std::string m_TextureID;
		};

		Sprite(const glm::vec3 pos, const  glm::vec2 scale, const glm::vec4 texCoords, const std::string& textureID);
		~Sprite() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void SetPosition(const glm::vec3& position) { m_Data.m_Position = position; }
		void SetScale(const glm::vec2& scale) { m_Data.m_Scale = scale; }
		void SetTexCoords(const glm::vec4& texCoords) { m_Data.m_TexCoordsRect = texCoords; }

		const glm::vec3& GetPosition() const { return m_Data.m_Position; }
		const glm::vec2& GetScale() const { return m_Data.m_Scale; }
		const glm::vec4& GetTexCoordsRect() const { return m_Data.m_TexCoordsRect; }
		const std::string& GetTextureID() const { return m_Data.m_TextureID; }

	private:
		Data m_Data;
	};
}