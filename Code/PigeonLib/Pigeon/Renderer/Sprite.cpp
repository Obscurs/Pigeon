#include "pch.h"
#include "Pigeon/Renderer/Sprite.h"

#include "Pigeon/Renderer/Texture.h"

pg::Sprite::Sprite(const glm::mat4 transform, const glm::vec4 texCoordsRect, const pg::UUID& textureID, const glm::vec3 origin)
{
	m_Data = { texCoordsRect, textureID, transform, std::move(origin) };
}

void pg::Sprite::SetTransform(const glm::mat4& transform)
{
	m_Data.m_Transform = transform;
}

void pg::Sprite::SetTexCoords(const glm::vec4& texCoords)
{
	m_Data.m_TexCoordsRect = texCoords;
}

void pg::Sprite::SetOrigin(const glm::vec3& origin)
{
	m_Data.m_Origin = origin;
}
