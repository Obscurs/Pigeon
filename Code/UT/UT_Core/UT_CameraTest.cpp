#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "Utils/TestApp.h"

#include <Pigeon/KeyCodes.h>
#include <Pigeon/Renderer/OrthographicCamera.h>
#include <Pigeon/Renderer/Renderer2D.h>
#include <Pigeon/Renderer/Texture.h>

#include <Platform/Testing/TestingInput.h>

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
		SECTION("Keyboard events")
		{
			pig::OrthographicCameraController cameraController(0.5f);
			const glm::vec3& camPos = cameraController.GetData().m_CameraPosition;

			pig::TestingInput& appInput = static_cast<pig::TestingInput&>(pig::Input::GetInput());

			CHECK(camPos.x == 0.f);
			CHECK(camPos.y == 0.f);
			CHECK(camPos.z == 0.f);

			SECTION("MovingDirections")
			{
				SECTION("Moving left")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_A);
					cameraController.OnUpdate(5);
					CHECK(camPos.x < 0.f);
					CHECK(camPos.y == 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving right")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_D);
					cameraController.OnUpdate(5);
					CHECK(camPos.x > 0.f);
					CHECK(camPos.y == 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving up")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_W);
					cameraController.OnUpdate(5);
					CHECK(camPos.x == 0.f);
					CHECK(camPos.y > 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving down")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_S);
					cameraController.OnUpdate(5);
					CHECK(camPos.x == 0.f);
					CHECK(camPos.y < 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving up and left")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_A);
					appInput.TESTING_KeysPressed.push_back(PG_KEY_W);
					cameraController.OnUpdate(5);
					CHECK(camPos.x < 0.f);
					CHECK(camPos.y > 0.f);
					CHECK(camPos.z == 0.f);
				}
				SECTION("Moving down and right")
				{
					appInput.TESTING_KeysPressed.push_back(PG_KEY_D);
					appInput.TESTING_KeysPressed.push_back(PG_KEY_S);
					cameraController.OnUpdate(5);
					CHECK(camPos.x > 0.f);
					CHECK(camPos.y < 0.f);
					CHECK(camPos.z == 0.f);
				}
				appInput.TESTING_KeysPressed.clear();
			}
		}
		SECTION("Mouse events")
		{
			SECTION("Mouse scrolled")
			{
				pig::OrthographicCameraController cameraController(0.5f);

				cameraController.SetZoomLevel(3.5f);
				CHECK(cameraController.GetZoomLevel() == 3.5f);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 5.f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				cameraController.OnUpdate(5);

				CHECK(cameraController.GetData().m_AspectRatio == 0.5f);
				CHECK(cameraController.GetData().m_Rotation == false);
				CHECK(cameraController.GetData().m_CameraRotation == 0.f);
				CHECK(cameraController.GetData().m_CameraTranslationSpeed == 3.5f);
				CHECK(cameraController.GetData().m_CameraRotationSpeed == 180.f);

				const glm::vec3 pos0(0.f, 0.f, 0.f);
				const float rot0 = 0.f;

				pig::MouseScrolledEvent event(123.f, 456.f);

				cameraController.OnEvent(event);
				cameraController.OnUpdate(5);
				
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
				pig::OrthographicCameraController cameraController(0.123f);
				CHECK(cameraController.GetZoomLevel() == 1.0f);
				cameraController.SetZoomLevel(4.f);

				pig::WindowResizeEvent event(100.f, 1000.f);
				cameraController.OnEvent(event);
				cameraController.OnUpdate(10);

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
	}
} // End namespace: CatchTestsetFail

