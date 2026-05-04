#include "pch.h"
#include "Renderer.h"

void pig::Renderer::Init()
{
	pig::RenderCommand::Init();
}

void pig::Renderer::OnWindowResize(uint32_t width, uint32_t height)
{
	pig::RenderCommand::SetViewport(0, 0, width, height);
}

void pig::Renderer::BeginScene()
{
	pig::RenderCommand::Begin();
}

void pig::Renderer::EndScene()
{
	pig::RenderCommand::End();
}

void pig::Renderer::Submit(unsigned int count)
{
	pig::RenderCommand::DrawIndexed(count);
}