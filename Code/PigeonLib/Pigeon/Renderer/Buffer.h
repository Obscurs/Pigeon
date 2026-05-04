#pragma once

namespace pig
{
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static pig::S_Ptr<VertexBuffer> Create(const float* vertices, uint32_t size, uint32_t stride);

		virtual void SetVertices(const float* vertices, unsigned int count, unsigned int countOffset) = 0;
		virtual void AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset) = 0;
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static pig::S_Ptr<IndexBuffer> Create(const uint32_t* indices, uint32_t size);

		virtual void SetIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) = 0;
		virtual void AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) = 0;
	};

}