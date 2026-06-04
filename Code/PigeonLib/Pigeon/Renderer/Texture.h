#pragma once

#include <string>

#include "Pigeon/Core/Core.h"

namespace pg 
{
	enum class EMappedTextureType
	{
		eQuad,
		eText
	};

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

	class Texture2D : public Texture
	{
	public:
		virtual glm::vec4 GetTexCoordsRect(glm::vec2 pixelsOffset, glm::vec2 pixelsSize) const = 0;

		static pg::S_Ptr<Texture2D> Create(const std::string& path);
		static pg::S_Ptr<Texture2D> Create(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		static bool FlipY();
	};

	class Texture2DArray : public Texture
	{
	public:
		static pg::S_Ptr<Texture2DArray> Create(unsigned int count);
		virtual void Append(const std::string& path) = 0;
		virtual unsigned int GetCount() const = 0;
		virtual unsigned int GetMaxCount() const = 0;
	};

	struct TextureData
	{
		pg::EMappedTextureType m_TextureType = pg::EMappedTextureType::eQuad;
		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
		unsigned int m_Channels = 0;
		pg::S_Ptr<pg::Texture2D> m_Texture = nullptr;
		pg::UUID m_TextureID;
	};
}