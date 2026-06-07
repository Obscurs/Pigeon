#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
#include "Sandbox/SpriteComponent.h"
#include "Sandbox/SpriteRenderSystem.h"

#include <glm/gtc/matrix_transform.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: one sprite draw event per sprite, with every field copied across.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::EmitsMatchingSpriteEvent")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		// SpriteComponent is added in production by SceneSetupSystem (a different system).
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		sbx::SpriteComponent& sprite = pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(ent);
		sprite.m_Transform = glm::translate(glm::mat4(1.f), glm::vec3(1.f, 2.f, 0.f));
		sprite.m_TexCoordsRect = { 16.f, 16.f, 48.f, 48.f };
		sprite.m_Origin = { 0.5f, 0.5f, 0.f };
		sprite.m_TextureID = pg::UUID::Generate();

		world.UpdateRetainingEvents(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<const pg::DrawSpriteInFrameEvent>();
		REQUIRE(view.size() == 1);
		const pg::Sprite::Data& data = view.get<const pg::DrawSpriteInFrameEvent>(view.front()).m_Sprite.GetData();
		CHECK(data.m_Transform == sprite.m_Transform);
		CHECK(data.m_TexCoordsRect == sprite.m_TexCoordsRect);
		CHECK(data.m_Origin == sprite.m_Origin);
		CHECK(data.m_TextureID == sprite.m_TextureID);
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
	// Edge case: one draw event per sprite when several exist.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SpriteRenderSystem::EmitsOneEventPerSprite")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());

		pg::ecs::Entity a = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(a);
		pg::ecs::Entity b = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::SpriteComponent>(b);

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
		CHECK(decl.inframeAddSet.count(std::type_index(typeid(pg::DrawSpriteInFrameEvent))) > 0);
	}

} // namespace CatchTestsetFail
