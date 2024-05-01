#pragma once

#include "pch.h"

#include "Pigeon/Core/Core.h"
#include "Pigeon/Events/Event.h"

namespace pig {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps(const std::string& title = "Pigeon Engine",
			        unsigned int width = 1280,
			        unsigned int height = 720)
			: Title(title), Width(width), Height(height)
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
			EventCallbackFn EventCallback;
		};

		virtual ~Window() {}

		virtual void OnUpdate() = 0;
		virtual void Shutdown() = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes

		inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual void* GetGraphicsContext() const = 0;

		static pig::S_Ptr<Window> Create(const WindowProps& props = WindowProps());
	protected:
		static Data m_Data;
	};

}