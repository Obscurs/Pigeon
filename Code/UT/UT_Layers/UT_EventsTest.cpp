#pragma once
#include <catch.hpp>
#include <cstdlib>
#include <variant>

#include "Utils/TestApp.h"

#include <Pigeon/Events/KeyEvent.h>
#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>

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

	template<typename T>
	T GetEventValue(const ExpectedEventResult& eventResult)
	{
		REQUIRE(std::holds_alternative<T>(eventResult));
		return std::get<T>(eventResult);
	}
	
	ExpectedEventResult StoreToVariant(pigeon::Event& ev)
	{
		if (auto e = dynamic_cast<pigeon::KeyPressedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::KeyReleasedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::KeyTypedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::WindowResizeEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::WindowCloseEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseMovedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseScrolledEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseButtonPressedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<pigeon::MouseButtonReleasedEvent*>(&ev)) {
			return *e;
		}
		else
		{
			return false;
			CHECK(false); //type not implemented?
		}
	}

	class TestLayer : public pigeon::Layer
	{
	public:
		TestLayer()
			: Layer("UTLayerEventPropagate")
		{
		}
		void OnUpdate(pigeon::Timestep ts) override
		{
			m_ExpectedEvent = false;
		}

		void OnEvent(pigeon::Event& event) override
		{
			m_ExpectedEvent = StoreToVariant(event);

		}
		ExpectedEventResult m_ExpectedEvent = false;
	};
}

namespace CatchTestsetFail
{
	TEST_CASE("app.Layers::EventsTest")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());

		TestLayer* testLayer = new TestLayer();
		app->PushLayer(testLayer);

		int wParam = 123;
		int lParam = 456;
		SECTION("Key pressed events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			pigeon::KeyPressedEvent event = GetEventValue<pigeon::KeyPressedEvent>(testLayer->m_ExpectedEvent);
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
			event = GetEventValue<pigeon::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(!event.GetRepeatCount());

			wParam = 678;
			lParam = 0x40000000;
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			event = GetEventValue<pigeon::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.GetRepeatCount());
		}
		SECTION("Key released events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYUP, wParam, lParam);
			pigeon::KeyReleasedEvent event = GetEventValue<pigeon::KeyReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyReleased);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Key char events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::CHAR, wParam, lParam);
			pigeon::KeyTypedEvent event = GetEventValue<pigeon::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::KeyTyped);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse scroll events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEWHEEL, wParam, lParam);
			pigeon::MouseScrolledEvent event = GetEventValue<pigeon::MouseScrolledEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseScrolled);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse move events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMOVE, wParam, lParam);
			pigeon::MouseMovedEvent event = GetEventValue<pigeon::MouseMovedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseMoved);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse button down events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			pigeon::MouseButtonPressedEvent event = GetEventValue<pigeon::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonPressed);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSERBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mouse button up events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pigeon::MouseButtonReleasedEvent event = GetEventValue<pigeon::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSERBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSEMBUTTONUP, wParam, lParam);
			event = GetEventValue<pigeon::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mix Events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pigeon::MouseButtonReleasedEvent event = GetEventValue<pigeon::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::CHAR, wParam, lParam);
			pigeon::KeyTypedEvent event2 = GetEventValue<pigeon::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event2.GetEventType() == pigeon::EventType::KeyTyped);
			CHECK(event2.GetKeyCode() == wParam);
			CHECK(event2.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(event2.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event2.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application close Events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::DESTROY, wParam, lParam);
			pigeon::WindowCloseEvent event = GetEventValue<pigeon::WindowCloseEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pigeon::EventType::WindowClose);
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryKeyboard));
			CHECK(event.IsInCategory(pigeon::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pigeon::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application resize Events")
		{
			app->SendFakeEvent(pigeon::WindowsWindow::EventType::SIZE, wParam, lParam);
			pigeon::WindowResizeEvent event = GetEventValue<pigeon::WindowResizeEvent>(testLayer->m_ExpectedEvent);
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

