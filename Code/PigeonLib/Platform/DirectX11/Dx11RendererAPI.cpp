#include "pch.h"
#include "Dx11RendererAPI.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Context.h"

namespace pigeon 
{
	void Dx11RendererAPI::CreateRenderTarget()
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11Texture2D* pBackBuffer;
		context->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		context->GetPd3dDevice()->CreateRenderTargetView(pBackBuffer, nullptr, &m_MainRenderTargetView);
		pBackBuffer->Release();
	}

	void Dx11RendererAPI::CleanupRenderTarget()
	{
		if (m_MainRenderTargetView) { m_MainRenderTargetView->Release(); m_MainRenderTargetView = nullptr; }
	}

	void Dx11RendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor[0] = color.r;
		m_ClearColor[1] = color.g;
		m_ClearColor[2] = color.b;
		m_ClearColor[3] = color.a;
	}

	void Dx11RendererAPI::Begin()
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		
		if (!m_Initialized)
		{
			CreateRenderTarget();
		}
		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (context->NeedsResize())
		{
			CleanupRenderTarget();
			context->ResizeBuffers();
			CreateRenderTarget();
		}

		context->GetPd3dDeviceContext()->OMSetRenderTargets(1, &m_MainRenderTargetView, nullptr);
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

	void Dx11RendererAPI::End()
	{

	}

	void Dx11RendererAPI::Clear()
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		context->GetPd3dDeviceContext()->ClearRenderTargetView(m_MainRenderTargetView, m_ClearColor);
	}

	void Dx11RendererAPI::DrawIndexed()
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		context->GetPd3dDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->GetPd3dDeviceContext()->DrawIndexed(3, 0, 0);
	}
}