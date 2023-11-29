#pragma once

#include "Pigeon/Renderer/Buffer.h"

#include <d3d11.h>
namespace pigeon 
{
	class Dx11VertexBuffer : public VertexBuffer
	{
	public:
		Dx11VertexBuffer(float* vertices, uint32_t size);
		virtual ~Dx11VertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		ID3D11Buffer* m_Buffer = nullptr;

	};

	class Dx11IndexBuffer : public IndexBuffer
	{
	public:
		Dx11IndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~Dx11IndexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const { return m_Count; }
	private:
		ID3D11Buffer* m_Buffer = nullptr;
		uint32_t m_Count;
	};

}