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
			glm::vec2 m_SpriteSize;
			glm::vec2 m_Offset;
			glm::vec4 m_TexCoordsRect;
			std::string m_TextureID;
		};

		Sprite(const glm::vec3 pos, const  glm::vec2 scale, const glm::vec2 spriteSize, const glm::vec2 offset, const std::string& textureID);
		~Sprite() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void SetPosition(const glm::vec3& position) { m_Data.m_Position = position; }
		void SetScale(const glm::vec2& scale) { m_Data.m_Scale = scale; }
		void SetOffset(const glm::vec2& offset) { m_Data.m_Offset = offset; }

		const glm::vec3& GetPosition() const { return m_Data.m_Position; }
		const glm::vec2& GetScale() const { return m_Data.m_Scale; }
		const glm::vec2& GetOffset() const { return m_Data.m_Offset; }
		const glm::vec2& GetSpriteSize() const { return m_Data.m_SpriteSize; }
		const glm::vec4& GetTexCoordsRect() const { return m_Data.m_TexCoordsRect; }
		const std::string& GetTextureID() const { return m_Data.m_TextureID; }

	private:
		Data m_Data;
	};
}