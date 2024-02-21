#include "pch.h"
#include "Sprite.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Texture.h"

pig::Sprite::Sprite(const glm::vec3 pos, const  glm::vec2 scale, const glm::vec4 texCoordsRect, const std::string& textureID)
{
	m_Data = { pos, scale, texCoordsRect, textureID };
}
