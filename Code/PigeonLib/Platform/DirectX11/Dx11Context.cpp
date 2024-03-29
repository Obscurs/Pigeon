#include "pch.h"

#include "Dx11Context.h"

pig::Dx11Context::Dx11Context(HWND windowHandle)
{
	m_Data.m_HWnd = windowHandle;
	PG_CORE_ASSERT(windowHandle, "Window handle is null!")
}

void pig::Dx11Context::Shutdown()
{
	CleanupDeviceD3D();
}

void pig::Dx11Context::Init()
{
	if (!CreateDeviceD3D(m_Data.m_HWnd))
	{
		CleanupDeviceD3D();
		return;
	}
}

void pig::Dx11Context::SwapBuffers()
{
	m_Data.m_PSwapChain->Present(1, 0);
}

void pig::Dx11Context::SetSize(unsigned int width, unsigned int height)
{
	m_Data.m_ResizeHeight = height;
	m_Data.m_ResizeWidth = width;
	if (m_Data.m_Width == 0 && m_Data.m_Height == 0)
	{
		ResizeBuffers();
	}
}

bool pig::Dx11Context::NeedsResize() const
{
	return m_Data.m_ResizeWidth != 0 && m_Data.m_ResizeHeight != 0;
}

void pig::Dx11Context::ResizeBuffers()
{
	PG_CORE_ASSERT(m_Data.m_ResizeWidth != 0 || m_Data.m_ResizeHeight != 0, "Called resized but no resize was needed");
	m_Data.m_PSwapChain->ResizeBuffers(0, m_Data.m_ResizeWidth, m_Data.m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
	m_Data.m_Width = m_Data.m_ResizeWidth;
	m_Data.m_Height = m_Data.m_ResizeHeight;
	m_Data.m_ResizeWidth = m_Data.m_ResizeHeight = 0;
}

void pig::Dx11Context::CleanupDeviceD3D()
{
	if (m_Data.m_PSwapChain) { m_Data.m_PSwapChain.reset(); }
	if (m_Data.m_Pd3dDeviceContext) { m_Data.m_Pd3dDeviceContext.reset(); }
	if (m_Data.m_Pd3dDevice) { m_Data.m_Pd3dDevice.reset(); }
	m_Data.m_HWnd = nullptr;
}

bool pig::Dx11Context::CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	
	ID3D11Device* pd3dDevice = nullptr;
	ID3D11DeviceContext* pd3dDeviceContext = nullptr;
	IDXGISwapChain* pSwapChain = nullptr;

	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pd3dDeviceContext);
	if (res != S_OK)
		return false;

	m_Data.m_PSwapChain.reset(pSwapChain);
	m_Data.m_Pd3dDeviceContext.reset(pd3dDeviceContext);
	m_Data.m_Pd3dDevice.reset(pd3dDevice);
	return true;
}