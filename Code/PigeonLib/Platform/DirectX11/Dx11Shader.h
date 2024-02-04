#pragma once

#include "pigeon/Renderer/Shader.h"

#include <string>
#include <d3d11.h>
#include <DirectXMath.h>

namespace pig
{
	class Dx11Shader : public Shader
	{
	public:
		struct Data
		{
			pig::U_Ptr<ID3D11InputLayout, pig::ReleaseDeleter> m_InputLayout = nullptr;
			pig::U_Ptr<ID3D11VertexShader, pig::ReleaseDeleter> m_VertexShader = nullptr;
			pig::U_Ptr<ID3D11PixelShader, pig::ReleaseDeleter> m_PixelShader = nullptr;

			std::string m_Name;
		};
#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif
		static DirectX::XMMATRIX ConvertGLMMatrixToDX(const glm::mat4& glmMatrix);

		Dx11Shader(const std::string& filepath);
		Dx11Shader(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout);
		~Dx11Shader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return m_Data.m_Name; }

		virtual void UploadUniformInt(const std::string& name, int value) const override;

		virtual void UploadUniformFloat(const std::string& name, float value) const override;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const override;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const override;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const override;

		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const override;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const override;

	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<unsigned int, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);

	private:
		Data m_Data;
	};
}