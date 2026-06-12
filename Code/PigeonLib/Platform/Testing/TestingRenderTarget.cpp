#include "pch.h"
#include "Platform/Testing/TestingRenderTarget.h"

#include "Pigeon/Renderer/Texture.h"

#include <vector>

pg::TestingRenderTarget::TestingRenderTarget(uint32_t width, uint32_t height)
	: m_Width(width), m_Height(height)
{
	std::vector<unsigned char> pixels(4, 255);
	m_ColorTexture = pg::Texture2D::Create(1, 1, 4, pixels.data());
}
