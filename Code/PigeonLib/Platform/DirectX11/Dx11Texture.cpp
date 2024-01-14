#include "pch.h"

#include "Dx11Texture.h"

#include "Pigeon/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

#include "vendor/stb_image/stb_image.h"

pig::Dx11Texture2D::Dx11Texture2D(const std::string& path)
	: m_Path(path)
{
    //ARNAU TODO MOVE THIS!
    auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
    ID3D11Device* device = context->GetPd3dDevice();
    /*ID3D11RasterizerState* pRasterizerState;
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
        device->CreateBlendState(&desc, &pBlendState);
    }

    // Create the rasterizer state
    {
        D3D11_RASTERIZER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.ScissorEnable = true;
        desc.DepthClipEnable = true;
        device->CreateRasterizerState(&desc, &pRasterizerState);
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
        device->CreateDepthStencilState(&desc, &pDepthStencilState);
    }

    // Setup blend state
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    context->GetPd3dDeviceContext()->OMSetBlendState(pBlendState, blend_factor, 0xffffffff);
    context->GetPd3dDeviceContext()->OMSetDepthStencilState(pDepthStencilState, 0);
    context->GetPd3dDeviceContext()->RSSetState(pRasterizerState);
    */
    int width, height, channels;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) throw std::runtime_error("Failed to load image!");

    m_Width = width;
    m_Height = height;

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = m_Width;
    textureDesc.Height = m_Height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;
    initData.SysMemPitch = static_cast<UINT>(width * 4);

    ID3D11Texture2D* pTexture = nullptr;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &pTexture);
    PG_CORE_ASSERT(!FAILED(hr), "Failed to create texture!");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(pTexture, &srvDesc, &m_TextureView);
    PG_CORE_ASSERT(!FAILED(hr), "Failed to create shader resource view!");
    pTexture->Release();

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 0;

    device->CreateSamplerState(&sampDesc, &m_SamplerState);

    stbi_image_free(data);
}

pig::Dx11Texture2D::~Dx11Texture2D()
{

}

void pig::Dx11Texture2D::Bind(uint32_t slot) const
{
    auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
    ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();
    deviceContext->PSSetShaderResources(slot, 1, m_TextureView.GetAddressOf());
    deviceContext->PSSetSamplers(slot, 1, &m_SamplerState);
}