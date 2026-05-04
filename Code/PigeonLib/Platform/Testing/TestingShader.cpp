#include "pch.h"
#include "Platform/Testing/TestingShader.h"

pig::TestingShader::TestingShader(const std::string& filepath)
{

}

pig::TestingShader::TestingShader(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout)
{
	
}

void pig::TestingShader::Bind() const
{
	
}

void pig::TestingShader::Unbind() const
{
	
}

void pig::TestingShader::UploadUniformInt(const std::string& name, int value) const
{
	
}

void pig::TestingShader::UploadUniformFloat(const std::string& name, float value) const
{

}

void pig::TestingShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
{

}

void pig::TestingShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
{
}

void pig::TestingShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
{
}

void pig::TestingShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
{
}

void pig::TestingShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
{
}
