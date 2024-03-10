#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>
#include <variant>

#include <minwindef.h>

#include "Utils/TestApp.h"
#include <Platform/Testing/TestingWindow.h>

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
	TEST_CASE("Core.Layers::Events")
	{
		pig::Application& app = pig::CreateApplication();

		pig::S_Ptr<TestLayer> testLayer = std::make_shared<TestLayer>();
		app.PushLayer(testLayer);


		pig::TestingWindow& appWindow = static_cast<pig::TestingWindow&>(app.GetWindow());

		SECTION("Key pressed events")
		{
			int eventKeyCode = 123;
			int eventRepeat = 0;
			pig::KeyPressedEvent eventSent1(eventKeyCode, eventRepeat);
			appWindow.TESTING_TriggerEvent(&eventSent1);

			pig::KeyPressedEvent event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == eventKeyCode);
			CHECK(!event.GetRepeatCount());
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
				
			eventKeyCode = 123;
			eventRepeat = 0;
			pig::KeyPressedEvent eventSent2(eventKeyCode, eventRepeat);
			appWindow.TESTING_TriggerEvent(&eventSent2);
			event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == eventKeyCode);
			CHECK(!event.GetRepeatCount());

			eventKeyCode = 678;
			eventRepeat = 0x40000000;
			pig::KeyPressedEvent eventSent3(eventKeyCode, eventRepeat);
			appWindow.TESTING_TriggerEvent(&eventSent3);
			event = GetEventValue<pig::KeyPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyPressed);
			CHECK(event.GetKeyCode() == eventKeyCode);
			CHECK(event.GetRepeatCount());
		}
		SECTION("Key released events")
		{
			const int eventKeyCode = 123;
			pig::KeyReleasedEvent eventSent(eventKeyCode);
			appWindow.TESTING_TriggerEvent(&eventSent);
			pig::KeyReleasedEvent event = GetEventValue<pig::KeyReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyReleased);
			CHECK(event.GetKeyCode() == eventKeyCode);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Key char events")
		{
			const int eventKeyCode = 123;
			pig::KeyTypedEvent eventSent(eventKeyCode);
			appWindow.TESTING_TriggerEvent(&eventSent);

			pig::KeyTypedEvent event = GetEventValue<pig::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::KeyTyped);
			CHECK(event.GetKeyCode() == eventKeyCode);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Mouse scroll events")
		{
			const float offsetX = 123.f;
			const float offsetY = 456.f;
			pig::MouseScrolledEvent eventSent(offsetX, offsetY);
			appWindow.TESTING_TriggerEvent(&eventSent);

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
			const float offsetX = 123.f;
			const float offsetY = 456.f;
			pig::MouseMovedEvent eventSent(offsetX, offsetY);
			appWindow.TESTING_TriggerEvent(&eventSent);

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
			const int keyButton = 123;
			pig::MouseButtonPressedEvent eventSent(keyButton);
			appWindow.TESTING_TriggerEvent(&eventSent);

			pig::MouseButtonPressedEvent event = GetEventValue<pig::MouseButtonPressedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonPressed);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == keyButton);
		}
		SECTION("Mouse button up events")
		{
			const int keyButton = 123;
			pig::MouseButtonReleasedEvent eventSent(keyButton);
			appWindow.TESTING_TriggerEvent(&eventSent);

			pig::MouseButtonReleasedEvent event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == keyButton);
		}
		SECTION("Mix Events")
		{
			const int keyButton1 = 123;
			pig::MouseButtonReleasedEvent eventSent1(keyButton1);
			appWindow.TESTING_TriggerEvent(&eventSent1);

			pig::MouseButtonReleasedEvent event = GetEventValue<pig::MouseButtonReleasedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event.GetEventType() == pig::EventType::MouseButtonReleased);
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(event.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
			CHECK(event.GetMouseButton() == keyButton1);

			const int keyButton2 = 456;
			pig::KeyTypedEvent eventSent2(keyButton2);
			appWindow.TESTING_TriggerEvent(&eventSent2);

			pig::KeyTypedEvent event2 = GetEventValue<pig::KeyTypedEvent>(testLayer->m_ExpectedEvent);
			CHECK(event2.GetEventType() == pig::EventType::KeyTyped);
			CHECK(event2.GetKeyCode() == keyButton2);
			CHECK(event2.IsInCategory(pig::EventCategory::EventCategoryInput));
			CHECK(event2.IsInCategory(pig::EventCategory::EventCategoryKeyboard));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryApplication));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryMouse));
			CHECK(!event2.IsInCategory(pig::EventCategory::EventCategoryMouseButton));
		}
		SECTION("Application close Events")
		{
			pig::WindowCloseEvent eventSent1;
			appWindow.TESTING_TriggerEvent(&eventSent1);

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
			const int width = 1337;
			const int height = 7331;
			pig::WindowResizeEvent eventSent1(width, height);
			appWindow.TESTING_TriggerEvent(&eventSent1);
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
				const int width = 1337;
				const int height = 0;
				pig::WindowResizeEvent eventSent1(width, height);
				appWindow.TESTING_TriggerEvent(&eventSent1);
			}
			SECTION("height is 0")
			{
				const int width = 0;
				const int height = 7331;
				pig::WindowResizeEvent eventSent1(width, height);
				appWindow.TESTING_TriggerEvent(&eventSent1);
			}
			
			const bool hasEvent = GetEventValue<bool>(testLayer->m_ExpectedEvent);
			CHECK(!hasEvent);
		}
	}
} // End namespace: CatchTestsetFail

