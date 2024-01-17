#include "pch.h"
#include "Dx11Shader.h"

#include <D3DCompiler.h>

#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Buffer.h"
#include "Platform/DirectX11/Dx11Context.h"

namespace
{
	struct IntBufferType
	{
		int data;
		int padding[3] = { 0, 0, 0 }; // Padding to align to 16 bytes
	};

	struct FloatBufferType
	{
		float data;
		float padding[3] = { 0.f, 0.f, 0.f }; // Padding to align to 16 bytes
	};

	struct Vector2BufferType
	{
		DirectX::XMFLOAT2 data;
		float padding[2] = { 0.f, 0.f}; // Padding to align to 16 bytes
	};

	struct Vector3BufferType
	{
		DirectX::XMFLOAT3 data;
		float padding = 0.f; // Padding to align to 16 bytes
	};

	struct Vector4BufferType
	{
		DirectX::XMFLOAT4 data;
	};

	static unsigned int GetConstantBufferSlot(std::string name)
	{
		if (name == "u_ViewProjection")
		{
			return 0;
		}
		else if (name == "u_Transform")
		{
			return 1;
		}
		else if (name == "u_Color")
		{
			return 2;
		}
		else
		{
			PG_CORE_ASSERT(false, "uniform not defined");
		}
		return 0;
	}
	static DXGI_FORMAT ShaderDataTypeToDx11BaseType(pig::ShaderDataType type)
	{
		switch (type)
		{
		case pig::ShaderDataType::Float:    return DXGI_FORMAT_R32_FLOAT;
		case pig::ShaderDataType::Float2:   return DXGI_FORMAT_R32G32_FLOAT;
		case pig::ShaderDataType::Float3:   return DXGI_FORMAT_R32G32B32_FLOAT;
		case pig::ShaderDataType::Float4:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
			//case pig::ShaderDataType::Mat3:    
			//case pig::ShaderDataType::Mat4:     
		case pig::ShaderDataType::Int:      return DXGI_FORMAT_R32_SINT;
		case pig::ShaderDataType::Int2:     return DXGI_FORMAT_R32G32_SINT;
		case pig::ShaderDataType::Int3:     return DXGI_FORMAT_R32G32B32_SINT;
		case pig::ShaderDataType::Int4:     return DXGI_FORMAT_R32G32B32A32_SINT;
		case pig::ShaderDataType::Bool:     return DXGI_FORMAT_R32_UINT;
		}

		PG_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return DXGI_FORMAT_UNKNOWN;
	}

	static void BufferLayoutToDx11InputDesc(const pig::BufferLayout& bufferLayout, std::vector<D3D11_INPUT_ELEMENT_DESC>& layoutDesc)
	{
		for (const pig::BufferElement& elem : bufferLayout)
		{
			layoutDesc.push_back({ elem.Name.c_str(), 0, ShaderDataTypeToDx11BaseType(elem.Type), 0, elem.Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		}
	}
}

pig::Dx11Shader::Dx11Shader(const char* vertexSrc, const char* fragmentSrc, const pig::BufferLayout& buffLayout)
{
	pig::RAII_PtrRelease<ID3D10Blob> errorBlob;
	pig::RAII_PtrRelease<ID3D10Blob> vsBlob;
	pig::RAII_PtrRelease<ID3D10Blob> psBlob;
	// Compile vertex shader
	if (FAILED(D3DCompile(vertexSrc, strlen(vertexSrc), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob.value, &errorBlob.value))) {
		// Handle errors
		if (errorBlob.value) {
			OutputDebugStringA((char*)errorBlob.value->GetBufferPointer());
			PG_CORE_ASSERT(false, "Failed to compile vertex shaders!");
			return;
		}
	}

	// Compile pixel shader
	if (FAILED(D3DCompile(fragmentSrc, strlen(fragmentSrc), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob.value, &errorBlob.value))) {
		// Handle errors
		if (errorBlob.value) {
			OutputDebugStringA((char*)errorBlob.value->GetBufferPointer());
			PG_CORE_ASSERT(false, "Failed to compile fragment shaders!");
			return;
		}
	}

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11Device* device = context->GetPd3dDevice();

	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	device->CreateVertexShader(vsBlob.value->GetBufferPointer(), vsBlob.value->GetBufferSize(), nullptr, &vs);
	m_Data.m_VertexShader.reset(vs);
	device->CreatePixelShader(psBlob.value->GetBufferPointer(), psBlob.value->GetBufferSize(), nullptr, &ps);
	m_Data.m_PixelShader.reset(ps);
	SetLayout(buffLayout);

	std::vector<D3D11_INPUT_ELEMENT_DESC> layoutDesc;
	BufferLayoutToDx11InputDesc(buffLayout, layoutDesc);

	ID3D11InputLayout* layout = nullptr;
	device->CreateInputLayout(layoutDesc.data(), layoutDesc.size(), vsBlob.value->GetBufferPointer(), vsBlob.value->GetBufferSize(), &layout);
	m_Data.m_InputLayout.reset(layout);
}

pig::Dx11Shader::~Dx11Shader()
{
	Unbind();
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

DirectX::XMMATRIX pig::Dx11Shader::ConvertGLMMatrixToDX(const glm::mat4& glmMatrix)
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

void pig::Dx11Shader::Bind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	deviceContext->IASetInputLayout(m_Data.m_InputLayout.get());
	// Set the vertex and pixel shaders
	deviceContext->VSSetShader(m_Data.m_VertexShader.get(), nullptr, 0);
	deviceContext->PSSetShader(m_Data.m_PixelShader.get(), nullptr, 0);
}

void pig::Dx11Shader::Unbind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	deviceContext->VSSetShader(NULL, NULL, 0);
	deviceContext->VSSetShader(NULL, NULL, 0);
	deviceContext->IASetInputLayout(NULL);
}

void pig::Dx11Shader::UploadUniformInt(const std::string& name, int value) const
{
	IntBufferType vectorData;
	vectorData.data = value;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	pig::Dx11ConstantBuffer<IntBufferType> buffer(&vectorData);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformFloat(const std::string& name, float value) const
{
	FloatBufferType vectorData;
	vectorData.data = value;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	pig::Dx11ConstantBuffer<FloatBufferType> buffer(&vectorData);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
{
	Vector2BufferType vectorData;
	vectorData.data = DirectX::XMFLOAT2(value.x, value.y);

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	pig::Dx11ConstantBuffer<Vector2BufferType> buffer(&vectorData);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
{
	Vector3BufferType vectorData;
	vectorData.data = DirectX::XMFLOAT3(value.x, value.y, value.z);

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	pig::Dx11ConstantBuffer<Vector3BufferType> buffer(&vectorData);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
{
	Vector4BufferType vectorData;
	vectorData.data = DirectX::XMFLOAT4(value.x, value.y, value.z, value.w);

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	pig::Dx11ConstantBuffer<Vector4BufferType> buffer(&vectorData);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	DirectX::XMMATRIX mat = ConvertGLMMatrixToDX(matrix);
	pig::Dx11ConstantBuffer<DirectX::XMMATRIX> buffer(&mat);
	buffer.Bind(GetConstantBufferSlot(name));
}

void pig::Dx11Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11DeviceContext* deviceContext = context->GetPd3dDeviceContext();

	DirectX::XMMATRIX mat = ConvertGLMMatrixToDX(matrix);
	pig::Dx11ConstantBuffer<DirectX::XMMATRIX> buffer(&mat);
	buffer.Bind(GetConstantBufferSlot(name));
}
