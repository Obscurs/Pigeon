#include "pch.h"
#include "Pigeon/Renderer/Buffer.h"

#include "Pigeon/Renderer/Renderer.h"
#include "Platform/DirectX11/Dx11Buffer.h"
#include "Platform/Testing/TestingBuffer.h"

pg::S_Ptr<pg::VertexBuffer> pg::VertexBuffer::Create(const float* vertices, uint32_t size, uint32_t stride)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11VertexBuffer>(vertices, size, stride);
	case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingVertexBuffer>(vertices, size, stride);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pg::S_Ptr<pg::IndexBuffer> pg::IndexBuffer::Create(const uint32_t* indices, uint32_t size)
{
	switch (pg::Renderer::GetAPI())
	{
	case pg::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pg::RendererAPI::API::DirectX11:  return std::make_shared<pg::Dx11IndexBuffer>(indices, size);
	case pg::RendererAPI::API::Testing:  return std::make_shared<pg::TestingIndexBuffer>(indices, size);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}