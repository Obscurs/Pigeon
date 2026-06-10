#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/SetCameraRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Sandbox/CameraControlSystem.h"

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	pg::ecs::Entity MakeCamera(pg::ecs::Registry& registry, float zoom, const glm::vec3& position)
	{
		pg::ecs::Entity ent = registry.create();
		pg::OrthographicCameraComponent& cam = registry.emplace<pg::OrthographicCameraComponent>(ent);
		cam.m_ZoomLevel = zoom;
		registry.emplace<pg::PositionComponent>(ent).m_Position = position;
		return ent;
	}

	pg::InputStateSingletonComponent& MakeInput(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity ent = registry.create();
		return registry.emplace<pg::InputStateSingletonComponent>(ent);
	}

	void MakeScroll(pg::ecs::Registry& registry, float yOffset)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::MouseScrolledEventComponent>(ent).m_YOffset = yOffset;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no input state -> no request emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::NoInputStateNoRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no camera -> no request emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::NoCameraNoRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_D] = 1;

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: camera + input present but no WASD/scroll -> no request emitted
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::NoCameraInputNoRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect());
		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: D held -> request pans +X, zoom unchanged
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::HeldDKeyPansRight")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_D] = 1;
		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000)); // 1 second, zoom 1 -> speed 1 -> dx = 1

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_Position.x, 1.f));
		CHECK(FLOAT_EQ(req.m_Position.y, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.z, 0.f));
		CHECK(FLOAT_EQ(req.m_ZoomLevel, 1.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: A held -> request pans -X
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::HeldAKeyPansLeft")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_A] = 1;
		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_Position.x, -1.f));
		CHECK(FLOAT_EQ(req.m_Position.y, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.z, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: W held -> request pans +Y
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::HeldWKeyPansUp")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_W] = 1;
		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_Position.x, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.y, 1.f));
		CHECK(FLOAT_EQ(req.m_Position.z, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: S held -> request pans -Y
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::HeldSKeyPansDown")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_S] = 1;
		MakeCamera(pg::World::GetRegistryDirect(), 1.f, { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_Position.x, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.y, -1.f));
		CHECK(FLOAT_EQ(req.m_Position.z, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: scroll up -> zoom in (decreases zoom level), position unchanged
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::ScrollZoomsIn")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect());
		MakeCamera(pg::World::GetRegistryDirect(), 2.f, { 0.f, 0.f, 0.f });
		MakeScroll(pg::World::GetRegistryDirect(), 4.f); // 2.0 - 4*0.25 = 1.0

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_ZoomLevel, 1.f));
		CHECK(FLOAT_EQ(req.m_Position.x, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.y, 0.f));
		CHECK(FLOAT_EQ(req.m_Position.z, 0.f));
	}

	// ---------------------------------------------------------------------------
	// Edge case: large scroll would push zoom below 0.25 -> clamped to 0.25
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::ScrollZoomClampedToMinimum")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect());
		MakeCamera(pg::World::GetRegistryDirect(), 0.5f, { 0.f, 0.f, 0.f });
		MakeScroll(pg::World::GetRegistryDirect(), 100.f);

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_ZoomLevel, 0.25f));
	}

	// ---------------------------------------------------------------------------
	// Edge case: pan distance scales with zoom (zoomed in -> slower pan)
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::PanSpeedScalesWithZoom")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_D] = 1;
		MakeCamera(pg::World::GetRegistryDirect(), 0.5f, { 0.f, 0.f, 0.f }); // speed 0.5 -> dx = 0.5

		world.Update(pg::Timestep(1000));

		auto view = pg::World::GetRegistryDirect().view<pg::SetCameraRequestOneFrameComponent>();
		REQUIRE(view.size() == 1);
		const pg::SetCameraRequestOneFrameComponent& req = view.get<pg::SetCameraRequestOneFrameComponent>(view.front());
		CHECK(FLOAT_EQ(req.m_Position.x, 0.5f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CameraControlSystem::DeclareAccessIsCorrect")
	{
		sbx::CameraControlSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseScrolledEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SetCameraRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
