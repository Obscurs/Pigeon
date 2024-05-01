#pragma once

#include "Pigeon/Core/Window.h"
#include "Pigeon/Renderer/GraphicsContext.h"

namespace pig 
{
	class WindowsWindow : public Window
	{
	public:
		struct WindowsData
		{
			HINSTANCE m_HInstance;
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

		void OnUpdate() override;

		void Shutdown() override;
		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		// Window attributes
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		inline virtual void* GetNativeWindow() const override { return m_Window; }
		inline virtual void* GetGraphicsContext() const override { return m_Context.get(); }

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

		pig::S_Ptr<GraphicsContext> m_Context = nullptr;
		HWND m_Window = nullptr;

		static WindowsData m_WindowsData;
	};
}