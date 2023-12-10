#include "pch.h"
#include "Dx11Shader.h"

#include <D3DCompiler.h>

#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Context.h"

namespace pigeon
{
	static DXGI_FORMAT ShaderDataTypeToDx11BaseType(ShaderDataType type)
	{
		switch (type)
		{
		case pigeon::ShaderDataType::Float:    return DXGI_FORMAT_R32_FLOAT;
		case pigeon::ShaderDataType::Float2:   return DXGI_FORMAT_R32G32_FLOAT;
		case pigeon::ShaderDataType::Float3:   return DXGI_FORMAT_R32G32B32_FLOAT;
		case pigeon::ShaderDataType::Float4:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
			//case pigeon::ShaderDataType::Mat3:    
			//case pigeon::ShaderDataType::Mat4:     
		case pigeon::ShaderDataType::Int:      return DXGI_FORMAT_R32_SINT;
		case pigeon::ShaderDataType::Int2:     return DXGI_FORMAT_R32G32_SINT;
		case pigeon::ShaderDataType::Int3:     return DXGI_FORMAT_R32G32B32_SINT;
		case pigeon::ShaderDataType::Int4:     return DXGI_FORMAT_R32G32B32A32_SINT;
		case pigeon::ShaderDataType::Bool:     return DXGI_FORMAT_R32_UINT;
		}

		PG_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return DXGI_FORMAT_UNKNOWN;
	}

	void BufferLayoutToDx11InputDesc(const BufferLayout& bufferLayout, std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc)
	{
		for (const BufferElement& elem : bufferLayout)
		{
			layoutDesc.push_back({ elem.Name.c_str(), 0, ShaderDataTypeToDx11BaseType(elem.Type), 0, elem.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
	}

	Dx11Shader::Dx11Shader(const char* vertexSrc, const char* fragmentSrc, const BufferLayout& buffLayout)
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

			SetLayout(buffLayout);

			std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
			BufferLayoutToDx11InputDesc(buffLayout, layoutDesc);

			device->CreateInputLayout(layoutDesc.data(), layoutDesc.size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
		}

		vsBlob->Release();
		psBlob->Release();

		PG_CORE_ASSERT(success, "Failed to compile shaders!");
	}

	Dx11Shader::~Dx11Shader()
	{
		Unbind();

		if (m_InputLayout) { m_InputLayout->Release(); m_InputLayout = nullptr; }
		if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
		if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	}

	void Dx11Shader::Bind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->IASetInputLayout(m_InputLayout);
		// Set the vertex and pixel shaders
		deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
		deviceContext->PSSetShader(m_PixelShader, nullptr, 0);
	}

	void Dx11Shader::Unbind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->IASetInputLayout(NULL);
	}
}