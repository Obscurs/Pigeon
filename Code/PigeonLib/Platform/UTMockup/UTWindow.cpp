#include "pch.h"
#include "UTWindow.h"

#ifdef PG_PLATFORM_TEST
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/MouseEvent.h"
#include "Pigeon/Events/KeyEvent.h"

namespace pigeon {
	
	static bool s_WindowInitialized = false;

	UTWindow::WindowData UTWindow::m_Data;

	Window* Window::Create(const WindowProps& props)
	{
		return new UTWindow(props);
	}

	UTWindow::UTWindow(const WindowProps& props)
	{
		m_Data.m_Title = props.Title.c_str();
		m_Data.m_Width = props.Width;
		m_Data.m_Height = props.Height;
	}

	UTWindow::~UTWindow()
	{
	}

	void UTWindow::OnUpdate()
	{
	}

	void UTWindow::SendFakeWindowCloseEvent()
	{
		WindowCloseEvent event;
		m_Data.EventCallback(event);
	}
}
#endif