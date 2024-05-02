#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>
#include <variant>

#include <minwindef.h>

#include "Utils/TestApp.h"
#include <Platform/Testing/TestingWindow.h>

#include <Pigeon/Core/InputLayer.h>
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
		void OnUpdate(const pig::Timestep& ts) override
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
	TEST_CASE("Core.Layers::Input")
	{
		pig::Application& app = pig::CreateApplication();
		const pig::Timestep timestep(5);
		pig::TestingWindow& appWindow = static_cast<pig::TestingWindow&>(app.GetWindow());

		SECTION("Key pressed and released")
		{
			int eventKeyCode = PG_KEY_A;
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));
			
			pig::KeyPressedEvent pressEvent(eventKeyCode, 0);
			appWindow.TESTING_TriggerEvent(&pressEvent);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));
			app.TestUpdate(timestep);
			CHECK(pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));
			
			app.TestUpdate(timestep);
			CHECK(pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));

			pig::KeyReleasedEvent releaseEvent(eventKeyCode);
			appWindow.TESTING_TriggerEvent(&releaseEvent);
			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(pig::Input::IsKeyReleased(eventKeyCode));

			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));
		}
		SECTION("Mouse button pressed and released")
		{
			int eventKeyCode = PG_KEY_BACKSPACE;
			CHECK(!pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(!pig::Input::IsMouseButtonReleased(eventKeyCode));

			pig::KeyPressedEvent pressEvent(eventKeyCode, 0);
			appWindow.TESTING_TriggerEvent(&pressEvent);
			CHECK(!pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(!pig::Input::IsMouseButtonReleased(eventKeyCode));

			app.TestUpdate(timestep);
			CHECK(pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(pig::Input::IsMouseButtonPressed(eventKeyCode, true));
			CHECK(!pig::Input::IsMouseButtonReleased(eventKeyCode));

			app.TestUpdate(timestep);
			CHECK(pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(!pig::Input::IsMouseButtonPressed(eventKeyCode, true));
			CHECK(!pig::Input::IsMouseButtonReleased(eventKeyCode));

			pig::KeyReleasedEvent releaseEvent(eventKeyCode);
			appWindow.TESTING_TriggerEvent(&releaseEvent);
			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(pig::Input::IsMouseButtonReleased(eventKeyCode));

			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsMouseButtonPressed(eventKeyCode));
			CHECK(!pig::Input::IsMouseButtonReleased(eventKeyCode));
		}
		SECTION("Mouse position")
		{
			glm::vec2 targetPos(123.f, 456.f);
			CHECK(pig::Input::GetMousePosition() != targetPos);

			pig::MouseMovedEvent moveEvent(123.f, 456.f);
			appWindow.TESTING_TriggerEvent(&moveEvent);
			CHECK(pig::Input::GetMousePosition() != targetPos);
			app.TestUpdate(timestep);
			CHECK(pig::Input::GetMousePosition() == targetPos);
		}

		SECTION("Mouse scroll")
		{
			glm::vec2 targetScroll(123.f, 456.f);
			CHECK(pig::Input::GetMouseScrolled() == glm::vec2(0.f,0.f));

			pig::MouseScrolledEvent scrollEvent(targetScroll.x, targetScroll.y);
			appWindow.TESTING_TriggerEvent(&scrollEvent);
			CHECK(pig::Input::GetMouseScrolled() != targetScroll);
			app.TestUpdate(timestep);
			CHECK(pig::Input::GetMouseScrolled() == targetScroll);
		}

		SECTION("multiple events concurrently")
		{
			int eventKeyCode1 = PG_KEY_D;
			int eventKeyCode2 = PG_KEY_LEFT;
			int eventKeyCode3 = PG_KEY_F16;
			glm::vec2 targetPos(123.f, 456.f);

			pig::KeyPressedEvent pressEvent1(eventKeyCode1, 0);
			pig::KeyPressedEvent pressEvent2(eventKeyCode2, 0);
			pig::KeyPressedEvent pressEvent3(eventKeyCode3, 0);
			appWindow.TESTING_TriggerEvent(&pressEvent1);
			appWindow.TESTING_TriggerEvent(&pressEvent2);
			app.TestUpdate(timestep);
			appWindow.TESTING_TriggerEvent(&pressEvent3);
			CHECK(pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode1, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode2, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode3));

			app.TestUpdate(timestep);
			CHECK(pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode3));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode3, true));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode3));

			pig::KeyReleasedEvent releaseEvent3(eventKeyCode3);
			appWindow.TESTING_TriggerEvent(&releaseEvent3);
			pig::MouseMovedEvent moveEvent(123.f, 456.f);
			appWindow.TESTING_TriggerEvent(&moveEvent);
			app.TestUpdate(timestep);
			CHECK(pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode3));
			CHECK(pig::Input::IsKeyReleased(eventKeyCode3));
			CHECK(pig::Input::GetMousePosition() == targetPos);

			pig::KeyReleasedEvent releaseEvent1(eventKeyCode1);
			pig::KeyReleasedEvent releaseEvent2(eventKeyCode2);
			appWindow.TESTING_TriggerEvent(&releaseEvent1);
			appWindow.TESTING_TriggerEvent(&releaseEvent2);
			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(pig::Input::IsKeyReleased(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode3));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode3));

			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode3));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode3));
			CHECK(pig::Input::GetMousePosition() == targetPos);
		}
	}
} // End namespace: CatchTestsetFail

