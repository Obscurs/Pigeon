#pragma once

#include "Pigeon/Renderer/Texture.h"

namespace pig 
{
	class TestingTexture2D : public Texture2D
	{
	public:

		TestingTexture2D(const std::string& path);
		TestingTexture2D(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);

		virtual ~TestingTexture2D() = default;

		virtual void Bind(uint32_t slot = 0) const override;

		static uint32_t s_ExpectedWidth;
		static uint32_t s_ExpectedHeight;
	};
	
	class TestingTexture2DArray : public Texture2DArray
	{
	public:
		TestingTexture2DArray(unsigned int count);

		virtual void Append(const std::string& path) override;

		virtual ~TestingTexture2DArray() = default;

		virtual uint32_t GetWidth() const override { return 0; }
		virtual uint32_t GetHeight() const override { return 0; }

		virtual void Bind(uint32_t slot = 0) const override;

		unsigned int GetCount() const override { return 0; };
		unsigned int GetMaxCount() const override { return 0; };
	};
}