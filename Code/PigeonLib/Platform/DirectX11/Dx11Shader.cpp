#include "pch.h"
#include "Dx11Shader.h"

#include <D3DCompiler.h>

#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Buffer.h"
#include "Platform/DirectX11/Dx11Context.h"

namespace pigeon
{
	struct Vector3BufferType
	{
		DirectX::XMFLOAT3 myVector;
		float padding; // Padding to align to 16 bytes
	};

	static unsigned int GetConstantBufferSlot(std::string name)
	{
		if (name == "u_ViewProjection")
		{
			return 0;
		}
		else if(name == "u_Transform")
		{
			return 1;
		}
		else if (name == "u_Color")
		{
			return 2;
		}
		else
		{
			PG_CORE_ERROR("uniform not defined");
		}
		return 0;
	}
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

	static void BufferLayoutToDx11InputDesc(const BufferLayout& bufferLayout, std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc)
	{
		for (const BufferElement& elem : bufferLayout)
		{
			layoutDesc.push_back({ elem.Name.c_str(), 0, ShaderDataTypeToDx11BaseType(elem.Type), 0, elem.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
	}
}

namespace pigeon
{
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

			device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_Data.m_VertexShader);
			device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_Data.m_PixelShader);

			SetLayout(buffLayout);

			std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
			BufferLayoutToDx11InputDesc(buffLayout, layoutDesc);

			device->CreateInputLayout(layoutDesc.data(), layoutDesc.size(), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_Data.m_InputLayout);
		}

		vsBlob->Release();
		psBlob->Release();

		PG_CORE_ASSERT(success, "Failed to compile shaders!");
	}

	Dx11Shader::~Dx11Shader()
	{
		Unbind();

		if (m_Data.m_InputLayout) { m_Data.m_InputLayout->Release(); m_Data.m_InputLayout = nullptr; }
		if (m_Data.m_VertexShader) { m_Data.m_VertexShader->Release(); m_Data.m_VertexShader = nullptr; }
		if (m_Data.m_PixelShader) { m_Data.m_PixelShader->Release(); m_Data.m_PixelShader = nullptr; }
	}

	/*glm::mat4 Dx11Shader::ConvertDXMatrixToGLM(const DirectX::XMMATRIX& dxMatrix)
	{
		// Transpose is required because XMMATRIX is row-major and glm::mat4 is column-major
		//DirectX::XMMATRIX transposedMatrix = DirectX::XMMatrixTranspose(dxMatrix);

		// Copy the elements
		return glm::mat4(
			transposedMatrix.r[0].m128_f32[0], transposedMatrix.r[0].m128_f32[1], transposedMatrix.r[0].m128_f32[2], transposedMatrix.r[0].m128_f32[3],
			transposedMatrix.r[1].m128_f32[0], transposedMatrix.r[1].m128_f32[1], transposedMatrix.r[1].m128_f32[2], transposedMatrix.r[1].m128_f32[3],
			transposedMatrix.r[2].m128_f32[0], transposedMatrix.r[2].m128_f32[1], transposedMatrix.r[2].m128_f32[2], transposedMatrix.r[2].m128_f32[3],
			transposedMatrix.r[3].m128_f32[0], transposedMatrix.r[3].m128_f32[1], transposedMatrix.r[3].m128_f32[2], transposedMatrix.r[3].m128_f32[3]
		);
	}*/

	DirectX::XMMATRIX Dx11Shader::ConvertGLMMatrixToDX(const glm::mat4& glmMatrix)
	{
		// Since glm::mat4 is column-major and DirectX::XMMATRIX is row-major, a transpose is needed
		DirectX::XMMATRIX dxMatrix = DirectX::XMMATRIX(
			glmMatrix[0][0], glmMatrix[1][0], glmMatrix[2][0], glmMatrix[3][0],
			glmMatrix[0][1], glmMatrix[1][1], glmMatrix[2][1], glmMatrix[3][1],
			glmMatrix[0][2], glmMatrix[1][2], glmMatrix[2][2], glmMatrix[3][2],
			glmMatrix[0][3], glmMatrix[1][3], glmMatrix[2][3], glmMatrix[3][3]
		);

		// Optionally, transpose the result if you need the DirectX matrix in its default row-major format
		//return DirectX::XMMatrixTranspose(dxMatrix);
		return dxMatrix;
	}

	void Dx11Shader::Bind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->IASetInputLayout(m_Data.m_InputLayout);
		// Set the vertex and pixel shaders
		deviceContext->VSSetShader(m_Data.m_VertexShader, nullptr, 0);
		deviceContext->PSSetShader(m_Data.m_PixelShader, nullptr, 0);
	}

	void Dx11Shader::Unbind() const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->VSSetShader(NULL, NULL, 0);
		deviceContext->IASetInputLayout(NULL);
	}

	void Dx11Shader::UploadUniformInt(const std::string& name, int value) const
	{
		//TODO
	}

	void Dx11Shader::UploadUniformFloat(const std::string& name, float value) const
	{
		//TODO
	}

	void Dx11Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
	{
		//TODO
	}

	void Dx11Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
	{
		Vector3BufferType vectorData;
		vectorData.myVector = DirectX::XMFLOAT3(value.x, value.y, value.z);
		vectorData.padding = 0.0f; // Set padding to a defined value

		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		pigeon::Dx11ConstantBuffer<Vector3BufferType> buffer(&vectorData);
		buffer.Bind(GetConstantBufferSlot(name));
	}

	void Dx11Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
	{
		//TODO
	}

	void Dx11Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		//TODO
	}

	void Dx11Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
	{
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

		DirectX::XMMATRIX mat = ConvertGLMMatrixToDX(matrix);
		pigeon::Dx11ConstantBuffer<DirectX::XMMATRIX> buffer(&mat);
		buffer.Bind(GetConstantBufferSlot(name));
	}
}