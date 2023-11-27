#pragma once
#include <catch.hpp>
#include <cstdlib>
#include <variant>

#include <Pigeon.h>
#include <Pigeon/Events/KeyEvent.h>
#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>
#include <Platform/Windows/WindowsWindow.h>

namespace
{
	using ExpectedEventResult = std::variant<
		pigeon::KeyPressedEvent,
		pigeon::KeyReleasedEvent,
		pigeon::KeyTypedEvent,
		pigeon::WindowResizeEvent,
		pigeon::WindowCloseEvent,
		pigeon::MouseMovedEvent,
		pigeon::MouseScrolledEvent,
		pigeon::MouseButtonPressedEvent,
		pigeon::MouseButtonReleasedEvent,
		bool
	>;

	ExpectedEventResult s_ExpectedEvent = false;

	template<typename T>
	T GetEventValue(const ExpectedEventResult& eventResult)
	{
		REQUIRE(std::holds_alternative<T>(eventResult));
		return std::get<T>(eventResult);
	}
}

class TestLayer : public pigeon::Layer
{
public:
	TestLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		if (pigeon::Input::IsKeyPressed(PG_KEY_TAB))
			PG_TRACE("Tab key is pressed (poll)!");
	}

	virtual void OnImGuiRender() override
	{
	}

	void OnEvent(pigeon::Event& event) override
	{
		s_ExpectedEvent = false;

		if (auto e = dynamic_cast<pigeon::KeyPressedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::KeyReleasedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::KeyTypedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::WindowResizeEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::WindowCloseEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseMovedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseScrolledEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseButtonPressedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseButtonReleasedEvent*>(&event)) {
			s_ExpectedEvent = *e;
		}
		else
		{
			s_ExpectedEvent = false;
			CHECK(false); //type not implemented?
		}
	}
	
};

class TestApp : public pigeon::Application
{
public:
	TestApp()
	{
		PushLayer(new TestLayer());
	}

	~TestApp()
	{

	}

	void TestApp::SendFakeEvent(pigeon::WindowsWindow::EventType type, WPARAM wParam, LPARAM lParam)
	{
		static_cast<pigeon::WindowsWindow&>(GetWindow()).SendFakeEvent(type, wParam, lParam);
	}

};

pigeon::Application* pigeon::CreateApplication()
{
	pigeon::Log::Init();
	return new TestApp();
}

namespace CatchTestsetFail
{
	TEST_CASE("EventsTest")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());
		app->TestUpdate();

		SECTION("Key pressed events")
		{
			int wParam = 555;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			pigeon::KeyPressedEvent event = GetEventValue<pigeon::KeyPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(!event.GetRepeatCount());
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));

			wParam = 123;
			lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			event = GetEventValue<pigeon::KeyPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(!event.GetRepeatCount());

			wParam = 678;
			lParam = 0x40000000;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			event = GetEventValue<pigeon::KeyPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.GetRepeatCount());
		}
		SECTION("Key released events")
		{
			int wParam = 555;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYUP, wParam, lParam);
			pigeon::KeyReleasedEvent event = GetEventValue<pigeon::KeyReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyReleased);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));

			wParam = 123;
			lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYUP, wParam, lParam);
			event = GetEventValue<pigeon::KeyReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyReleased);
			CHECK(event.GetKeyCode() == wParam);
		}
		SECTION("Key char events")
		{
			int wParam = 555;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::CHAR, wParam, lParam);
			pigeon::KeyTypedEvent event = GetEventValue<pigeon::KeyTypedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyTyped);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));

			wParam = 123;
			lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::CHAR, wParam, lParam);
			REQUIRE(std::holds_alternative<pigeon::KeyTypedEvent>(s_ExpectedEvent));
			event = GetEventValue<pigeon::KeyTypedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyTyped);
			CHECK(event.GetKeyCode() == wParam);
		}
		SECTION("Mouse scroll events")
		{
			int wParam = 12341235;
			int lParam = 2;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEWHEEL, wParam, lParam);
			pigeon::MouseScrolledEvent event = GetEventValue<pigeon::MouseScrolledEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseScrolled);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));

			wParam = 123161;
			lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEWHEEL, wParam, lParam);
			event = GetEventValue<pigeon::MouseScrolledEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseScrolled);
		}
		SECTION("Mouse move events")
		{
			int wParam = 12341235;
			int lParam = 2;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMOVE, wParam, lParam);
			pigeon::MouseMovedEvent event = GetEventValue<pigeon::MouseMovedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseMoved);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse button down events")
		{
			int wParam = 0;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			pigeon::MouseButtonPressedEvent event = GetEventValue<pigeon::MouseButtonPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonPressed);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == wParam);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSERBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mouse button up events")
		{
			int wParam = 0;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pigeon::MouseButtonReleasedEvent event = GetEventValue<pigeon::MouseButtonReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == wParam);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSERBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mix Events")
		{
			int wParam = 0;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pigeon::MouseButtonReleasedEvent event = GetEventValue<pigeon::MouseButtonReleasedEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == wParam);

			int wParam2 = 555;
			int lParam2 = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::CHAR, wParam2, lParam2);
			pigeon::KeyTypedEvent event2 = GetEventValue<pigeon::KeyTypedEvent>(s_ExpectedEvent);
			CHECK(event2.GetEventType() == pigeon::EventType::KeyTyped);
			CHECK(event2.GetKeyCode() == wParam2);
			CHECK(event2.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event2.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application close Events")
		{
			int wParam = 0;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::DESTROY, wParam, lParam);
			pigeon::WindowCloseEvent event = GetEventValue<pigeon::WindowCloseEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::WindowClose);
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application resize Events")
		{
			int wParam = 0;
			int lParam = 0;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::SIZE, wParam, lParam);
			pigeon::WindowResizeEvent event = GetEventValue<pigeon::WindowResizeEvent>(s_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::WindowResize);
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		delete app;
	}
} // End namespace: CatchTestsetFail

