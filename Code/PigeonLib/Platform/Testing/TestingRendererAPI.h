#pragma once

#include "Pigeon/Renderer/RendererAPI.h"

namespace pig 
{
	class TestingRendererAPI : public RendererAPI
	{
	public:
		virtual void SetClearColor(const glm::vec4& color) override;

		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void Begin() override;
		virtual void End() override;
		virtual void Clear() override;

		virtual void DrawIndexed(unsigned int count) override;
	};
}