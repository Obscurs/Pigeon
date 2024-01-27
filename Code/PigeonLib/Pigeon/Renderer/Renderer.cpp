#include "pch.h"
#include "Renderer.h"

void pig::Renderer::Init()
{
	pig::RenderCommand::Init();
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