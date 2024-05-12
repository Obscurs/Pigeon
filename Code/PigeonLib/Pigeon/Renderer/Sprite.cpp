#include "pch.h"
#include "Sprite.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Texture.h"

pig::Sprite::Sprite(const glm::mat4 transform, const glm::vec4 texCoordsRect, const std::string& textureID, const glm::vec3 origin)
{
	const pig::S_Ptr<pig::Texture2D> texture = pig::Renderer2D::GetTexture(textureID);
	PG_CORE_ASSERT(texture, "Texture not found for sprite");
	m_Data = { texCoordsRect, textureID, glm::vec2(texture->GetWidth(), texture->GetHeight()), transform, std::move(origin) };
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
