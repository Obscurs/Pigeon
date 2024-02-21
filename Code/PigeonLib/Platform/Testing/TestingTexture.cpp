#include "pch.h"

#include "TestingTexture.h"

uint32_t pig::TestingTexture2D::s_ExpectedWidth = 0;
uint32_t pig::TestingTexture2D::s_ExpectedHeight = 0;

pig::TestingTexture2D::TestingTexture2D(const std::string& path)
{
	m_Data.m_Height = s_ExpectedHeight;
	m_Data.m_Width = s_ExpectedWidth;
}

pig::TestingTexture2D::TestingTexture2D(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data)
{
	m_Data.m_Height = s_ExpectedHeight;
	m_Data.m_Width = s_ExpectedWidth;
}

void pig::TestingTexture2D::Bind(uint32_t slot) const
{
}

pig::TestingTexture2DArray::TestingTexture2DArray(unsigned int count)
{
}

void pig::TestingTexture2DArray::Append(const std::string& path)
{
}

void pig::TestingTexture2DArray::Bind(uint32_t slot) const
{
}
