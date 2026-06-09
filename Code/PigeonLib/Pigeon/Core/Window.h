#pragma once

#include "pch.h"

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Events/Event.h"

namespace pg {

	struct WindowProps
	{
		std::string m_Title;
		unsigned int m_Width;
		unsigned int m_Height;

		WindowProps(const std::string& title = "Pigeon Engine",
			        unsigned int width = 1280,
			        unsigned int height = 720)
			: m_Title(title), m_Width(width), m_Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class PIGEON_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		struct Data
		{
			const char* m_Title = "Windows Window";
			EventCallbackFn m_EventCallback;
		};

		virtual ~Window() {}

		virtual void OnUpdate() = 0;
		virtual void Shutdown() = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Resizes the live window and switches its display mode. Fullscreen is borderless windowed
		// (a borderless window sized to the primary monitor); width/height are the windowed client size.
		// The resulting OS resize event drives the swapchain/renderer resize through the normal path.
		virtual void ApplyWindowConfig(unsigned int width, unsigned int height, EWindowMode mode) = 0;

		// Window attributes

		inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.m_EventCallback = callback; }
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual void* GetGraphicsContext() const = 0;

		static pg::S_Ptr<Window> Create(const WindowProps& props = WindowProps());
	protected:
		static Data m_Data;
	};

}