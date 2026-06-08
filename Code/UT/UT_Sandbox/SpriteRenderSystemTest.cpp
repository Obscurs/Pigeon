#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Pigeon/Transform/WorldTransformComponent.h"
#include "Sandbox/SpriteComponent.h"
#include "Sandbox/SpriteRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: a sprite with a resolved world transform emits one sprite draw
	// event carrying the world matrix, sort key, UVs, and texture (origin is zero).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::EmitsMatchingSpriteEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		sbx::SpriteComponent& sprite = pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(ent);
		sprite.m_TexCoordsRect = { 16.f, 16.f, 48.f, 48.f };
		sprite.m_TextureID = pg::UUID::Generate();
		pg::WorldTransformComponent& worldTransform = pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(ent);
		worldTransform.m_Matrix = glm::translate(glm::mat4(1.f), glm::vec3(1.f, 2.f, 0.f));
		worldTransform.m_SortKey = 2.f;

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawSpriteInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::DrawSpriteInFrameEvent& event = view.get<const pg::DrawSpriteInFrameEvent>(view.front());
		const pg::Sprite::Data& data = event.m_Sprite.GetData();
		CHECK(data.m_Transform == worldTransform.m_Matrix);
		CHECK(data.m_TexCoordsRect == sprite.m_TexCoordsRect);
		CHECK(data.m_Origin == glm::vec3(0.f));
		CHECK(data.m_TextureID == sprite.m_TextureID);
		CHECK(event.m_SortKey == Approx(2.f));
	}

	// ---------------------------------------------------------------------------
	// Guard: a sprite without a resolved world transform yet emits no event.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::NoWorldTransformNoEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(ent);

		world.UpdateRetainingEvents(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<const pg::DrawSpriteInFrameEvent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: no sprites -> no draw events.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::NoSpritesNoEvents")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		world.UpdateRetainingEvents(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<const pg::DrawSpriteInFrameEvent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Edge case: one draw event per sprite with a world transform.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::EmitsOneEventPerSprite")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		pg::ecs::Entity a = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(a);
		pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(a);
		pg::ecs::Entity b = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(b);
		pg::World::GetRegistryDirect().emplace<pg::WorldTransformComponent>(b);

		world.UpdateRetainingEvents(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<const pg::DrawSpriteInFrameEvent>().size() == 2);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::DeclareAccessIsCorrect")
	{
		sbx::SpriteRenderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SpriteComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::WorldTransformComponent))) > 0);
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
