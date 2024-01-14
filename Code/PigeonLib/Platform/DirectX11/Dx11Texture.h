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
		Dx11Texture2D(const std::string& path);
		virtual ~Dx11Texture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		
		ComPtr<ID3D11ShaderResourceView> m_TextureView;
		ID3D11SamplerState* m_SamplerState;
	};
}