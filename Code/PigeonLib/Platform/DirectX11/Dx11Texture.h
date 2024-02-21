#pragma once

#include "Pigeon/Renderer/Texture.h"

#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace pig 
{
	class Dx11Texture2D : public Texture2D
	{
	public:
		struct DxData
		{
			ComPtr<ID3D11ShaderResourceView> m_TextureView;
			ID3D11SamplerState* m_SamplerState;
		};

		virtual glm::vec4 GetTexCoordsRect(glm::vec2 pixelsOffset, glm::vec2 pixelsSize) const override;

		Dx11Texture2D(const std::string& path);
		Dx11Texture2D(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);

		virtual ~Dx11Texture2D() = default;

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		void CreateTextue(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		DxData m_DxData;
	};
	
	class Dx11Texture2DArray : public Texture2DArray
	{
	public:
		struct DxData
		{
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
			unsigned int m_Count = 0;
			unsigned int m_MaxCount = 0;

			//ComPtr<ID3D11ShaderResourceView> m_TextureView;
			//ID3D11SamplerState* m_SamplerState;
		};

		Dx11Texture2DArray(unsigned int count);

		virtual void Append(const std::string& path) override;

		virtual ~Dx11Texture2DArray() = default;

		virtual uint32_t GetWidth() const override { return m_DxData.m_Width; }
		virtual uint32_t GetHeight() const override { return m_DxData.m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;

		unsigned int GetCount() const override { return m_DxData.m_Count; };
		unsigned int GetMaxCount() const override { return m_DxData.m_MaxCount; };

	private:
		//void CreateTextue(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		DxData m_DxData;
	};
}