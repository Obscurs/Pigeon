#include "pch.h"
#include "Platform/Testing/TestingRendererAPI.h"

#include "Platform/Testing/TestingHelper.h"

void pg::TestingRendererAPI::Init()
{
   
}

void pg::TestingRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	
}

void pg::TestingRendererAPI::SetClearColor(const glm::vec4& color)
{
	
}

void pg::TestingRendererAPI::Begin()
{
	pg::TestingHelper::GetInstance().m_VertexBufferSetVertices.clear();
	pg::TestingHelper::GetInstance().m_VertexBufferAppendVertices.clear();
	pg::TestingHelper::GetInstance().m_VertexBuffer.clear();
	pg::TestingHelper::GetInstance().m_VertexBufferBind.clear();
	pg::TestingHelper::GetInstance().m_VertexBufferUnbind.clear();

	pg::TestingHelper::GetInstance().m_IndexBufferSetIndices.clear();
	pg::TestingHelper::GetInstance().m_IndexBufferAppendIndices.clear();
	pg::TestingHelper::GetInstance().m_IndexBuffer.clear();
	pg::TestingHelper::GetInstance().m_IndexBufferBind.clear();
	pg::TestingHelper::GetInstance().m_IndexBufferUnbind.clear();
}

void pg::TestingRendererAPI::End()
{

}

void pg::TestingRendererAPI::Clear()
{
	
}

void pg::TestingRendererAPI::DrawIndexed(unsigned int count)
{
	
}