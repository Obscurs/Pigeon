#include "pch.h"
#include "Dx11RendererAPI.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Context.h"

void pig::Dx11RendererAPI::CreateRenderTarget()
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11Texture2D* pBackBuffer;
	context->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	context->GetPd3dDevice()->CreateRenderTargetView(pBackBuffer, nullptr, &m_Data.m_MainRenderTargetView);
	pBackBuffer->Release();
}

void pig::Dx11RendererAPI::CleanupRenderTarget()
{
	if (m_Data.m_MainRenderTargetView) { m_Data.m_MainRenderTargetView->Release(); m_Data.m_MainRenderTargetView = nullptr; }
}

void pig::Dx11RendererAPI::SetClearColor(const glm::vec4& color)
{
	m_Data.m_ClearColor[0] = color.r;
	m_Data.m_ClearColor[1] = color.g;
	m_Data.m_ClearColor[2] = color.b;
	m_Data.m_ClearColor[3] = color.a;
}

void pig::Dx11RendererAPI::Begin()
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
		
	if (!m_Data.m_Initialized)
	{
		m_Data.m_Initialized = true;
		CreateRenderTarget();
	}
	// Handle window resize (we don't resize directly in the WM_SIZE handler)
	if (context->NeedsResize())
	{
		CleanupRenderTarget();
		context->ResizeBuffers();
		CreateRenderTarget();
	}

	context->GetPd3dDeviceContext()->OMSetRenderTargets(1, &m_Data.m_MainRenderTargetView, nullptr);
	Clear();

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(context->GetWidth());  // Replace 'width' with actual width
	viewport.Height = static_cast<float>(context->GetHeight()); // Replace 'height' with actual height
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	context->GetPd3dDeviceContext()->RSSetViewports(1, &viewport);
}

void pig::Dx11RendererAPI::End()
{

}

void pig::Dx11RendererAPI::Clear()
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->ClearRenderTargetView(m_Data.m_MainRenderTargetView, m_Data.m_ClearColor);
}

void pig::Dx11RendererAPI::DrawIndexed(unsigned int count)
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->GetPd3dDeviceContext()->DrawIndexed(count, 0, 0);
}