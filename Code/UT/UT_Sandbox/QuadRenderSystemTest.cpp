#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawQuadInFrameEvent.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/QuadRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: one draw event per quad, with every field copied across.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::EmitsMatchingDrawEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		// QuadComponent is added in production by QuadSpawnSystem (a different system).
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		sbx::QuadComponent& quad = pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(ent);
		quad.m_Transform = glm::translate(glm::mat4(1.f), glm::vec3(2.f, 3.f, 1.f));
		quad.m_Color = { 0.1f, 0.2f, 0.3f };
		quad.m_Origin = { 0.4f, 0.5f, 0.f };
		quad.m_TextureID = pg::UUID::Generate();

		// In-frame events are cleared at end of frame; retain them so the test can inspect them.
		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawQuadInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::DrawQuadInFrameEvent& event = view.get<const pg::DrawQuadInFrameEvent>(view.front());
		CHECK(event.m_Transform == quad.m_Transform);
		CHECK(event.m_Color == quad.m_Color);
		CHECK(event.m_Origin == quad.m_Origin);
		CHECK(event.m_TextureID == quad.m_TextureID);
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
	// Edge case: one draw event per quad when several exist.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadRenderSystem::EmitsOneEventPerQuad")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());

		pg::ecs::Entity a = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(a);
		pg::ecs::Entity b = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::QuadComponent>(b);

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
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawQuadInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
