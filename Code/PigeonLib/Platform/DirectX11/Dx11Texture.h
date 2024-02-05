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

		virtual ~Dx11Texture2D();

		virtual uint32_t GetWidth() const override { return m_Data.m_Width; }
		virtual uint32_t GetHeight() const override { return m_Data.m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		void CreateTextue(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data);
		Data m_Data;
	};
}