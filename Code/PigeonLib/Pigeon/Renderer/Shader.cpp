#include "pch.h"
#include "Pigeon/Renderer/Shader.h"

#include "Pigeon/Core/Application.h"
#include "Pigeon/Renderer/Renderer.h"
#include "Platform/DirectX11/Dx11Shader.h"
#include "Platform/Testing/TestingShader.h"

pg::S_Ptr<pg::Shader> pg::Shader::Create(const std::string& filepath)
{
	switch (Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11: return std::make_shared<pg::Dx11Shader>(filepath);
	case pg::RendererAPI::API::Testing: return std::make_shared<pg::TestingShader>(filepath);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pg::S_Ptr<pg::Shader> pg::Shader::Create(const std::string& name, const char* vertexSrc, const char* fragmentSrc, const char* buffLayout)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11Shader>(name, vertexSrc, fragmentSrc, buffLayout);
	case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingShader>(name, vertexSrc, fragmentSrc, buffLayout);
	}
	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

void pg::ShaderLibrary::Add(const std::string& name, const pg::S_Ptr<pg::Shader>& shader)
{
	PG_CORE_ASSERT(!Exists(name), "Shader already exists!");
	m_Shaders[name] = shader;
}

void pg::ShaderLibrary::Add(const pg::S_Ptr<pg::Shader>& shader)
{
	auto& name = shader->GetName();
	Add(name, shader);
}

pg::S_Ptr<pg::Shader> pg::ShaderLibrary::Load(const std::string& filepath)
{
	auto shader = pg::Shader::Create(filepath);
	Add(shader);
	return shader;
}

pg::S_Ptr<pg::Shader> pg::ShaderLibrary::Load(const std::string& name, const std::string& filepath)
{
	auto shader = pg::Shader::Create(filepath);
	Add(name, shader);
	return shader;
}

pg::S_Ptr<pg::Shader> pg::ShaderLibrary::Get(const std::string& name)
{
	PG_CORE_ASSERT(Exists(name), "Shader not found!");
	return m_Shaders[name];
}

bool pg::ShaderLibrary::Exists(const std::string& name) const
{
	return m_Shaders.find(name) != m_Shaders.end();
}

