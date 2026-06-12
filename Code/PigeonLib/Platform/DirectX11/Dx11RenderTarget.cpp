#include "pch.h"
#include "Platform/DirectX11/Dx11RenderTarget.h"

#include "Pigeon/Core/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

pg::Dx11RenderTarget::Dx11RenderTarget(uint32_t width, uint32_t height)
	: m_Width(width), m_Height(height)
{
	auto context = static_cast<pg::Dx11Context*>(pg::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11Device* device = context->GetPd3dDevice();

	// Colour buffer: a texture bindable both as a render target (drawn into) and a shader resource
	// (sampled by the 2D pass).
	D3D11_TEXTURE2D_DESC colorDesc = {};
	colorDesc.Width = width;
	colorDesc.Height = height;
	colorDesc.MipLevels = 1;
	colorDesc.ArraySize = 1;
	colorDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	colorDesc.SampleDesc.Count = 1;
	colorDesc.Usage = D3D11_USAGE_DEFAULT;
	colorDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> colorTexture;
	HRESULT hr = device->CreateTexture2D(&colorDesc, nullptr, &colorTexture);
	PG_CORE_ASSERT(!FAILED(hr), "Failed to create render target colour texture");

	hr = device->CreateRenderTargetView(colorTexture.Get(), nullptr, &m_RenderTargetView);
	PG_CORE_ASSERT(!FAILED(hr), "Failed to create render target view");

	ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	hr = device->CreateShaderResourceView(colorTexture.Get(), nullptr, &shaderResourceView);
	PG_CORE_ASSERT(!FAILED(hr), "Failed to create render target shader resource view");
	m_ColorTexture = std::make_shared<pg::Dx11Texture2D>(shaderResourceView, width, height);

	// Depth buffer for the offscreen 3D pass (the window back buffer has no depth).
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> depthTexture;
	hr = device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
	PG_CORE_ASSERT(!FAILED(hr), "Failed to create render target depth texture");

	hr = device->CreateDepthStencilView(depthTexture.Get(), nullptr, &m_DepthStencilView);
	PG_CORE_ASSERT(!FAILED(hr), "Failed to create render target depth stencil view");
}
