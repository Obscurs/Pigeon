#include "pch.h"
#include "Shader.h"

#include <D3DCompiler.h>

#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Context.h"

namespace pigeon 
{

	Shader::Shader(const char* vertexSrc, const char* fragmentSrc)
	{
		bool success = true;
		ID3D10Blob* errorBlob;
		ID3D10Blob* vsBlob;
		ID3D10Blob* psBlob;
		// Compile vertex shader
		if (FAILED(D3DCompile(vertexSrc, strlen(vertexSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, &errorBlob))) {
			// Handle errors
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
				success = false;
			}
			// Handle further error
		}

		// Compile pixel shader
		if (FAILED(D3DCompile(fragmentSrc, strlen(fragmentSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, &errorBlob))) {
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
			auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
			ID3D11Device* device = context->GetPd3dDevice();

			device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_VertexShader);
			device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);

			// Define and create the input layout
			D3D11_INPUT_ELEMENT_DESC layout[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};
			UINT numElements = ARRAYSIZE(layout);

			device->CreateInputLayout(layout, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
		}

		vsBlob->Release();
		psBlob->Release();

		PG_CORE_ASSERT(success, "Failed to compile shaders!");
	}

	Shader::~Shader()
	{
		Unbind();

		if (m_InputLayout) { m_InputLayout->Release(); m_InputLayout = nullptr; }
		if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
		if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	}

	void Shader::Bind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->IASetInputLayout(m_InputLayout);
		// Set the vertex and pixel shaders
		deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
		deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
	}

	void Shader::Unbind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->IASetInputLayout(NULL);
	}
}