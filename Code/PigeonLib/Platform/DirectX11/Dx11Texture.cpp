#include "pch.h"

#include "Dx11Texture.h"

#include "Pigeon/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

#include "vendor/stb_image/stb_image.h"

namespace
{
    class RAIIHelperStbi
    {
    public:
        RAIIHelperStbi() : value(nullptr) {}
        ~RAIIHelperStbi() { if (value) stbi_image_free(value); }

        stbi_uc* value;
    };
}

pig::Dx11Texture2D::Dx11Texture2D(const std::string& path)
	: m_Path(path)
{
    auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
    ID3D11Device* device = context->GetPd3dDevice();
    
    int width, height, channels;
    RAIIHelperStbi dataLoaded;

    unsigned char* dataProcessed = nullptr;

    dataLoaded.value = stbi_load(path.c_str(), &width, &height, &channels, STBI_default);
    PG_CORE_ASSERT(dataLoaded.value, "Failed to load image!");
    PG_CORE_ASSERT(channels == 3 || channels == 4, "Unsupported image format!");

    m_Width = width;
    m_Height = height;

    const UINT bytesPerPixel = 4;

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
    initData.SysMemPitch = static_cast<UINT>(width * bytesPerPixel);
    if (channels == 4)
    {
        initData.pSysMem = dataLoaded.value;
    }
    else if (channels == 3)
    {
        //ARNAU TODO RAII
        dataProcessed = new unsigned char[width * height * bytesPerPixel];

        for (int i = 0; i < width * height; ++i) {
            dataProcessed[i * 4 + 0] = dataLoaded.value[i * 3 + 0]; // R
            dataProcessed[i * 4 + 1] = dataLoaded.value[i * 3 + 1]; // G
            dataProcessed[i * 4 + 2] = dataLoaded.value[i * 3 + 2]; // B
            dataProcessed[i * 4 + 3] = 255;                         // A
        }

        initData.pSysMem = dataProcessed;
    }

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
    sampDesc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 0;

    device->CreateSamplerState(&sampDesc, &m_SamplerState);

    if (dataProcessed) 
        delete[] dataProcessed;
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