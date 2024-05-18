#include "pch.h"
#include "Sprite.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Texture.h"

pig::Sprite::Sprite(const glm::mat4 transform, const glm::vec4 texCoordsRect, const pig::UUID& textureID, const glm::vec3 origin)
{
	const Texture2D& texture = pig::Renderer2D::GetTexture(textureID);

	m_Data = { texCoordsRect, textureID, glm::vec2(texture.GetWidth(), texture.GetHeight()), transform, std::move(origin) };
}

void pig::Sprite::SetTransform(const glm::mat4& transform)
{
	m_Data.m_Transform = transform;
}

void pig::Sprite::SetTexCoords(const glm::vec4& texCoords)
{
	m_Data.m_TexCoordsRect = texCoords;
}

void pig::Sprite::SetOrigin(const glm::vec3& origin)
{
	m_Data.m_Origin = origin;
}
