#include "pch.h"
#include "Dx11Context.h"

#include <D3DCompiler.h>

namespace
{
// Simple vertex shader
char* s_VsCode =
	"struct VS_INPUT\n"
	"{\n"
	"	float4 Pos : POSITION;\n"
	"	float4 Col : COLOR;\n"
	"};\n"
	"struct PS_INPUT\n"
	"{\n"
	"	float4 Pos : SV_POSITION; \n"
	"	float4 Col : COLOR; \n"
	"};\n"
	"PS_INPUT main(VS_INPUT input)\n"
	"{\n"
	"	PS_INPUT output;\n"
	"	output.Pos = input.Pos; // Pass position to rasterizer\n"
	"	output.Col = input.Col; // Pass color to pixel shader\n"
	"	return output;\n"
	"};";

// Simple pixel shader
char* s_PsCode =
	"struct PS_INPUT\n"
	"{\n"
	"	float4 Pos : SV_POSITION;\n"
	"	float4 Col : COLOR;\n"
	"};\n"
	"float4 main(PS_INPUT input) : SV_TARGET\n"
	"{\n"
	"    return input.Col; // Output the interpolated color\n"
	"};";
}
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
		// Initialize Direct3D
		if (!CreateDeviceD3D(m_HWnd))
		{
			CleanupDeviceD3D();
			//UnregisterClass(m_Data.m_Title, m_Data.m_HInstance);
			return;
		}
		CompileShaders();

		PG_CORE_ASSERT(status, "Failed to initialize DirectX!");
	}

	void Dx11Context::Begin()
	{
		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (m_ResizeWidth != 0 && m_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			m_PSwapChain->ResizeBuffers(0, m_ResizeWidth, m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			m_Width = m_ResizeWidth;
			m_Height = m_ResizeHeight;
			m_ResizeWidth = m_ResizeHeight = 0;
			CreateRenderTarget();
		}

		const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
		m_Pd3dDeviceContext->OMSetRenderTargets(1, &m_MainRenderTargetView, nullptr);
		m_Pd3dDeviceContext->ClearRenderTargetView(m_MainRenderTargetView, clear_color_with_alpha);

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(m_Width);  // Replace 'width' with actual width
		viewport.Height = static_cast<float>(m_Height); // Replace 'height' with actual height
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_Pd3dDeviceContext->RSSetViewports(1, &viewport);

		m_Pd3dDeviceContext->IASetInputLayout(m_InputLayout);

		// Set the vertex and pixel shaders
		m_Pd3dDeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
		m_Pd3dDeviceContext->PSSetShader(m_PixelShader, nullptr, 0);
	}

	void Dx11Context::SwapBuffers()
	{
		m_PSwapChain->Present(1, 0);
	}

	void Dx11Context::SetSize(unsigned int width, unsigned int height)
	{
		m_ResizeHeight = height;
		m_ResizeWidth = width;
	}

	void Dx11Context::CleanupDeviceD3D()
	{
		CleanupRenderTarget();
		if (m_PSwapChain) { m_PSwapChain->Release(); m_PSwapChain = nullptr; }
		if (m_Pd3dDeviceContext) { m_Pd3dDeviceContext->Release(); m_Pd3dDeviceContext = nullptr; }
		if (m_Pd3dDevice) { m_Pd3dDevice->Release(); m_Pd3dDevice = nullptr; }
		if (m_InputLayout) { m_InputLayout->Release(); m_InputLayout = nullptr; }
		if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
		if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	}

	void Dx11Context::CreateRenderTarget()
	{
		ID3D11Texture2D* pBackBuffer;
		m_PSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		m_Pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_MainRenderTargetView);
		pBackBuffer->Release();
	}

	void Dx11Context::CleanupRenderTarget()
	{
		if (m_MainRenderTargetView) { m_MainRenderTargetView->Release(); m_MainRenderTargetView = nullptr; }
	}

	void Dx11Context::CompileShaders()
	{
		bool success = true;
		ID3D10Blob* errorBlob;
		ID3D10Blob* vsBlob;
		ID3D10Blob* psBlob;
		// Compile vertex shader
		if (FAILED(D3DCompile(s_VsCode, strlen(s_VsCode), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob))) {
			// Handle errors
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
				success = false;
			}
			// Handle further error
		}

		// Compile pixel shader
		if (FAILED(D3DCompile(s_PsCode, strlen(s_PsCode), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob))) {
			// Handle errors
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
				success = false;
			}
			// Handle further error
		}
		if (success)
		{
			m_Pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
			m_Pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

			// Define and create the input layout
			D3D11_INPUT_ELEMENT_DESC layout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			UINT numElements = ARRAYSIZE(layout);

			m_Pd3dDevice->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
		}

		vsBlob->Release();
		psBlob->Release();

		PG_CORE_ASSERT(success, "Failed to compile shaders!");
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

		CreateRenderTarget();
		return true;
	}
}