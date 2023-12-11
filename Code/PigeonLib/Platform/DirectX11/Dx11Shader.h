#pragma once

#include "pigeon/Renderer/Shader.h"

#include <string>
#include <d3d11.h>
#include <DirectXMath.h>

namespace pigeon
{
	class Dx11Shader : public Shader
	{
	public:
		struct Data
		{
			ID3D11InputLayout* m_InputLayout = nullptr;
			ID3D11VertexShader* m_VertexShader = nullptr;
			ID3D11PixelShader* m_PixelShader = nullptr;
		};
#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif
		//static glm::mat4 ConvertDXMatrixToGLM(const DirectX::XMMATRIX& dxMatrix);
		static DirectX::XMMATRIX ConvertGLMMatrixToDX(const glm::mat4& glmMatrix);
		Dx11Shader(const char* vertexSrc, const char* fragmentSrc, const BufferLayout& buffLayout);
		~Dx11Shader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UploadUniformInt(const std::string& name, int value) const override;

		virtual void UploadUniformFloat(const std::string& name, float value) const override;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const override;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const override;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const override;

		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const override;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const override;

	private:
		Data m_Data;
	};
}