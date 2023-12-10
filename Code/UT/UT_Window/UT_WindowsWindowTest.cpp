#pragma once
#include <catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"
namespace
{
	bool s_EventReceived = false;
	void TestOnEvent(pigeon::Event& e)
	{
		if(e.GetEventType() == pigeon::EventType::KeyPressed)
			s_EventReceived = true;
	}
}
namespace CatchTestsetFail
{
	TEST_CASE("app.Window::WindowsWindowTest")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());
		pigeon::WindowsWindow& window = static_cast<pigeon::WindowsWindow&>(app->GetWindow());
		CHECK(window.GetNativeWindow() != nullptr);
		CHECK(window.GetGraphicsContext() != nullptr);
		CHECK(!window.IsVSync());
		CHECK(window.GetHeight() == 720);
		CHECK(window.GetWidth() == 1280);

		window.SetSize(100, 200);

		//SetSize not applied since it will be done only when renderer frame begins
		CHECK(window.GetHeight() == 720);
		CHECK(window.GetWidth() == 1280);
		
		CHECK(!s_EventReceived);
		window.SetEventCallback(TestOnEvent);
		window.OnUpdate();
		CHECK(!s_EventReceived);

		int wParam = 123; //Arbitrary value, not needed
		int lParam = 456; //Arbitrary value, not needed
		app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
		CHECK(s_EventReceived);

		CHECK(window.GetHeight() == 720);
		CHECK(window.GetWidth() == 1280);

		window.Shutdown();
		CHECK(window.GetNativeWindow() == nullptr);
		CHECK(window.GetGraphicsContext() == nullptr);

		delete app;
	}
} // End namespace: CatchTestsetFail

