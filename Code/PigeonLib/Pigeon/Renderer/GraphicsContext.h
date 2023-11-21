#pragma once

namespace pigeon {

	class GraphicsContext
	{
	public:
		virtual void Shutdown() = 0;
		virtual void Init() = 0;
		virtual void Begin() = 0;
		virtual void SwapBuffers() = 0;
	};

}