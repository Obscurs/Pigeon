#include "pch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Pigeon/Application.h"

#include "Platform/DirectX11/Dx11Shader.h"

pig::S_Ptr<pig::Shader> pig::Shader::Create(const std::string& filepath)
{
	switch (Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11: return std::make_shared<pig::Dx11Shader>(filepath);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pig::S_Ptr<pig::Shader> pig::Shader::Create(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_shared<pig::Dx11Shader>(name, vertexSrc, fragmentSrc, buffLayout);
	}
	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

void pig::ShaderLibrary::Add(const std::string& name, const pig::S_Ptr<pig::Shader>& shader)
{
	PG_CORE_ASSERT(!Exists(name), "Shader already exists!");
	m_Shaders[name] = shader;
}

void pig::ShaderLibrary::Add(const pig::S_Ptr<pig::Shader>& shader)
{
	auto& name = shader->GetName();
	Add(name, shader);
}

pig::S_Ptr<pig::Shader> pig::ShaderLibrary::Load(const std::string& filepath)
{
	auto shader = pig::Shader::Create(filepath);
	Add(shader);
	return shader;
}

pig::S_Ptr<pig::Shader> pig::ShaderLibrary::Load(const std::string& name, const std::string& filepath)
{
	auto shader = pig::Shader::Create(filepath);
	Add(name, shader);
	return shader;
}

pig::S_Ptr<pig::Shader> pig::ShaderLibrary::Get(const std::string& name)
{
	PG_CORE_ASSERT(Exists(name), "Shader not found!");
	return m_Shaders[name];
}

bool pig::ShaderLibrary::Exists(const std::string& name) const
{
	return m_Shaders.find(name) != m_Shaders.end();
}

