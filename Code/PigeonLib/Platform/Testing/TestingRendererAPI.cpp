#include "pch.h"
#include "TestingRendererAPI.h"

#include <Platform/Testing/TestingHelper.h>

void pig::TestingRendererAPI::Init()
{
   
}

void pig::TestingRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	
}

void pig::TestingRendererAPI::SetClearColor(const glm::vec4& color)
{
	
}

void pig::TestingRendererAPI::Begin()
{
	pig::TestingHelper::GetInstance().m_VertexBufferSetVertices.clear();
	pig::TestingHelper::GetInstance().m_VertexBufferAppendVertices.clear();
	pig::TestingHelper::GetInstance().m_VertexBuffer.clear();
	pig::TestingHelper::GetInstance().m_VertexBufferBind.clear();
	pig::TestingHelper::GetInstance().m_VertexBufferUnbind.clear();

	pig::TestingHelper::GetInstance().m_IndexBufferSetIndices.clear();
	pig::TestingHelper::GetInstance().m_IndexBufferAppendIndices.clear();
	pig::TestingHelper::GetInstance().m_IndexBuffer.clear();
	pig::TestingHelper::GetInstance().m_IndexBufferBind.clear();
	pig::TestingHelper::GetInstance().m_IndexBufferUnbind.clear();
}

void pig::TestingRendererAPI::End()
{

}

void pig::TestingRendererAPI::Clear()
{
	
}

void pig::TestingRendererAPI::DrawIndexed(unsigned int count)
{
	
}