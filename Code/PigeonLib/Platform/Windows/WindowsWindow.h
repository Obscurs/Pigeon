#pragma once

#include "Pigeon/Window.h"
#include "Pigeon/Renderer/GraphicsContext.h"

namespace pigeon 
{
	class WindowsWindow : public Window
	{
	public:
		struct WindowData
		{
			const char* m_Title = "Windows Window";
			HINSTANCE m_HInstance;
			EventCallbackFn EventCallback;
		};

		enum class EventType
		{
			DESTROY,
			SIZE,
			MOUSEMOVE,
			MOUSEWHEEL,
			MOUSELBUTTONDOWN,
			MOUSERBUTTONDOWN,
			MOUSEMBUTTONDOWN,
			MOUSELBUTTONUP,
			MOUSERBUTTONUP,
			MOUSEMBUTTONUP,
			KEYDOWN,
			KEYUP,
			CHAR,
		};

		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void SetSize(unsigned int width, unsigned int height) override;
		void OnUpdate() override;

		void Shutdown() override;
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_WindowData.EventCallback = callback; }
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		inline virtual void* GetNativeWindow() const { return m_Window; }
		inline virtual void* GetGraphicsContext() const { return m_Context; }

#ifdef TESTS_ENABLED
		static void SendFakeEvent(EventType type, WPARAM wParam, LPARAM lParam);
#endif

	private:
		static std::optional<int> ProcessMessages();

		virtual void Init(const WindowProps& props);
		
		static LRESULT __stdcall WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static bool ProcessEvent(UINT msg, WPARAM wParam, LPARAM lParam);
		static void ProcessCharPressedEvent(WPARAM wParam);
		static void ProcessKeyUpEvent(WPARAM wParam);
		static void ProcessKeyDownEvent(LPARAM lParam, WPARAM wParam);
		static void ProcessMouseButtonEvent(UINT msg);
		static void ProcessMouseWheelEvent(WPARAM wParam);
		static void ProcessMouseMoveEvent(LPARAM lParam);
		static void ProcessWindowResizeEvent(LPARAM lParam);
		static bool ProcessWindowDestroyEvent();

		GraphicsContext* m_Context = nullptr;
		HWND m_Window = nullptr;

		static WindowData m_WindowData;
	};
}