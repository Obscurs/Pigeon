#include "pch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/DirectX11/Dx11Buffer.h"

pig::VertexBuffer* pig::VertexBuffer::Create(float* vertices, uint32_t size)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return new pig::Dx11VertexBuffer(vertices, size);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pig::IndexBuffer* pig::IndexBuffer::Create(uint32_t* indices, uint32_t size)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return new pig::Dx11IndexBuffer(indices, size);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}