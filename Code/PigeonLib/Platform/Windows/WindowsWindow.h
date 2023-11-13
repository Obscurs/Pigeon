#pragma once

#ifndef PG_PLATFORM_TEST
#include "Pigeon/Window.h"
#include <d3d11.h>
#include <tchar.h>

namespace pigeon {

	class WindowsWindow : public Window
	{
	public:
		struct WindowData
		{
			const char* m_Title = "DirectX Window";
			unsigned int m_Width, m_Height;
			HWND m_HWnd;
			HINSTANCE m_HInstance;
			ID3D11Device* g_pd3dDevice = nullptr;
			ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
			IDXGISwapChain* g_pSwapChain = nullptr;
			UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
			ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

			EventCallbackFn EventCallback;
		};

		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnPreUpdate() override;
		void OnUpdate() override;
		void OnPostUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.m_Width; }
		inline unsigned int GetHeight() const override { return m_Data.m_Height; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		static std::optional<int> ProcessMessages();
		
		static const WindowData& GetWindowData() { return m_Data; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		static LRESULT __stdcall WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM hParam);
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
	
		static WindowData m_Data;
	};
}
#endif