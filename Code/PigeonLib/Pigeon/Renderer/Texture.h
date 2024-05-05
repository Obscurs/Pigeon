#pragma once

#include <string>

#include "Pigeon/Core/Core.h"

namespace pig 
{
	class Texture
	{
	public:
		struct Data
		{
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
		};
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const { return m_Data.m_Width; }
		virtual uint32_t GetHeight() const { return m_Data.m_Height; }

		virtual void Bind(uint32_t slot = 0) const = 0;

	protected:
		Data m_Data;
	};

	// TODO Arnau: have some kind of textureid (uuid) generated on the texture creation and use it instead of the string handle. 
	// Also maybe some kind of resource manager with a refcount to prevent texture leaks
	class Texture2D : public Texture
	{
	public:
		virtual glm::vec4 GetTexCoordsRect(glm::vec2 pixelsOffset, glm::vec2 pixelsSize) const = 0;

		static pig::S_Ptr<Texture2D> Create(const std::string& path);
		static pig::S_Ptr<Texture2D> Create(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
	};

	class Texture2DArray : public Texture
	{
	public:
		static pig::S_Ptr<Texture2DArray> Create(unsigned int count);
		virtual void Append(const std::string& path) = 0;
		virtual unsigned int GetCount() const = 0;
		virtual unsigned int GetMaxCount() const = 0;
	};
}