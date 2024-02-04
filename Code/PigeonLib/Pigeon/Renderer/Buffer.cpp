#include "pch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/DirectX11/Dx11Buffer.h"

pig::U_Ptr<pig::VertexBuffer> pig::VertexBuffer::Create(const float* vertices, uint32_t size, uint32_t stride)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_unique<pig::Dx11VertexBuffer>(vertices, size, stride);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}

pig::U_Ptr<pig::IndexBuffer> pig::IndexBuffer::Create(const uint32_t* indices, uint32_t size)
{
	switch (pig::Renderer::GetAPI())
	{
	case pig::RendererAPI::API::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
	case pig::RendererAPI::API::DirectX11:  return std::make_unique<pig::Dx11IndexBuffer>(indices, size);
	}

	PG_CORE_ASSERT(false, "Unknown RendererAPI!");
	return nullptr;
}