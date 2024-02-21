#pragma once

#include "Pigeon/Core.h"

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
			const float* m_Vertices;
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
			const uint32_t* m_Indices;
			unsigned int m_Count = 0;
			unsigned int m_CountOffset = 0;
			const pig::TestingIndexBuffer* m_Buffer = nullptr;
		};
		std::vector<const pig::TestingIndexBuffer*> m_IndexBuffer;
		std::vector<const pig::TestingIndexBuffer*> m_IndexBufferBind;
		std::vector<const pig::TestingIndexBuffer*> m_IndexBufferUnbind;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferSetIndices;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferAppendIndices;

	private:
		static pig::S_Ptr<TestingHelper> s_Instance;
	};
}
