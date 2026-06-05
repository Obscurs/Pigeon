#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Renderer/CameraSystem.h>
#include <Pigeon/Renderer/OrthographicCameraComponent.h>
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
	TEST_CASE("Renderer.CameraSystem::NoEventsNoCameraMovement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition = { 0.f, 0.f, 0.f };
		cam.m_ReactsToInput  = true;

		world.Update(pg::Timestep(100));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_A pressed -> camera moves left (negative X)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyAMovesCameraLeft")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evt =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pg::PG_KEY_A;

		pg::Timestep ts(1000); // 1 second
		world.Update(ts);

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(camAfter.m_CameraPosition.x < 0.f);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_D pressed -> camera moves right (positive X)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyDMovesCameraRight")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evt =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pg::PG_KEY_D;

		pg::Timestep ts(1000);
		world.Update(ts);

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(camAfter.m_CameraPosition.x > 0.f);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_W pressed -> camera moves up (positive Y)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyWMovesCameraUp")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evt =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pg::PG_KEY_W;

		pg::Timestep ts(1000);
		world.Update(ts);

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(camAfter.m_CameraPosition.y > 0.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_S pressed -> camera moves down (negative Y)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeySMovesCameraDown")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evt =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pg::PG_KEY_S;

		pg::Timestep ts(1000);
		world.Update(ts);

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(camAfter.m_CameraPosition.y < 0.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: scroll event -> zoom level decreases
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::ScrollEventChangesZoomLevel")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_ZoomLevel     = 2.f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::MouseScrolledEventComponent& scroll =
			pg::World::GetRegistryDirect().emplace<pg::MouseScrolledEventComponent>(evtEnt);
		scroll.m_YOffset = 4.f; // zoom in

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		// ZoomLevel = 2.0 - 4.0 * 0.25 = 1.0
		CHECK(FLOAT_EQ(camAfter.m_ZoomLevel, 1.f));
	}

	// ---------------------------------------------------------------------------
	// Edge case: scroll event would push zoom below 0.25 -> clamped to 0.25
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::ScrollZoomClampedToMinimum")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_ZoomLevel     = 0.5f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::MouseScrolledEventComponent& scroll =
			pg::World::GetRegistryDirect().emplace<pg::MouseScrolledEventComponent>(evtEnt);
		scroll.m_YOffset = 100.f; // large zoom in

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_ZoomLevel, 0.25f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: window resize event -> aspect ratio updated
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::ResizeEventUpdatesAspectRatio")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_AspectRatio   = 1.f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::WindowResizeEventComponent& resize =
			pg::World::GetRegistryDirect().emplace<pg::WindowResizeEventComponent>(evtEnt);
		resize.m_Width  = 1280;
		resize.m_Height = 720;

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		const float expectedAspect = 1280.f / 720.f;
		CHECK(std::fabs(camAfter.m_AspectRatio - expectedAspect) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Guard: camera with m_ReactsToInput=false -> not moved by key events
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::NonReactingCameraIgnoresInput")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraPosition        = { 0.f, 0.f, 0.f };
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput          = false; // <-- does not react

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evt =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evt.m_KeyCode = pg::PG_KEY_D;

		pg::Timestep ts(1000);
		world.Update(ts);

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.x, 0.f));
		CHECK(FLOAT_EQ(camAfter.m_CameraPosition.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::DeclareAccessIsCorrect")
	{
		pg::CameraSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseScrolledEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WindowResizeEventComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
	}

} // namespace CatchTestsetFail
