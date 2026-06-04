#include "pch.h"
#include "Platform/Testing/TestingShader.h"

pg::TestingShader::TestingShader(const std::string& filepath)
{

}

pg::TestingShader::TestingShader(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout)
{
	
}

void pg::TestingShader::Bind() const
{
	
}

void pg::TestingShader::Unbind() const
{
	
}

void pg::TestingShader::UploadUniformInt(const std::string& name, int value) const
{
	
}

void pg::TestingShader::UploadUniformFloat(const std::string& name, float value) const
{

}

void pg::TestingShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) const
{

}

void pg::TestingShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) const
{
}

void pg::TestingShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) const
{
}

void pg::TestingShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
{
}

void pg::TestingShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const
{
}
