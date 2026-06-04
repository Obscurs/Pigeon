#include "pch.h"
#include "TestingBuffer.h"

#include "Platform/Testing/TestingHelper.h"

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pg::TestingVertexBuffer::TestingVertexBuffer(const float* vertices, uint32_t size, uint32_t stride)
{
	pg::TestingHelper::GetInstance().m_VertexBuffer.push_back(this);
}

void pg::TestingVertexBuffer::Bind() const
{
	pg::TestingHelper::GetInstance().m_VertexBufferBind.push_back(this);
}

void pg::TestingVertexBuffer::Unbind() const
{
	pg::TestingHelper::GetInstance().m_VertexBufferUnbind.push_back(this);
}

void pg::TestingVertexBuffer::SetVertices(const float* vertices, unsigned int count, unsigned int countOffset)
{
	pg::TestingHelper::VertexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pg::TestingHelper::GetInstance().m_Vertices) + countOffset * 10 * sizeof(float);
	memcpy(destData, vertices, count * 10 * sizeof(float));
	pg::TestingHelper::GetInstance().m_VertexBufferSetVertices.push_back(data);
}

void pg::TestingVertexBuffer::AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset)
{
	pg::TestingHelper::VertexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pg::TestingHelper::GetInstance().m_Vertices) + countOffset * 10 * sizeof(float);
	memcpy(destData, vertices, count * 10 * sizeof(float));
	pg::TestingHelper::GetInstance().m_VertexBufferAppendVertices.push_back(data);
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pg::TestingIndexBuffer::TestingIndexBuffer(const uint32_t* indices, uint32_t count)
{
	pg::TestingHelper::GetInstance().m_IndexBuffer.push_back(this);
}

void pg::TestingIndexBuffer::Bind() const
{
	pg::TestingHelper::GetInstance().m_IndexBufferBind.push_back(this);
}

void pg::TestingIndexBuffer::Unbind() const
{
	pg::TestingHelper::GetInstance().m_IndexBufferUnbind.push_back(this);
}

void pg::TestingIndexBuffer::SetIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset)
{
	pg::TestingHelper::IndexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pg::TestingHelper::GetInstance().m_Indices) + countOffset * sizeof(int);
	memcpy(destData, indices, count * sizeof(int));
	pg::TestingHelper::GetInstance().m_IndexBufferSetIndices.push_back(data);
}
void pg::TestingIndexBuffer::AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset)
{
	pg::TestingHelper::IndexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pg::TestingHelper::GetInstance().m_Indices) + countOffset * sizeof(int);
	memcpy(destData, indices, count * sizeof(int));
	pg::TestingHelper::GetInstance().m_IndexBufferAppendIndices.push_back(data);
}