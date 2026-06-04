#include "pch.h"

#include "TestingTexture.h"

uint32_t pg::TestingTexture2D::s_ExpectedWidth = 0;
uint32_t pg::TestingTexture2D::s_ExpectedHeight = 0;

glm::vec4 pg::TestingTexture2D::GetTexCoordsRect(glm::vec2 pixelsOffset, glm::vec2 pixelsSize) const
{
	glm::vec2 offsetNormalized(pixelsOffset.x / s_ExpectedWidth, pixelsOffset.y / s_ExpectedHeight);
	glm::vec2 sizeNormalized(pixelsSize.x / s_ExpectedWidth, pixelsSize.y / s_ExpectedHeight);

	return glm::vec4(offsetNormalized.x, offsetNormalized.y, offsetNormalized.x + sizeNormalized.x, offsetNormalized.y + sizeNormalized.y);
}

pg::TestingTexture2D::TestingTexture2D(const std::string& path)
{
	m_Data.m_Height = s_ExpectedHeight;
	m_Data.m_Width = s_ExpectedWidth;
}

pg::TestingTexture2D::TestingTexture2D(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data)
{
	m_Data.m_Height = s_ExpectedHeight;
	m_Data.m_Width = s_ExpectedWidth;
}

void pg::TestingTexture2D::Bind(uint32_t slot) const
{
}

pg::TestingTexture2DArray::TestingTexture2DArray(unsigned int count)
{
}

void pg::TestingTexture2DArray::Append(const std::string& path)
{
}

void pg::TestingTexture2DArray::Bind(uint32_t slot) const
{
}
