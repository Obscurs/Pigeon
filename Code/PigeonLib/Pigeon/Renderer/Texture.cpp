#include "pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/DirectX11/Dx11Texture.h"
#include "Platform/Testing/TestingTexture.h"

pg::S_Ptr<pg::Texture2D> pg::Texture2D::Create(const std::string& path)
{
	switch (pg::Renderer::GetAPI())
	{
		case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11Texture2D>(path);
		case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingTexture2D>(path);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pg::S_Ptr<pg::Texture2D> pg::Texture2D::Create(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11Texture2D>(width, height, channels, data);
	case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingTexture2D>(width, height, channels, data);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pg::S_Ptr<pg::Texture2DArray> pg::Texture2DArray::Create(unsigned int count)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11Texture2DArray>(count);
	case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingTexture2DArray>(count);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

bool pg::Texture2D::FlipY()
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return true;
	case pg::RendererAPI::API::Testing:  return false;
	default:
		return false;
	}
}
