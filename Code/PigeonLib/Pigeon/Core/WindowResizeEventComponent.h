#pragma once

namespace pg
{
	struct WindowResizeEventComponent
	{
		WindowResizeEventComponent() {};
		WindowResizeEventComponent(const WindowResizeEventComponent&) = default;

		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
	};
}