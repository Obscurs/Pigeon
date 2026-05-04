#pragma once

#include "Pigeon/Renderer/GraphicsContext.h"


namespace pig 
{
	class TestingContext : public GraphicsContext
	{
	public:
		TestingContext(HWND windowHandle);

		virtual void Init() override;
		virtual void Shutdown() override;
		virtual void SwapBuffers() override;
		virtual void SetSize(unsigned int width, unsigned int height) override;

		virtual unsigned int GetWidth() override { return 0; }
		virtual unsigned int GetHeight() override { return 0; }
	};
}