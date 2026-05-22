#pragma once

namespace pig
{
	struct WindowCloseEventComponent
	{
		WindowCloseEventComponent() {};
		WindowCloseEventComponent(const WindowCloseEventComponent&) = default;

		bool m_Dummy = false;
	};
}