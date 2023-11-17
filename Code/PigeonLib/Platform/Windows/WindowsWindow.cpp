#include "pch.h"
#include "WindowsWindow.h"

#include <d3d11.h>
#include <tchar.h>

#ifndef PG_PLATFORM_TEST

#include "imgui/imgui.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/MouseEvent.h"
#include "Pigeon/Events/KeyEvent.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace pigeon {
	
	static bool s_WindowInitialized = false;

	WindowsWindow::WindowData WindowsWindow::m_Data;

	void WindowsWindow::CleanupDeviceD3D()
	{
		CleanupRenderTarget();
		if (m_Data.m_PSwapChain) { m_Data.m_PSwapChain->Release(); m_Data.m_PSwapChain = nullptr; }
		if (m_Data.m_Pd3dDeviceContext) { m_Data.m_Pd3dDeviceContext->Release(); m_Data.m_Pd3dDeviceContext = nullptr; }
		if (m_Data.m_Pd3dDevice) { m_Data.m_Pd3dDevice->Release(); m_Data.m_Pd3dDevice = nullptr; }
	}

	void WindowsWindow::CreateRenderTarget()
	{
		ID3D11Texture2D* pBackBuffer;
		m_Data.m_PSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		m_Data.m_Pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_Data.m_MainRenderTargetView);
		pBackBuffer->Release();
	}

	void WindowsWindow::CleanupRenderTarget()
	{
		if (m_Data.m_MainRenderTargetView) { m_Data.m_MainRenderTargetView->Release(); m_Data.m_MainRenderTargetView = nullptr; }
	}

	bool WindowsWindow::CreateDeviceD3D(HWND hWnd)
	{
		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT createDeviceFlags = 0;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_Data.m_PSwapChain, &m_Data.m_Pd3dDevice, &featureLevel, &m_Data.m_Pd3dDeviceContext);
		if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
			res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_Data.m_PSwapChain, &m_Data.m_Pd3dDevice, &featureLevel, &m_Data.m_Pd3dDeviceContext);
		if (res != S_OK)
			return false;

		CreateRenderTarget();
		return true;
	}

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

		// Initialize Direct3D
		if (!CreateDeviceD3D(m_Data.m_HWnd))
		{
			CleanupDeviceD3D();
			UnregisterClass(m_Data.m_Title, m_Data.m_HInstance);
			return;
		}

		PG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		ShowWindow(m_Data.m_HWnd, SW_SHOWDEFAULT);
		UpdateWindow(m_Data.m_HWnd);

		if (!s_WindowInitialized)
		{
			PG_CORE_ASSERT(success, "Could not intialize GLFW!");
			s_WindowInitialized = true;
		}
	}

	void WindowsWindow::Shutdown()
	{
		CleanupDeviceD3D();
		UnregisterClass(m_Data.m_Title, m_Data.m_HInstance);
		DestroyWindow(m_Data.m_HWnd);
	}

	void WindowsWindow::OnPreUpdate()
	{
		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (m_Data.m_ResizeWidth != 0 && m_Data.m_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			m_Data.m_PSwapChain->ResizeBuffers(0, m_Data.m_ResizeWidth, m_Data.m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			m_Data.m_ResizeWidth = m_Data.m_ResizeHeight = 0;
			CreateRenderTarget();
		}

		const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
		m_Data.m_Pd3dDeviceContext->OMSetRenderTargets(1, &m_Data.m_MainRenderTargetView, nullptr);
		m_Data.m_Pd3dDeviceContext->ClearRenderTargetView(m_Data.m_MainRenderTargetView, clear_color_with_alpha);
	}

	void WindowsWindow::OnUpdate()
	{
		ProcessMessages();
	}
	void WindowsWindow::OnPostUpdate()
	{
		m_Data.m_PSwapChain->Present(1, 0);
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