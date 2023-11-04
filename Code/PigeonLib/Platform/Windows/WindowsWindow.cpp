#include "pch.h"
#include "WindowsWindow.h"

namespace pigeon {
	
	static bool s_WindowInitialized = false;

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		m_Data.m_Title = props.Title.c_str();
		m_Data.m_Width = props.Width;
		m_Data.m_Height = props.Height;
		m_Data.m_HInstance = GetModuleHandle(nullptr);

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		WNDCLASS wc = {};
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = m_Data.m_HInstance;
		wc.lpszClassName = m_Data.m_Title;

		RegisterClass(&wc);

		RECT winRect;
		winRect.left = 100;
		winRect.right = m_Data.m_Width + winRect.left;
		winRect.top = 100;
		winRect.bottom = m_Data.m_Height + +winRect.top;
		AdjustWindowRect(&winRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, false);

		m_Data.m_HWnd = CreateWindow(
			m_Data.m_Title, m_Data.m_Title,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top,
			nullptr, nullptr, m_Data.m_HInstance, this
		);

		PG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		ShowWindow(m_Data.m_HWnd, SW_SHOWDEFAULT);

		if (!s_WindowInitialized)
		{
			PG_CORE_ASSERT(success, "Could not intialize GLFW!");
			s_WindowInitialized = true;
		}
	}

	void WindowsWindow::Shutdown()
	{
		UnregisterClass(m_Data.m_Title, m_Data.m_HInstance);
		DestroyWindow(m_Data.m_HWnd);
	}

	void WindowsWindow::OnUpdate()
	{
		ProcessMessages();
	}

	std::optional<int> WindowsWindow::ProcessMessages()
	{
		MSG msg = {};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return {};
	}

	LRESULT __stdcall WindowsWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
