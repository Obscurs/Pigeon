#pragma once

#include "pigeon/Renderer/Shader.h"

#include <string>

namespace pig
{
	class TestingShader : public Shader
	{
	public:
		TestingShader(const std::string& filepath);
		TestingShader(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout);
		~TestingShader() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const std::string& GetName() const override { return TEST_Name; }

		virtual void UploadUniformInt(const std::string& name, int value) const override;

		virtual void UploadUniformFloat(const std::string& name, float value) const override;
		virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) const override;
		virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) const override;
		virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) const override;

		virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const override;
		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const override;

		std::string TEST_Name;
	};
}