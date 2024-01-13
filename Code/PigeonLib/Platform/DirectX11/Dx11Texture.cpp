#include "pch.h"

#include "Dx11Texture.h"

#include "Pigeon/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

#include "vendor/stb_image/stb_image.h"

pig::Dx11Texture2D::Dx11Texture2D(const std::string& path)
	: m_Path(path)
{
    int width, height, channels;
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
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

    auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
    ID3D11Device* device = context->GetPd3dDevice();

    HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &m_Texture);
    PG_CORE_ASSERT(!FAILED(hr), "Failed to create texture!");

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, &m_TextureView);
    PG_CORE_ASSERT(!FAILED(hr), "Failed to create shader resource view!");

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
}