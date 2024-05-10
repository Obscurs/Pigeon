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
			
			appWindow.TESTING_TriggerEvent(&pig::KeyPressedEvent(eventKeyCode, 0));
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

			appWindow.TESTING_TriggerEvent(&pig::KeyReleasedEvent(eventKeyCode));
			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(pig::Input::IsKeyReleased(eventKeyCode));

			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode));
		}
		SECTION("Key typed events")
		{
			int eventKeyCode1 = 123;
			int eventKeyCode2 = 456;
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode1));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode2));

			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode1));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode2));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			app.TestUpdate(timestep);
			CHECK(pig::Input::IsKeyTyped(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(pig::Input::IsKeyTyped(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
			app.TestUpdate(timestep);
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode1));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode1));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode1));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode2));
			CHECK(!pig::Input::IsKeyPressed(eventKeyCode2));
			CHECK(!pig::Input::IsKeyReleased(eventKeyCode2));
		}
		SECTION("Multiple keys typed event")
		{
			int eventKeyCode1 = 1;
			int eventKeyCode2 = 2;
			int eventKeyCode3 = 3;
			int eventKeyCode4 = 2;
			int eventKeyCode5 = 2;
			int eventKeyCode6 = 1;

			CHECK(!pig::Input::IsKeyTyped(eventKeyCode1));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode2));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode3));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode4));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode5));
			CHECK(!pig::Input::IsKeyTyped(eventKeyCode6));

			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode1));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode2));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode3));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode4));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode5));
			appWindow.TESTING_TriggerEvent(&pig::KeyTypedEvent(eventKeyCode6));
			app.TestUpdate(timestep);
			const std::vector<int>& keysTyped = pig::Input::GetKeysTyped();
			REQUIRE(keysTyped.size() == 6);
			CHECK(keysTyped[0] == eventKeyCode1);
			CHECK(keysTyped[1] == eventKeyCode2);
			CHECK(keysTyped[2] == eventKeyCode3);
			CHECK(keysTyped[3] == eventKeyCode4);
			CHECK(keysTyped[4] == eventKeyCode5);
			CHECK(keysTyped[5] == eventKeyCode6);
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

