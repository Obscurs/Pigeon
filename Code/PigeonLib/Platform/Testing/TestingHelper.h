#pragma once

#include "Pigeon/Core/Core.h"

namespace pg
{
	class TestingIndexBuffer;
	class TestingVertexBuffer;
}
namespace pg
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
			const pg::TestingVertexBuffer* m_Buffer = nullptr;
		};
		std::vector<const pg::TestingVertexBuffer*> m_VertexBuffer;
		std::vector<const pg::TestingVertexBuffer*> m_VertexBufferBind;
		std::vector<const pg::TestingVertexBuffer*> m_VertexBufferUnbind;
		std::vector<VertexBufferSetVerticesData> m_VertexBufferSetVertices;
		std::vector<VertexBufferSetVerticesData> m_VertexBufferAppendVertices;

		// IndexBuffer
		struct IndexBufferSetVerticesData
		{
			unsigned int m_Count = 0;
			unsigned int m_CountOffset = 0;
			const pg::TestingIndexBuffer* m_Buffer = nullptr;
		};
		std::vector<const pg::TestingIndexBuffer*> m_IndexBuffer;
		std::vector<const pg::TestingIndexBuffer*> m_IndexBufferBind;
		std::vector<const pg::TestingIndexBuffer*> m_IndexBufferUnbind;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferSetIndices;
		std::vector<IndexBufferSetVerticesData> m_IndexBufferAppendIndices;

		// Scissor rects (x, y, width, height) set via the RendererAPI this frame, in submission order.
		struct ScissorData
		{
			int m_X = 0;
			int m_Y = 0;
			int m_Width = 0;
			int m_Height = 0;
		};
		std::vector<ScissorData> m_Scissors;

		float m_Vertices[20000];
		uint32_t m_Indices[20000];
	private:
		static pg::S_Ptr<TestingHelper> s_Instance;
	};
}
