#pragma once

#include "Pigeon/Renderer/RenderTarget.h"
#include "Platform/DirectX11/Dx11Texture.h"

#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

namespace pg
{
	class Dx11RenderTarget : public RenderTarget
	{
	public:
		Dx11RenderTarget(uint32_t width, uint32_t height);
		virtual ~Dx11RenderTarget() = default;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual pg::S_Ptr<pg::Texture2D> GetColorTexture() const override { return m_ColorTexture; }

		ID3D11RenderTargetView* GetRenderTargetView() const { return m_RenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const { return m_DepthStencilView.Get(); }

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
		pg::S_Ptr<pg::Dx11Texture2D> m_ColorTexture;
	};
}
