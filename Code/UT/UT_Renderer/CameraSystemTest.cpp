#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/CameraSystem.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/SetCameraRequestOneFrameComponent.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	pg::ecs::Entity MakeCamera(pg::ecs::Registry& registry, const glm::vec3& position)
	{
		pg::ecs::Entity camEnt = registry.create();
		registry.emplace<pg::OrthographicCameraComponent>(camEnt);
		registry.emplace<pg::PositionComponent>(camEnt).m_Position = position;
		return camEnt;
	}

	void MakeRequest(pg::ecs::Registry& registry, float zoom, const glm::vec3& position)
	{
		pg::ecs::Entity reqEnt = registry.create();
		pg::SetCameraRequestOneFrameComponent& req = registry.emplace<pg::SetCameraRequestOneFrameComponent>(reqEnt);
		req.m_ZoomLevel = zoom;
		req.m_Position = position;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no request and no resize -> no pan request, zoom unchanged
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::NoRequestNoCameraMovement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeCamera(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });
		pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt).m_ZoomLevel = 1.f;

		world.Update(pg::Timestep(1000));

		CHECK_FALSE(pg::World::GetRegistryDirect().all_of<pg::CameraTransformRequestOneFrameComponent>(camEnt));
		const pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(cam.m_ZoomLevel, 1.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: request zoom -> applied to the camera component
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::AppliesZoomFromRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeCamera(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });
		MakeRequest(pg::World::GetRegistryDirect(), 1.5f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(cam.m_ZoomLevel, 1.5f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: request position -> emits a pan request on the camera entity
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::EmitsPanRequestFromRequestPosition")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeCamera(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });
		MakeRequest(pg::World::GetRegistryDirect(), 1.f, { 2.f, 3.f, 0.f });

		world.Update(pg::Timestep(0));

		REQUIRE(pg::World::GetRegistryDirect().all_of<pg::CameraTransformRequestOneFrameComponent>(camEnt));
		const pg::TransformRequestData& data =
			pg::World::GetRegistryDirect().get<pg::CameraTransformRequestOneFrameComponent>(camEnt).m_Data;
		CHECK(data.m_SetPosition);
		CHECK(FLOAT_EQ(data.m_Position.x, 2.f));
		CHECK(FLOAT_EQ(data.m_Position.y, 3.f));
	}

	// ---------------------------------------------------------------------------
	// Every frame: the internal camera is synced from the resolved PositionComponent
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::SyncsCameraPositionFromPositionComponent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = MakeCamera(pg::World::GetRegistryDirect(), { 5.f, 6.f, 0.f });

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(cam.m_Camera.GetPosition().x, 5.f));
		CHECK(FLOAT_EQ(cam.m_Camera.GetPosition().y, 6.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: window resize event -> aspect ratio updated (viewport, not input)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::ResizeEventUpdatesAspectRatio")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_AspectRatio = 1.f;

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
	// Edge case: resize event with zero height -> aspect ratio left unchanged
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::ResizeZeroHeightIgnored")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::CameraSystem>());

		pg::ecs::Entity camEnt = pg::World::GetRegistryDirect().create();
		pg::OrthographicCameraComponent& cam =
			pg::World::GetRegistryDirect().emplace<pg::OrthographicCameraComponent>(camEnt);
		cam.m_AspectRatio = 1.f;

		pg::ecs::Entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::WindowResizeEventComponent& resize =
			pg::World::GetRegistryDirect().emplace<pg::WindowResizeEventComponent>(evtEnt);
		resize.m_Width = 1280;
		resize.m_Height = 0;

		world.Update(pg::Timestep(0));

		const pg::OrthographicCameraComponent& camAfter =
			pg::World::GetRegistryDirect().get<pg::OrthographicCameraComponent>(camEnt);
		CHECK(FLOAT_EQ(camAfter.m_AspectRatio, 1.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets (no longer reads raw input events)
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.CameraSystem::DeclareAccessIsCorrect")
	{
		pg::CameraSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::SetCameraRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WindowResizeEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::CameraTransformRequestOneFrameComponent))) > 0);

		CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyPressedEventComponent))) == 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseScrolledEventComponent))) == 0);
	}

} // namespace CatchTestsetFail
