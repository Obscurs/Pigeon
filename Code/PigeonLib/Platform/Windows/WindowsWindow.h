#pragma once

#ifndef PG_PLATFORM_TEST
#include "Pigeon/Window.h"

namespace pigeon {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.m_Width; }
		inline unsigned int GetHeight() const override { return m_Data.m_Height; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		static std::optional<int> ProcessMessages();
		
		static HWND GetWindowHandle() { return m_Data.m_HWnd; }
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		static LRESULT __stdcall WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM hParam);
	
	private:
		struct WindowData
		{
			const char* m_Title = "DirectX Window";
			unsigned int m_Width, m_Height;
			HWND m_HWnd;
			HINSTANCE m_HInstance;

			EventCallbackFn EventCallback;
		};

		static WindowData m_Data;
	};
}
#endif