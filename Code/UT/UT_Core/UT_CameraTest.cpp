#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include <Pigeon/Core/InputLayer.h>
#include <Pigeon/Core/KeyCodes.h>

#include <Pigeon/Renderer/OrthographicCamera.h>
#include <Pigeon/Renderer/Renderer2D.h>
#include <Pigeon/Renderer/Texture.h>

#include <Platform/Testing/TestingWindow.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace CatchTestsetFail
{
	TEST_CASE("Core.Camera::OrthographicCamera")
	{
		pig::Application& app = pig::CreateApplication();
		const glm::vec4 ortoValues(-0.5f, 0.5f, -0.5f, 0.5f);
		const glm::vec3 pos0(0.f, 0.f, 0.f);
		const glm::vec3 pos1(-1.f, 2.f, 3.f);
		const glm::vec3 pos2(245.f, 672.6f, 12.341f);
		const float rot0 = 0.f;
		const float rot1 = 1.5f;
		const float rot2 = -4.5f;

		pig::OrthographicCamera camera(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w);
		CHECK(camera.GetPosition() == pos0);
		CHECK(camera.GetRotation() == rot0);
		glm::mat4 projMat = camera.GetProjectionMatrix();
		glm::mat4 viewMat = camera.GetViewMatrix();
		glm::mat4 projViewMat = camera.GetViewProjectionMatrix();

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == glm::mat4(1.f));
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));

		camera.SetPosition(pos1);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot0);
		camera.SetRotation(rot1);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot1);

		projMat = camera.GetProjectionMatrix();
		viewMat = camera.GetViewMatrix();
		projViewMat = camera.GetViewProjectionMatrix();

		glm::mat4 inverseTransform = glm::inverse(glm::translate(glm::mat4(1.0f), pos1) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rot1), glm::vec3(0, 0, 1)));

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == inverseTransform);
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f) * inverseTransform);

		camera.SetRotation(rot2);
		CHECK(camera.GetPosition() == pos1);
		CHECK(camera.GetRotation() == rot2);
		camera.SetPosition(pos2);
		CHECK(camera.GetPosition() == pos2);
		CHECK(camera.GetRotation() == rot2);

		projMat = camera.GetProjectionMatrix();
		viewMat = camera.GetViewMatrix();
		projViewMat = camera.GetViewProjectionMatrix();

		inverseTransform = glm::inverse(glm::translate(glm::mat4(1.0f), pos2) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rot2), glm::vec3(0, 0, 1)));

		CHECK(projMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f));
		CHECK(viewMat == inverseTransform);
		CHECK(projViewMat == glm::ortho(ortoValues.x, ortoValues.y, ortoValues.z, ortoValues.w, -1.0f, 1.0f) * inverseTransform);
	}
	TEST_CASE("Core.Camera::OrthographicCameraController")
	{
		pig::Application& app = pig::CreateApplication();
		pig::TestingWindow& appWindow = static_cast<pig::TestingWindow&>(app.GetWindow());
		const pig::Timestep timestep(5);
		app.TestUpdate(timestep);
		SECTION("Keyboard events")
		{
			pig::OrthographicCameraController cameraController(true, 0.5f, 0.f);
			const glm::vec3& camPos = cameraController.GetData().m_CameraPosition;

			pig::Input& appInput = static_cast<pig::Input&>(pig::Input::GetInput());

			CHECK(camPos.x == 0.f);
			CHECK(camPos.y == 0.f);
			CHECK(camPos.z == 0.f);

			SECTION("MovingDirections")
			{
				SECTION("Moving left")
				{
					pig::KeyPressedEvent pressEvent(PG_KEY_A, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x < 0.f);
					CHECK(camPos.y == 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving right")
				{
					pig::KeyPressedEvent pressEvent(PG_KEY_D, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x > 0.f);
					CHECK(camPos.y == 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving up")
				{
					pig::KeyPressedEvent pressEvent(PG_KEY_W, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x == 0.f);
					CHECK(camPos.y > 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving down")
				{
					pig::KeyPressedEvent pressEvent(PG_KEY_S, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x == 0.f);
					CHECK(camPos.y < 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving up and left")
				{
					pig::KeyPressedEvent pressEvent1(PG_KEY_A, 0);
					pig::KeyPressedEvent pressEvent2(PG_KEY_W, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent1);
					appWindow.TESTING_TriggerEvent(&pressEvent2);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x < 0.f);
					CHECK(camPos.y > 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving down and right")
				{
					pig::KeyPressedEvent pressEvent1(PG_KEY_D, 0);
					pig::KeyPressedEvent pressEvent2(PG_KEY_S, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent1);
					appWindow.TESTING_TriggerEvent(&pressEvent2);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x > 0.f);
					CHECK(camPos.y < 0.f);
					CHECK(camPos.z == 0.f);
				}
			}
		}
		SECTION("Mouse events")
		{
			SECTION("Mouse scrolled")
			{
				pig::OrthographicCameraController cameraController(true, 0.5f, 0.f);

				cameraController.SetZoomLevel(3.5f);
				CHECK(cameraController.GetZoomLevel() == 3.5f);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 5.f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				cameraController.OnUpdate(timestep);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 3.5f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				const glm::vec3 pos0(0.f, 0.f, 0.f);
				const float rot0 = 0.f;

				pig::MouseScrolledEvent event(123.f, 456.f);

				cameraController.OnEvent(event);
				cameraController.OnUpdate(timestep);
				
				CHECK(cameraController.GetCamera().GetPosition() == pos0);
				CHECK(cameraController.GetCamera().GetRotation() == rot0);
				CHECK(cameraController.GetZoomLevel() == 0.25f);
				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 0.25f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				pig::OrthographicCamera cameraTest(-0.125f, 0.125f, -0.25f, 0.25f);
				const pig::OrthographicCamera& controllerCamera = cameraController.GetCamera();
				CHECK(cameraTest.GetData().m_Position == controllerCamera.GetData().m_Position);
				CHECK(cameraTest.GetData().m_Rotation == controllerCamera.GetData().m_Rotation);
				CHECK(cameraTest.GetData().m_ViewMatrix == controllerCamera.GetData().m_ViewMatrix);
				CHECK(cameraTest.GetData().m_ViewProjectionMatrix == controllerCamera.GetData().m_ViewProjectionMatrix);
				CHECK(cameraTest.GetData().m_ProjectionMatrix == controllerCamera.GetData().m_ProjectionMatrix);
			}
			SECTION("Window Resized")
			{
				pig::OrthographicCameraController cameraController(true, 0.123f, 0.f);
				CHECK(cameraController.GetZoomLevel() == 1.0f);
				cameraController.SetZoomLevel(4.f);

				pig::WindowResizeEvent event(100.f, 1000.f);
				cameraController.OnEvent(event);
				
				cameraController.OnUpdate(timestep);
				cameraController.OnUpdate(timestep);

				const glm::vec3 pos0(0.f, 0.f, 0.f);
				const float rot0 = 0.f;

				CHECK(cameraController.GetCamera().GetPosition() == pos0);
				CHECK(cameraController.GetCamera().GetRotation() == rot0);
				CHECK(cameraController.GetZoomLevel() == 4.f);
				CHECK(cameraController.GetData().m_AspectRatio == 0.1f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 4.f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				pig::OrthographicCamera cameraTest(-0.4f, 0.4f, -4.f, 4.f);
				const pig::OrthographicCamera& controllerCamera = cameraController.GetCamera();
				CHECK(cameraTest.GetData().m_Position == controllerCamera.GetData().m_Position);
				CHECK(cameraTest.GetData().m_Rotation == controllerCamera.GetData().m_Rotation);
				CHECK(cameraTest.GetData().m_ViewMatrix == controllerCamera.GetData().m_ViewMatrix);
				CHECK(cameraTest.GetData().m_ViewProjectionMatrix == controllerCamera.GetData().m_ViewProjectionMatrix);
				CHECK(cameraTest.GetData().m_ProjectionMatrix == controllerCamera.GetData().m_ProjectionMatrix);
			}
		}
		SECTION("Camera that does not react to input")
		{
			pig::OrthographicCameraController cameraController(false, 0.5f, 0.f);
			const glm::vec3& camPos = cameraController.GetData().m_CameraPosition;

			pig::Input& appInput = static_cast<pig::Input&>(pig::Input::GetInput());

			CHECK(camPos.x == 0.f);
			CHECK(camPos.y == 0.f);
			CHECK(camPos.z == 0.f);

			SECTION("MovingDirections")
			{
				SECTION("Moving left")
				{
					pig::KeyPressedEvent pressEvent(PG_KEY_A, 0);
					appWindow.TESTING_TriggerEvent(&pressEvent);
					app.TestUpdate(timestep);
					cameraController.OnUpdate(timestep);
					CHECK(camPos.x == 0.f);
					CHECK(camPos.y == 0.f);
					CHECK(camPos.z == 0.f);
				}
			}
		}
	}
} // End namespace: CatchTestsetFail

