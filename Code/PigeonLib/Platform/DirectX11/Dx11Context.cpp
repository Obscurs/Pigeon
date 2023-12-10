#include "pch.h"

#include "Dx11Context.h"

namespace pigeon 
{
	Dx11Context::Dx11Context(HWND windowHandle)
		: m_HWnd(windowHandle)
	{
		PG_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void Dx11Context::Shutdown()
	{
		CleanupDeviceD3D();
	}

	void Dx11Context::Init()
	{
		if (!CreateDeviceD3D(m_HWnd))
		{
			CleanupDeviceD3D();
			return;
		}
	}

	void Dx11Context::SwapBuffers()
	{
		m_PSwapChain->Present(1, 0);
	}

	void Dx11Context::SetSize(unsigned int width, unsigned int height)
	{
		m_ResizeHeight = height;
		m_ResizeWidth = width;
		if (m_Width == 0 && m_Height == 0)
		{
			ResizeBuffers();
		}
	}

	bool Dx11Context::NeedsResize() const
	{
		return m_ResizeWidth != 0 && m_ResizeHeight != 0;
	}

	void Dx11Context::ResizeBuffers()
	{
		PG_CORE_ASSERT(m_ResizeWidth != 0 || m_ResizeHeight != 0, "Called resized but no resize was needed");
		m_PSwapChain->ResizeBuffers(0, m_ResizeWidth, m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		m_Width = m_ResizeWidth;
		m_Height = m_ResizeHeight;
		m_ResizeWidth = m_ResizeHeight = 0;
	}

	void Dx11Context::CleanupDeviceD3D()
	{
		if (m_PSwapChain) { m_PSwapChain->Release(); m_PSwapChain = nullptr; }
		if (m_Pd3dDeviceContext) { m_Pd3dDeviceContext->Release(); m_Pd3dDeviceContext = nullptr; }
		if (m_Pd3dDevice) { m_Pd3dDevice->Release(); m_Pd3dDevice = nullptr; }
	}

	bool Dx11Context::CreateDeviceD3D(HWND hWnd)
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
		HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_PSwapChain, &m_Pd3dDevice, &featureLevel, &m_Pd3dDeviceContext);
		if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
			res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_PSwapChain, &m_Pd3dDevice, &featureLevel, &m_Pd3dDeviceContext);
		if (res != S_OK)
			return false;

		return true;
	}
}