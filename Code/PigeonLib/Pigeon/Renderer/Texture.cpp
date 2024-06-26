#include "pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/DirectX11/Dx11Texture.h"
#include "Platform/Testing/TestingTexture.h"

pig::S_Ptr<pig::Texture2D> pig::Texture2D::Create(const std::string& path)
{
	switch (pig::Renderer::GetAPI())
	{
		case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case pig::RendererAPI::API::DirectX11:  return std::make_shared<pig::Dx11Texture2D>(path);
		case pig::RendererAPI::API::Testing:  return std::make_shared<pig::TestingTexture2D>(path);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pig::S_Ptr<pig::Texture2D> pig::Texture2D::Create(unsigned int width, unsigned int height, unsigned int channels, const unsigned char* data)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_shared<pig::Dx11Texture2D>(width, height, channels, data);
	case pig::RendererAPI::API::Testing:  return std::make_shared<pig::TestingTexture2D>(width, height, channels, data);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pig::S_Ptr<pig::Texture2DArray> pig::Texture2DArray::Create(unsigned int count)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_shared<pig::Dx11Texture2DArray>(count);
	case pig::RendererAPI::API::Testing:  return std::make_shared<pig::TestingTexture2DArray>(count);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

bool pig::Texture2D::FlipY()
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return true;
	case pig::RendererAPI::API::Testing:  return false;
	default:
		return false;
	}
}
