#include "pch.h"
#include "WindowsWindow.h"

#ifndef PG_PLATFORM_TEST

#include "imgui/imgui.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/MouseEvent.h"
#include "Pigeon/Events/KeyEvent.h"

#include "Platform/DirectX11/Dx11Context.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace pigeon {
	
	static bool s_WindowInitialized = false;

	WindowsWindow::WindowData WindowsWindow::m_Data;

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

		m_Window = CreateWindow(
			m_Data.m_Title, m_Data.m_Title,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top,
			nullptr, nullptr, m_Data.m_HInstance, this
		);

		m_Context = new Dx11Context(m_Window);
		m_Context->Init();

		PG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		ShowWindow(m_Window, SW_SHOWDEFAULT);
		UpdateWindow(m_Window);

		if (!s_WindowInitialized)
		{
			PG_CORE_ASSERT(success, "Could not intialize GLFW!");
			s_WindowInitialized = true;
		}
	}

	void WindowsWindow::Shutdown()
	{
		m_Context->Shutdown();
		UnregisterClass(m_Data.m_Title, m_Data.m_HInstance);
		DestroyWindow(m_Window);
	}

	void WindowsWindow::OnBegin()
	{
		m_Context->Begin();
	}

	void WindowsWindow::OnUpdate()
	{
		ProcessMessages();
		m_Context->SwapBuffers();
	}

	std::optional<int> WindowsWindow::ProcessMessages()
	{
		MSG msg = {};
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				return 0;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return {};
	}

	LRESULT __stdcall WindowsWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
			case WM_DESTROY:
			{
				WindowCloseEvent event;
				m_Data.EventCallback(event);
				PostQuitMessage(0);
				return 0;
			}
			case WM_SIZE: {
				// The LOWORD and HIWORD macros extract the width and height
				unsigned int width = LOWORD(lParam);
				unsigned int height = HIWORD(lParam);

				// Here you can update the internal data if you store the dimensions
				m_Data.m_Width = width;
				m_Data.m_Height = height;

				// Now create a WindowResizeEvent and dispatch it
				WindowResizeEvent event(width, height);
				if (m_Data.EventCallback) {
					m_Data.EventCallback(event);
				}
				break;
			}
			case WM_MOUSEMOVE: 
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				// Do something with x and y, like creating a mouse event and dispatching it.
				MouseMovedEvent event((float)x, (float)y);
				m_Data.EventCallback(event);
				break;
			}
			case WM_MOUSEWHEEL: 
			{
				float yOffset = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
				// Since there's no horizontal scrolling in this message, the xOffset is 0
				float xOffset = 0.0f;

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				m_Data.EventCallback(event);
				break;
			}

			case WM_LBUTTONDOWN:
			{
				MouseButtonPressedEvent event(0);
				m_Data.EventCallback(event);
				break;
			}
			case WM_RBUTTONDOWN:
			{
				MouseButtonPressedEvent event(1);
				m_Data.EventCallback(event);
				break;
			}
			case WM_MBUTTONDOWN:
			{
				MouseButtonPressedEvent event(2);
				m_Data.EventCallback(event);
				break;
			}
			case WM_LBUTTONUP:
			{
				MouseButtonReleasedEvent event(0);
				m_Data.EventCallback(event);
				break;
			}
			case WM_RBUTTONUP:
			{
				MouseButtonReleasedEvent event(1);
				m_Data.EventCallback(event);
				break;
			}
			case WM_MBUTTONUP: 
			{
				MouseButtonReleasedEvent event(2);
				m_Data.EventCallback(event);
				break;
			}

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN: 
			{ // SYSKEYDOWN is used for ALT key combinations
				bool isRepeat = (lParam & 0x40000000) != 0;
				if (!isRepeat) {
					// Key has been pressed (not a repeat)
					KeyPressedEvent event(static_cast<int>(wParam), 0);
					m_Data.EventCallback(event);
				}
				else
				{
					KeyPressedEvent event(static_cast<int>(wParam), 1);
					m_Data.EventCallback(event);
				}
				break;
			}

			case WM_CHAR:
			{
				KeyTypedEvent event(static_cast<int>(wParam));
				m_Data.EventCallback(event);
				break;
			}

			case WM_KEYUP:
			case WM_SYSKEYUP: 
			{
				KeyReleasedEvent event(static_cast<int>(wParam));
				m_Data.EventCallback(event);
				break;
			}
		}
		
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
#endif