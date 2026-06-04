#include "pch.h"
#include "Renderer.h"

void pg::Renderer::Init()
{
	pg::RenderCommand::Init();
}

void pg::Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
	pg::RenderCommand::SetViewport(0, 0, width, height);
}

void pg::Renderer::BeginScene()
{
	pg::RenderCommand::Begin();
}

void pg::Renderer::EndScene()
{
	pg::RenderCommand::End();
}

void pg::Renderer::Submit(unsigned int count)
{
	pg::RenderCommand::DrawIndexed(count);
}