#pragma once

#include "Pigeon/Core/Window.h"

namespace pig
{
	class MouseButtonEvent;
	class MouseButtonPressedEvent;
	class MouseButtonReleasedEvent;
	class MouseMovedEvent;
	class MouseScrolledEvent;

	class KeyPressedEvent;
	class KeyReleasedEvent;
	class KeyTypedEvent;

	class WindowCloseEvent;
	class WindowResizeEvent;
}

namespace pig 
{
	class TestingWindow : public Window
	{
	public:
		TestingWindow(const WindowProps& props);
		virtual ~TestingWindow();

		void OnUpdate() override;

		void Shutdown() override;
		unsigned int GetWidth() const override { return 0; };
		unsigned int GetHeight() const override { return 0; };

		// Window attributes
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		inline virtual void* GetNativeWindow() const override { return nullptr; }
		inline virtual void* GetGraphicsContext() const override { return nullptr; }

		void TESTING_TriggerEvent(pig::Event* event);
	};
}