#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/QuadRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: a quad with a resolved world transform emits one draw event that
	// carries the world matrix, sort key, colour, and texture.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::EmitsMatchingDrawEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		// QuadComponent and WorldTransformComponent are added in production by other systems.
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		sbx::QuadComponent& quad = pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(ent);
		quad.m_Color = { 0.1f, 0.2f, 0.3f };
		quad.m_TextureID = pg::UUID::Generate();
		pg::WorldTransformComponent& worldTransform = pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(ent);
		worldTransform.m_Matrix = glm::translate(glm::mat4(1.f), glm::vec3(2.f, 3.f, 0.f));
		worldTransform.m_SortKey = 3.f;

		// In-frame events are cleared at end of frame; retain them so the test can inspect them.
		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawQuadInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::DrawQuadInFrameEvent& event = view.get<const pg::DrawQuadInFrameEvent>(view.front());
		CHECK(event.m_Transform == worldTransform.m_Matrix);
		CHECK(event.m_SortKey == Approx(3.f));
		CHECK(event.m_Color == quad.m_Color);
		CHECK(event.m_TextureID == quad.m_TextureID);
	}

	// ---------------------------------------------------------------------------
	// Guard: a quad without a resolved world transform yet emits no draw event.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::NoWorldTransformNoEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(ent);

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawQuadInFrameEvent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no quads -> no draw events emitted.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::NoQuadsNoEvents")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawQuadInFrameEvent>();
		CHECK(view.size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Edge case: one draw event per quad with a world transform.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::EmitsOneEventPerQuad")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		pg::ecs::Entity a = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(a);
		pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(a);
		pg::ecs::Entity b = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(b);
		pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(b);

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawQuadInFrameEvent>();
		CHECK(view.size() == 2);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::DeclareAccessIsCorrect")
	{
		sbx::QuadRenderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::QuadComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WorldTransformComponent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
