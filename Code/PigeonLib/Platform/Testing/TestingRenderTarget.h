#pragma once

#include "Pigeon/Renderer/RenderTarget.h"

namespace pg
{
	class TestingRenderTarget : public RenderTarget
	{
	public:
		TestingRenderTarget(uint32_t width, uint32_t height);
		virtual ~TestingRenderTarget() = default;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual pg::S_Ptr<pg::Texture2D> GetColorTexture() const override { return m_ColorTexture; }

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		pg::S_Ptr<pg::Texture2D> m_ColorTexture;
	};
}
