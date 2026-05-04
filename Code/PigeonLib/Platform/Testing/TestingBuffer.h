#pragma once

#include "Pigeon/Renderer/Buffer.h"

namespace pig 
{
	class TestingVertexBuffer : public VertexBuffer
	{
	public:

		TestingVertexBuffer(const float* vertices, uint32_t size, uint32_t stride);
		virtual ~TestingVertexBuffer() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetVertices(const float* vertices, unsigned int count, unsigned int countOffset) override;
		virtual void AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset) override;
	};

	class TestingIndexBuffer : public IndexBuffer
	{
	public:
		TestingIndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~TestingIndexBuffer() = default;

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) override;
		virtual void AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) override;

		virtual uint32_t GetCount() const { return 0; }
	};

	template<typename T>
	class TestingConstantBuffer
	{
	public:
		TestingConstantBuffer(T* data)
		{
			
		}
		~TestingConstantBuffer() = default;

		void Bind(UINT slot)
		{
		}
	};
}