#include "pch.h"
#include "Buffer.h"

#include "Renderer.h"

#include "Platform/DirectX11/Dx11Buffer.h"

namespace pigeon 
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::DirectX11:  return new Dx11VertexBuffer(vertices, size);
		}

		PG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:    PG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::DirectX11:  return new Dx11IndexBuffer(indices, size);
		}

		PG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}