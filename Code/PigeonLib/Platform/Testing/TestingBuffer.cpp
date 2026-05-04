#include "pch.h"
#include "TestingBuffer.h"

#include "Platform/Testing/TestingHelper.h"

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::TestingVertexBuffer::TestingVertexBuffer(const float* vertices, uint32_t size, uint32_t stride)
{
	pig::TestingHelper::GetInstance().m_VertexBuffer.push_back(this);
}

void pig::TestingVertexBuffer::Bind() const
{
	pig::TestingHelper::GetInstance().m_VertexBufferBind.push_back(this);
}

void pig::TestingVertexBuffer::Unbind() const
{
	pig::TestingHelper::GetInstance().m_VertexBufferUnbind.push_back(this);
}

void pig::TestingVertexBuffer::SetVertices(const float* vertices, unsigned int count, unsigned int countOffset)
{
	pig::TestingHelper::VertexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pig::TestingHelper::GetInstance().m_Vertices) + countOffset * 10 * sizeof(float);
	memcpy(destData, vertices, count * 10 * sizeof(float));
	pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.push_back(data);
}

void pig::TestingVertexBuffer::AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset)
{
	pig::TestingHelper::VertexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pig::TestingHelper::GetInstance().m_Vertices) + countOffset * 10 * sizeof(float);
	memcpy(destData, vertices, count * 10 * sizeof(float));
	pig::TestingHelper::GetInstance().m_VertexBufferAppendVertices.push_back(data);
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::TestingIndexBuffer::TestingIndexBuffer(const uint32_t* indices, uint32_t count)
{
	pig::TestingHelper::GetInstance().m_IndexBuffer.push_back(this);
}

void pig::TestingIndexBuffer::Bind() const
{
	pig::TestingHelper::GetInstance().m_IndexBufferBind.push_back(this);
}

void pig::TestingIndexBuffer::Unbind() const
{
	pig::TestingHelper::GetInstance().m_IndexBufferUnbind.push_back(this);
}

void pig::TestingIndexBuffer::SetIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset)
{
	pig::TestingHelper::IndexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pig::TestingHelper::GetInstance().m_Indices) + countOffset * sizeof(int);
	memcpy(destData, indices, count * sizeof(int));
	pig::TestingHelper::GetInstance().m_IndexBufferSetIndices.push_back(data);
}
void pig::TestingIndexBuffer::AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset)
{
	pig::TestingHelper::IndexBufferSetVerticesData data;
	data.m_Count = count;
	data.m_CountOffset = countOffset;
	data.m_Buffer = this;
	BYTE* destData = reinterpret_cast<BYTE*>(pig::TestingHelper::GetInstance().m_Indices) + countOffset * sizeof(int);
	memcpy(destData, indices, count * sizeof(int));
	pig::TestingHelper::GetInstance().m_IndexBufferAppendIndices.push_back(data);
}