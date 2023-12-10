#pragma once

namespace pigeon
{
	class WindowsWindow;
}
namespace pigeon 
{
	class GraphicsContext
	{
	public:
		static GraphicsContext* Create(const WindowsWindow* window);

		virtual void Shutdown() = 0;
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual unsigned int GetWidth() = 0;
		virtual unsigned int GetHeight() = 0;
		virtual void SetSize(unsigned int width, unsigned int height) = 0;
	};

}