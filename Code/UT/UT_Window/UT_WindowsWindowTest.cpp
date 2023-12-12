#pragma once
#include <catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"
namespace
{
	bool s_EventReceived = false;
	void TestOnEvent(pig::Event& e)
	{
		if(e.GetEventType() == pig::EventType::KeyPressed)
			s_EventReceived = true;
	}
}
namespace CatchTestsetFail
{
	TEST_CASE("app.Window::WindowsWindowTest")
	{
		pig::S_Ptr<pig::Application> app = pig::CreateApplication();
		pig::WindowsWindow& window = static_cast<pig::WindowsWindow&>(app->GetWindow());
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
		window.SendFakeEvent(pig::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
		CHECK(s_EventReceived);

		CHECK(window.GetHeight() == 720);
		CHECK(window.GetWidth() == 1280);

		window.Shutdown();
		CHECK(window.GetNativeWindow() == nullptr);
		CHECK(window.GetGraphicsContext() == nullptr);
	}
} // End namespace: CatchTestsetFail

