#pragma once

#include "Pigeon/Renderer/Buffer.h"

#include <d3d11.h>
namespace pigeon 
{
	class Dx11VertexBuffer : public VertexBuffer
	{
	public:
		struct Data
		{
			ID3D11Buffer* m_Buffer = nullptr;
		};

		Dx11VertexBuffer(float* vertices, uint32_t size);
		virtual ~Dx11VertexBuffer();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		Data m_Data;
	};

	class Dx11IndexBuffer : public IndexBuffer
	{
	public:
		struct Data
		{
			ID3D11Buffer* m_Buffer = nullptr;
			uint32_t m_Count;
		};

		Dx11IndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~Dx11IndexBuffer();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const { return m_Data.m_Count; }
	private:
		Data m_Data;
	};

}