#pragma once

#ifndef PG_PLATFORM_TEST
#include "Pigeon/Window.h"
#include "Pigeon/Renderer/GraphicsContext.h"

namespace pigeon {

	class WindowsWindow : public Window
	{
	public:
		struct WindowData
		{
			const char* m_Title = "Windows Window";
			unsigned int m_Width, m_Height;
			HINSTANCE m_HInstance;
			EventCallbackFn EventCallback;
		};

		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnBegin() override;
		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.m_Width; }
		inline unsigned int GetHeight() const override { return m_Data.m_Height; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		inline virtual void* GetNativeWindow() const { return m_Window; }
		inline virtual void* GetGraphicsContext() const { return m_Context; }

		static std::optional<int> ProcessMessages();
		
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		static LRESULT __stdcall WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM hParam);
		
		GraphicsContext* m_Context;
		HWND m_Window;

		static WindowData m_Data;
	};
}
#endif