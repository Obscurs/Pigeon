#pragma once

#ifdef PG_PLATFORM_TEST

#include "Pigeon/Window.h"

namespace pigeon {

	class UTWindow : public Window
	{
	public:
		UTWindow(const WindowProps& props);
		virtual ~UTWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.m_Width; }
		inline unsigned int GetHeight() const override { return m_Data.m_Height; }

		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override {};
		bool IsVSync() const override { return false; };

		void SendFakeWindowCloseEvent();
	
	private:
		struct WindowData
		{
			const char* m_Title = "Test Window";
			unsigned int m_Width, m_Height;

			EventCallbackFn EventCallback;
		};

		static WindowData m_Data;
	};
}
#endif