#include "pch.h"
#include "Dx11RendererAPI.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Context.h"

void pig::Dx11RendererAPI::Init()
{
    auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());

    ID3D11BlendState* pBlendState;
    ID3D11DepthStencilState* pDepthStencilState;
    // Create the blending setup
    {
        D3D11_BLEND_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.AlphaToCoverageEnable = false;
        desc.RenderTarget[0].BlendEnable = true;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        context->GetPd3dDevice()->CreateBlendState(&desc, &pBlendState);
    }

    // Create depth-stencil State
    {
        D3D11_DEPTH_STENCIL_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = false;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable = false;
        desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        desc.BackFace = desc.FrontFace;
        context->GetPd3dDevice()->CreateDepthStencilState(&desc, &pDepthStencilState);
    }

    // Setup blend state
    const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
    context->GetPd3dDeviceContext()->OMSetBlendState(pBlendState, blend_factor, 0xffffffff);
    context->GetPd3dDeviceContext()->OMSetDepthStencilState(pDepthStencilState, 0);
}

void pig::Dx11RendererAPI::CreateRenderTarget()
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	pig::RAII_PtrRelease<ID3D11Texture2D> pBackBuffer;
	context->GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer.value));
	ID3D11RenderTargetView* renderView;
	context->GetPd3dDevice()->CreateRenderTargetView(pBackBuffer.value, nullptr, &renderView);
	m_Data.m_MainRenderTargetView.reset(renderView);
}

void pig::Dx11RendererAPI::CleanupRenderTarget()
{
	if (m_Data.m_MainRenderTargetView) { m_Data.m_MainRenderTargetView.reset(); }
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

	context->GetPd3dDeviceContext()->OMSetRenderTargets(1, pig::U_PtrToPtr<ID3D11RenderTargetView, pig::ReleaseDeleter>(m_Data.m_MainRenderTargetView), nullptr);
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
	context->GetPd3dDeviceContext()->ClearRenderTargetView(m_Data.m_MainRenderTargetView.get(), m_Data.m_ClearColor);
}

void pig::Dx11RendererAPI::DrawIndexed(unsigned int count)
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->GetPd3dDeviceContext()->DrawIndexed(count, 0, 0);
}