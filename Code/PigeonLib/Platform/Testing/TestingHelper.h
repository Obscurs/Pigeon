#pragma once

#include "Pigeon/Core/Core.h"

namespace pig
{
	class TestingIndexBuffer;
	class TestingVertexBuffer;
}
namespace pig
{
	class TestingHelper
	{
	public:
		inline static TestingHelper& GetInstance() { return *s_Instance; }

		static void Reset();

		// VertexBuffer
		struct VertexBufferSetVerticesData
		{
			unsigned int m_Count = 0;
			unsigned int m_CountOffset = 0;
			const pig::TestingVertexBuffer* m_Buffer = nullptr;
		};
		std::vector<const pig::TestingVertexBuffer*> m_VertexBuffer;
		std::vector<const pig::TestingVertexBuffer*> m_VertexBufferBind;
		std::vector<const pig::TestingVertexBuffer*> m_VertexBufferUnbind;
		std::vector<VertexBufferSetVerticesData> m_VertexBufferSetVertices;
		std::vector<VertexBufferSetVerticesData> m_VertexBufferAppendVertices;

		// IndexBuffer
		struct IndexBufferSetVerticesData
		{
			unsigned int m_Count = 0;
			unsigned int m_CountOffset = 0;
			const pig::TestingIndexBuffer* m_Buffer = nullptr;
		};
		std::vector<const pig::TestingIndexBuffer*> m_IndexBuffer;
		std::vector<const pig::TestingIndexBuffer*> m_IndexBufferBind;
		std::vector<const pig::TestingIndexBuffer*> m_IndexBufferUnbind;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferSetIndices;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferAppendIndices;

		float m_Vertices[20000];
		uint32_t m_Indices[20000];
	private:
		static pig::S_Ptr<TestingHelper> s_Instance;
	};
}
