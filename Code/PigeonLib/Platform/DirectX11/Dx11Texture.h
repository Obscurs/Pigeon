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
		struct Data
		{
			uint32_t m_Width, m_Height;

			ComPtr<ID3D11ShaderResourceView> m_TextureView;
			ID3D11SamplerState* m_SamplerState;
		};

		Dx11Texture2D(const std::string& path);
		Dx11Texture2D(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif 

		virtual ~Dx11Texture2D() = default;

		virtual uint32_t GetWidth() const override { return m_Data.m_Width; }
		virtual uint32_t GetHeight() const override { return m_Data.m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		void CreateTextue(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		Data m_Data;
	};
	
	class Dx11Texture2DArray : public Texture2DArray
	{
	public:
		struct Data
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

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif 

		virtual ~Dx11Texture2DArray() = default;

		virtual uint32_t GetWidth() const override { return m_Data.m_Width; }
		virtual uint32_t GetHeight() const override { return m_Data.m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;

		unsigned int GetCount() const override { return m_Data.m_Count; };
		unsigned int GetMaxCount() const override { return m_Data.m_MaxCount; };

	private:
		//void CreateTextue(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		Data m_Data;
	};
}