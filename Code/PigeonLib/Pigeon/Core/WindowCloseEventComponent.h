#pragma once

namespace pg
{
	struct WindowCloseEventComponent
	{
		WindowCloseEventComponent() {};
		WindowCloseEventComponent(const WindowCloseEventComponent&) = default;

		bool m_Dummy = false;
	};
}