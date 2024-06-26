#include "pch.h"
#include "WindowsWindow.h"

#include <imgui.h>

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/MouseEvent.h"
#include "Pigeon/Events/KeyEvent.h"

#include "Platform/Windows/WindowsInputKeyCodeMapping.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool s_WindowInitialized = false;

#ifndef TESTS_ENABLED
pig::Window::Data pig::Window::m_Data;
pig::S_Ptr<pig::Window> pig::Window::Create(const pig::WindowProps& props)
{
	return std::make_unique<pig::WindowsWindow>(props);
}
#endif
pig::WindowsWindow::WindowsData pig::WindowsWindow::m_WindowsData;

pig::WindowsWindow::WindowsWindow(const pig::WindowProps& props)
{
	m_Data.m_Title = props.Title.c_str();
	m_WindowsData.m_HInstance = GetModuleHandle(nullptr);
	Init(props);
}

pig::WindowsWindow::~WindowsWindow()
{
	Shutdown();
}

void pig::WindowsWindow::Init(const pig::WindowProps& props)
{
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = m_WindowsData.m_HInstance;
	wc.lpszClassName = m_Data.m_Title;

	RegisterClass(&wc);

	RECT winRect;
	winRect.left = 100;
	winRect.right = props.Width + winRect.left;
	winRect.top = 100;
	winRect.bottom = props.Height + winRect.top;
	AdjustWindowRect(&winRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, false);

	m_Window = CreateWindow(
		m_Data.m_Title, m_Data.m_Title,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top,
		nullptr, nullptr, m_WindowsData.m_HInstance, this
	);
	if (!m_Window) {
		DWORD errorCode = GetLastError();
		PG_CORE_ASSERT(false, "error creating window, error code %d", int(errorCode));
		// Now you can use errorCode to understand the issue.
		// You might want to convert it to a human-readable error message.
	}

	m_Context = std::move(pig::GraphicsContext::Create(this));
	m_Context->Init();
	m_Context->SetSize(props.Width, props.Height);

	PG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

	ShowWindow(m_Window, SW_SHOWDEFAULT);
	UpdateWindow(m_Window);

	if (!s_WindowInitialized)
	{
		s_WindowInitialized = true;
	}
}

void pig::WindowsWindow::Shutdown()
{
	if (m_Context)
	{
		m_Context->Shutdown();
		m_Context = nullptr;
	}
	if (m_Window)
	{
		UnregisterClass(m_Data.m_Title, m_WindowsData.m_HInstance);
		DestroyWindow(m_Window);
		m_Window = nullptr;
	}
	m_Data = pig::Window::Data();
	m_WindowsData = pig::WindowsWindow::WindowsData();
}


unsigned int pig::WindowsWindow::GetWidth() const
{
	PG_CORE_ASSERT(m_Context, "context is null");
	return m_Context->GetWidth();
}

unsigned int pig::WindowsWindow::GetHeight() const
{
	PG_CORE_ASSERT(m_Context, "context is null");
	return m_Context->GetHeight();
}

void pig::WindowsWindow::OnUpdate()
{
	ProcessMessages();
	m_Context->SwapBuffers();
}

std::optional<int> pig::WindowsWindow::ProcessMessages()
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

LRESULT __stdcall pig::WindowsWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	if (ProcessEvent(msg, wParam, lParam))
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

bool pig::WindowsWindow::ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
	{ return ProcessWindowDestroyEvent(); }
	case WM_SIZE:
	{ ProcessWindowResizeEvent(lParam); break; }
	case WM_MOUSEMOVE:
	{ ProcessMouseMoveEvent(lParam); break; }
	case WM_MOUSEWHEEL:
	{ ProcessMouseWheelEvent(wParam); break; }
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{ ProcessMouseButtonEvent(msg); break; }
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{ ProcessKeyDownEvent(lParam, wParam); break; }
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{ ProcessKeyUpEvent(wParam); break; }
	case WM_CHAR:
	{ ProcessCharPressedEvent(wParam); break; }
	}
	return true;
}

void pig::WindowsWindow::ProcessCharPressedEvent(WPARAM wParam)
{
	pig::KeyTypedEvent event(static_cast<int>(wParam));
	m_Data.EventCallback(event);
}

void pig::WindowsWindow::ProcessKeyUpEvent(WPARAM wParam)
{
	pig::KeyReleasedEvent event(static_cast<int>(wParam));
	m_Data.EventCallback(event);
}

void pig::WindowsWindow::ProcessKeyDownEvent(LPARAM lParam, WPARAM wParam)
{
	// SYSKEYDOWN is used for ALT key combinations
	bool isRepeat = (lParam & 0x40000000) != 0;
	if (!isRepeat) {
		// Key has been pressed (not a repeat)
		pig::KeyPressedEvent event(static_cast<int>(wParam), 0);
		m_Data.EventCallback(event);
	}
	else
	{
		pig::KeyPressedEvent event(static_cast<int>(wParam), 1);
		m_Data.EventCallback(event);
	}
}

void pig::WindowsWindow::ProcessMouseButtonEvent(UINT msg)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
	{
		pig::MouseButtonPressedEvent event(WIN_PG_MOUSE_BUTTON_LEFT);
		m_Data.EventCallback(event);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		pig::MouseButtonPressedEvent event(WIN_PG_MOUSE_BUTTON_RIGHT);
		m_Data.EventCallback(event);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		pig::MouseButtonPressedEvent event(WIN_PG_MOUSE_BUTTON_MIDDLE);
		m_Data.EventCallback(event);
		break;
	}
	case WM_LBUTTONUP:
	{
		pig::MouseButtonReleasedEvent event(WIN_PG_MOUSE_BUTTON_LEFT);
		m_Data.EventCallback(event);
		break;
	}
	case WM_RBUTTONUP:
	{
		pig::MouseButtonReleasedEvent event(WIN_PG_MOUSE_BUTTON_RIGHT);
		m_Data.EventCallback(event);
		break;
	}
	case WM_MBUTTONUP:
	{
		pig::MouseButtonReleasedEvent event(WIN_PG_MOUSE_BUTTON_MIDDLE);
		m_Data.EventCallback(event);
		break;
	}
	}
}

void pig::WindowsWindow::ProcessMouseWheelEvent(WPARAM wParam)
{
	float yOffset = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);
	// Since there's no horizontal scrolling in this message, the xOffset is 0
	float xOffset = 0.0f;

	pig::MouseScrolledEvent event((float)xOffset, (float)yOffset);
	m_Data.EventCallback(event);
}

void pig::WindowsWindow::ProcessMouseMoveEvent(LPARAM lParam)
{
	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	// Do something with x and y, like creating a mouse event and dispatching it.
	pig::MouseMovedEvent event((float)x, (float)y);
	m_Data.EventCallback(event);
}

void pig::WindowsWindow::ProcessWindowResizeEvent(LPARAM lParam)
{
	// The LOWORD and HIWORD macros extract the width and height
	unsigned int width = LOWORD(lParam);
	unsigned int height = HIWORD(lParam);

	// Now create a WindowResizeEvent and dispatch it
	pig::WindowResizeEvent event(width, height);
	if (m_Data.EventCallback)
		m_Data.EventCallback(event);
}

bool pig::WindowsWindow::ProcessWindowDestroyEvent()
{
	pig::WindowCloseEvent event;
	if (m_Data.EventCallback)
		m_Data.EventCallback(event);
	PostQuitMessage(0);
	return false;
}