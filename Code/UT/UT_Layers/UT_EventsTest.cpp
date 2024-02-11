#pragma once
#include <catch.hpp>
#include <cstdlib>
#include <variant>

#include <minwindef.h>

#include "Utils/TestApp.h"

#include <Pigeon/Events/KeyEvent.h>
#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>

namespace
{
	using ExpectedEventResult = std::variant<
		pig::KeyPressedEvent,
		pig::KeyReleasedEvent,
		pig::KeyTypedEvent,
		pig::WindowResizeEvent,
		pig::WindowCloseEvent,
		pig::MouseMovedEvent,
		pig::MouseScrolledEvent,
		pig::MouseButtonPressedEvent,
		pig::MouseButtonReleasedEvent,
		bool
	>;

	template<typename T>
	T GetEventValue(const ExpectedEventResult& eventResult)
	{
		REQUIRE(std::holds_alternative<T>(eventResult));
		return std::get<T>(eventResult);
	}
	
	ExpectedEventResult StoreToVariant(const pig::Event& ev)
	{
		if (auto e = dynamic_cast<const pig::KeyPressedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::KeyReleasedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::KeyTypedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::WindowResizeEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::WindowCloseEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::MouseMovedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::MouseScrolledEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::MouseButtonPressedEvent*>(&ev)) {
			return *e;
		}
		else if (auto e = dynamic_cast<const pig::MouseButtonReleasedEvent*>(&ev)) {
			return *e;
		}
		else
		{
			return false;
			CHECK(false); //type not implemented?
		}
	}

	class TestLayer : public pig::Layer
	{
	public:
		TestLayer()
			: Layer("UTLayerEventPropagate")
		{
		}
		void OnUpdate(pig::Timestep ts) override
		{
			m_ExpectedEvent = false;
		}

		bool OnEvent(const pig::Event& event) override
		{
			m_ExpectedEvent = StoreToVariant(event);
			return true;
		}
		ExpectedEventResult m_ExpectedEvent = false;
	};
}

namespace CatchTestsetFail
{
	TEST_CASE("app.Layers::EventsTest")
	{
		pig::Application& app = pig::CreateApplication();

		pig::S_Ptr<TestLayer> testLayer = std::make_shared<TestLayer>();
		app.PushLayer(testLayer);
		pig::WindowsWindow& appWindow = static_cast<pig::WindowsWindow&>(app.GetWindow());

		int wParam = 123;
		int lParam = 456;
		SECTION("Key pressed events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			pig::KeyPressedEvent event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(!event.GetRepeatCount());
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
				
			wParam = 123;
			lParam = 0;
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(!event.GetRepeatCount());

			wParam = 678;
			lParam = 0x40000000;
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.GetRepeatCount());
		}
		SECTION("Key released events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::KEYUP, wParam, lParam);
			pig::KeyReleasedEvent event = GetEventValue<pig::KeyReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyReleased);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Key char events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::CHAR, wParam, lParam);
			pig::KeyTypedEvent event = GetEventValue<pig::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyTyped);
			CHECK(event.GetKeyCode() == wParam);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse scroll events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSEWHEEL, wParam, lParam);
			pig::MouseScrolledEvent event = GetEventValue<pig::MouseScrolledEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseScrolled);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse move events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSEMOVE, wParam, lParam);
			pig::MouseMovedEvent event = GetEventValue<pig::MouseMovedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseMoved);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse button down events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			pig::MouseButtonPressedEvent event = GetEventValue<pig::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonPressed);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSELBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pig::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSERBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pig::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSEMBUTTONDOWN, wParam, lParam);
			event = GetEventValue<pig::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mouse button up events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pig::MouseButtonReleasedEvent event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 0);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSERBUTTONUP, wParam, lParam);
			event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 1);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSEMBUTTONUP, wParam, lParam);
			event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetMouseButton() == 2);
		}
		SECTION("Mix Events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::MOUSELBUTTONUP, wParam, lParam);
			pig::MouseButtonReleasedEvent event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == 0);

			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::CHAR, wParam, lParam);
			pig::KeyTypedEvent event2 = GetEventValue<pig::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event2.GetEventType() == pig::EventType::KeyTyped);
			CHECK(event2.GetKeyCode() == wParam);
			CHECK(event2.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event2.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application close Events")
		{
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::DESTROY, wParam, lParam);
			pig::WindowCloseEvent event = GetEventValue<pig::WindowCloseEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::WindowClose);
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application resize Events")
		{
			wParam = 1337;
			lParam = 7331;

			LPARAM param = MAKELONG(wParam, lParam);
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::SIZE, wParam, param);
			pig::WindowResizeEvent event = GetEventValue<pig::WindowResizeEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::WindowResize);
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application resize Events (minimize)")
		{
			LPARAM param;
			SECTION("width is 0")
			{
				param = MAKELONG(0, 7331);
			}
			SECTION("height is 0")
			{
				param = MAKELONG(1337, 0);
			}
			
			appWindow.SendFakeEvent(pig::WindowsWindow::EventType::SIZE, wParam, param);
			const bool hasEvent = GetEventValue<bool>(testLayer->m_ExpectedEvent);
			CHECK(!hasEvent);
		}
	}
} // End namespace: CatchTestsetFail

