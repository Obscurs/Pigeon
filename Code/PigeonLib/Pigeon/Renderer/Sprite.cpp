#include "pch.h"
#include "Sprite.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Texture.h"

pig::Sprite::Sprite(const glm::vec3 pos, const  glm::vec2 scale, const glm::vec4 texCoordsRect, const std::string& textureID)
{
	const pig::S_Ptr<pig::Texture2D> texture = pig::Renderer2D::GetTexture(textureID);
	PG_CORE_ASSERT(texture, "Texture not found for sprite");
	m_Data = { pos, scale, texCoordsRect, textureID, glm::vec2(texture->GetWidth(), texture->GetHeight())};
}
