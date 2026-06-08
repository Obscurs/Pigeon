#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/CameraSystem.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	pg::ecs::Entity MakeReactingCamera(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity camEnt = registry.create();
		pg::OrthographicCameraComponent& cam = registry.emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput = true;
		registry.emplace<pg::PositionComponent>(camEnt).m_Position = { 0.f, 0.f, 0.f };
		return camEnt;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no input events -> no pan request emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::NoEventsNoCameraMovement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeReactingCamera(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(100));

		CHECK_FALSE(pg::World::GetRegistryDirect().all_of<pg::CameraTransformRequestOneFrameComponent>(camEnt));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_A pressed -> pan request targets a smaller X
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyAMovesCameraLeft")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeReactingCamera(pg::World::GetRegistryDirect());

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = pg::PG_KEY_A;

		world.Update(pg::Timestep(1000)); // 1 second

		REQUIRE(pg::World::GetRegistryDirect().all_of<pg::CameraTransformRequestOneFrameComponent>(camEnt));
		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<pg::CameraTransformRequestOneFrameComponent>(camEnt).m_Data;
		CHECK(data.m_SetPosition);
		CHECK(data.m_Position.x < 0.f);
		CHECK(FLOAT_EQ(data.m_Position.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_D pressed -> pan request targets a larger X
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyDMovesCameraRight")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeReactingCamera(pg::World::GetRegistryDirect());

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = pg::PG_KEY_D;

		world.Update(pg::Timestep(1000));

		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<pg::CameraTransformRequestOneFrameComponent>(camEnt).m_Data;
		CHECK(data.m_Position.x > 0.f);
		CHECK(FLOAT_EQ(data.m_Position.y, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_W pressed -> pan request targets a larger Y
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeyWMovesCameraUp")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeReactingCamera(pg::World::GetRegistryDirect());

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = pg::PG_KEY_W;

		world.Update(pg::Timestep(1000));

		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<pg::CameraTransformRequestOneFrameComponent>(camEnt).m_Data;
		CHECK(FLOAT_EQ(data.m_Position.x, 0.f));
		CHECK(data.m_Position.y > 0.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: PG_KEY_S pressed -> pan request targets a smaller Y
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::KeySMovesCameraDown")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeReactingCamera(pg::World::GetRegistryDirect());

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = pg::PG_KEY_S;

		world.Update(pg::Timestep(1000));

		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<pg::CameraTransformRequestOneFrameComponent>(camEnt).m_Data;
		CHECK(FLOAT_EQ(data.m_Position.x, 0.f));
		CHECK(data.m_Position.y < 0.f);
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
		cam.m_ZoomLevel = 2.f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::MouseScrolledEventComponent>(evtEnt).m_YOffset = 4.f; // zoom in

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
		cam.m_ZoomLevel = 0.5f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::MouseScrolledEventComponent>(evtEnt).m_YOffset = 100.f; // large zoom in

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
		cam.m_AspectRatio = 1.f;
		cam.m_ReactsToInput = true;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::WindowResizeEventComponent& resize =
			pg::World::GetRegistryDirect().emplace<pg::WindowResizeEventComponent>(evtEnt);
		resize.m_Width = 1280;
		resize.m_Height = 720;

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		const float expectedAspect = 1280.f / 720.f;
		CHECK(std::fabs(camAfter.m_AspectRatio - expectedAspect) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Guard: camera with m_ReactsToInput=false -> emits no pan request
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::NonReactingCameraIgnoresInput")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_CameraTranslationSpeed = 5.f;
		cam.m_ReactsToInput = false; // <-- does not react
		pg::World::GetRegistryDirect().emplace<pg::PositionComponent>(camEnt);

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = pg::PG_KEY_D;

		world.Update(pg::Timestep(1000));

		CHECK_FALSE(pg::World::GetRegistryDirect().all_of<pg::CameraTransformRequestOneFrameComponent>(camEnt));
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
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::CameraTransformRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
