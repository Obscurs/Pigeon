#include "pch.h"
#include "Sprite.h"

#include "Pigeon/Renderer/Renderer2D.h"
#include "Pigeon/Renderer/Texture.h"

pig::Sprite::Sprite(const glm::vec3 pos, const  glm::vec2 scale, const glm::vec2 spriteSize, const glm::vec2 offset, const std::string& textureID)
{
	const pig::Texture2D* tex = pig::Renderer2D::GetTexture(textureID);
	PG_CORE_ASSERT(tex, "Trying to create a sprite with a null texture");
	glm::vec4 texCoordsRect(0.f, 0.f, 1.f, 1.f);
	if (tex)
	{
		const float texWidth = tex->GetWidth();
		const float texHeight = tex->GetHeight();

		texCoordsRect.x = offset.x / texWidth;
		texCoordsRect.y = offset.y / texWidth;
		texCoordsRect.z = texCoordsRect.x + spriteSize.x / texWidth;
		texCoordsRect.w = texCoordsRect.y + spriteSize.y / texHeight;
	}
	
	m_Data = { pos, scale, spriteSize, offset, texCoordsRect, textureID };
}
