#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/CameraSystem.h>
#include <Pigeon/Core/OrthographicCameraComponent.h>
#include <Pigeon/Core/KeyCodes.h>
#include <Pigeon/Core/KeyPressedEventComponent.h>
#include <Pigeon/Core/MouseScrolledEventComponent.h>
#include <Pigeon/Core/WindowResizeEventComponent.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no input events -> camera position unchanged
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::NoEventsNoCameraMovement")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition = { 0.f, 0.f, 0.f };
		cam.m_ReactsToInput  = true;

		world.Update(pig::Timestep(100));

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_A pressed -> camera moves left (negative X)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::KeyAMovesCameraLeft")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evt =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pig::PG_KEY_A;

		pig::Timestep ts(1000); // 1 second
		world.Update(ts);

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(camAfter.m_CameraPosition.x < 0.f);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_D pressed -> camera moves right (positive X)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::KeyDMovesCameraRight")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evt =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pig::PG_KEY_D;

		pig::Timestep ts(1000);
		world.Update(ts);

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(camAfter.m_CameraPosition.x > 0.f);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_W pressed -> camera moves up (positive Y)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::KeyWMovesCameraUp")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evt =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pig::PG_KEY_W;

		pig::Timestep ts(1000);
		world.Update(ts);

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(camAfter.m_CameraPosition.y > 0.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_S pressed -> camera moves down (negative Y)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::KeySMovesCameraDown")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evt =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pig::PG_KEY_S;

		pig::Timestep ts(1000);
		world.Update(ts);

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(camAfter.m_CameraPosition.y < 0.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: scroll event -> zoom level decreases
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::ScrollEventChangesZoomLevel")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_ZoomLevel     = 2.f;
		cam.m_ReactsToInput = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::MouseScrolledEventComponent& scroll =
			pig::World::GetRegistryDirect().emplace<pig::MouseScrolledEventComponent>(evtEnt);
		scroll.m_YOffset = 4.f; // zoom in

		world.Update(pig::Timestep(0));

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		// ZoomLevel = 2.0 - 4.0 * 0.25 = 1.0
		CHECK(FLOAT_EQ(camAfter.m_ZoomLevel, 1.f));
	}

	// ---------------------------------------------------------------------------
	// Edge case: scroll event would push zoom below 0.25 -> clamped to 0.25
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::ScrollZoomClampedToMinimum")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_ZoomLevel     = 0.5f;
		cam.m_ReactsToInput = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::MouseScrolledEventComponent& scroll =
			pig::World::GetRegistryDirect().emplace<pig::MouseScrolledEventComponent>(evtEnt);
		scroll.m_YOffset = 100.f; // large zoom in

		world.Update(pig::Timestep(0));

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_ZoomLevel, 0.25f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: window resize event -> aspect ratio updated
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::ResizeEventUpdatesAspectRatio")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_AspectRatio   = 1.f;
		cam.m_ReactsToInput = true;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::WindowResizeEventComponent& resize =
			pig::World::GetRegistryDirect().emplace<pig::WindowResizeEventComponent>(evtEnt);
		resize.m_Width  = 1280;
		resize.m_Height = 720;

		world.Update(pig::Timestep(0));

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		const float expectedAspect = 1280.f / 720.f;
		CHECK(std::fabs(camAfter.m_AspectRatio - expectedAspect) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Guard: camera with m_ReactsToInput=false -> not moved by key events
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::NonReactingCameraIgnoresInput")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::CameraSystem>());

		entt::entity camEnt = pig::World::GetRegistryDirect().create();
		pig::OrthographicCameraComponent& cam =
			pig::World::GetRegistryDirect().emplace<pig::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = false; // <-- does not react

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evt =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pig::PG_KEY_D;

		pig::Timestep ts(1000);
		world.Update(ts);

		const pig::OrthographicCameraComponent& camAfter =
			pig::World::GetRegistryDirect().get<pig::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.CameraSystem::DeclareAccessIsCorrect")
	{
		pig::CameraSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::KeyPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::MouseScrolledEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::WindowResizeEventComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pig::OrthographicCameraComponent))) > 0);
	}

} // namespace CatchTestsetFail
