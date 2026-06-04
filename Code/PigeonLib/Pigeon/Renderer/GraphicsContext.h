#pragma once

namespace pg
{
	class WindowsWindow;
}
namespace pg 
{
	class GraphicsContext
	{
	public:
		static pg::S_Ptr<GraphicsContext> Create(const WindowsWindow* window);

		virtual void Shutdown() = 0;
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;
		virtual void SetSize(unsigned int width, unsigned int height) = 0;
	};

}