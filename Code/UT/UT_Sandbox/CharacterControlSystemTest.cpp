#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/SetSpriteAnimationRequestOneFrameComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Sandbox/CharacterControlSystem.h"
#include "Sandbox/CharacterTagComponent.h"
#include "Sandbox/CharacterTransformRequestOneFrameComponent.h"

#include <cmath>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

namespace
{
	// Facing rows the system selects per direction (Down/Up/Left/Right). Mirrors the system's mapping;
	// asserted explicitly because the row index is the observable behaviour.
	constexpr int ROW_DOWN = 0;
	constexpr int ROW_UP = 1;
	constexpr int ROW_LEFT = 2;
	constexpr int ROW_RIGHT = 3;

	pg::InputStateSingletonComponent& MakeInput(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity ent = registry.create();
		return registry.emplace<pg::InputStateSingletonComponent>(ent);
	}

	// CharacterTagComponent is added in production by SceneSetupSystem and PositionComponent by the
	// engine TransformResolveSystem (both different systems), so the control test seeds them directly.
	pg::ecs::Entity MakeCharacter(pg::ecs::Registry& registry, const glm::vec3& position)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<sbx::CharacterTagComponent>(ent);
		registry.emplace<pg::PositionComponent>(ent).m_Position = position;
		return ent;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: no input state -> no requests emitted at all.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::NoInputStateNoRequests")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<sbx::CharacterTransformRequestOneFrameComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::SetSpriteAnimationRequestOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: input present but no character entity -> no requests emitted.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::NoCharacterNoRequests")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_RIGHT] = 1;

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<sbx::CharacterTransformRequestOneFrameComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::SetSpriteAnimationRequestOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Idle: no arrow held -> animation stops (playing false, row unchanged) and
	// no movement request is emitted.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::IdleStopsAnimationAndDoesNotMove")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect());
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		CHECK(pg::World::GetRegistryDirect().view<sbx::CharacterTransformRequestOneFrameComponent>().size() == 0);
		REQUIRE(pg::World::GetRegistryDirect().all_of<pg::SetSpriteAnimationRequestOneFrameComponent>(ent));
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Playing == false);
		CHECK(anim.m_SetRow == false);
	}

	// ---------------------------------------------------------------------------
	// Right arrow: moves +X by speed*dt and faces the right row, playing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::RightArrowMovesAndFacesRight")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_RIGHT] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000)); // 1s, speed 1.5 -> +1.5 on X

		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(move.m_Data.m_SetPosition);
		CHECK(FLOAT_EQ(move.m_Data.m_Position.x, 1.5f));
		CHECK(FLOAT_EQ(move.m_Data.m_Position.y, 0.f));
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Playing == true);
		CHECK(anim.m_SetRow == true);
		CHECK(anim.m_Row == ROW_RIGHT);
	}

	// ---------------------------------------------------------------------------
	// Left arrow: moves -X and faces the left row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::LeftArrowMovesAndFacesLeft")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_LEFT] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(FLOAT_EQ(move.m_Data.m_Position.x, -1.5f));
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Row == ROW_LEFT);
	}

	// ---------------------------------------------------------------------------
	// Up arrow: moves up on screen. World space is Y-up, so moving up increases
	// world Y, and faces the up row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::UpArrowMovesAndFacesUp")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_UP] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(FLOAT_EQ(move.m_Data.m_Position.y, 1.5f));
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Row == ROW_UP);
	}

	// ---------------------------------------------------------------------------
	// Down arrow: moves down on screen. World space is Y-up, so world Y decreases,
	// and faces the down row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::DownArrowMovesAndFacesDown")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_DOWN] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(FLOAT_EQ(move.m_Data.m_Position.y, -1.5f));
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Row == ROW_DOWN);
	}

	// ---------------------------------------------------------------------------
	// Diagonal: both axes move (direction normalised so diagonal isn't faster),
	// and facing favours the horizontal row.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::DiagonalNormalisedFacesHorizontal")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		pg::InputStateSingletonComponent& input = MakeInput(pg::World::GetRegistryDirect());
		input.m_KeysPressed[pg::PG_KEY_RIGHT] = 1;
		input.m_KeysPressed[pg::PG_KEY_UP] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 0.f, 0.f, 0.f });

		world.Update(pg::Timestep(1000));

		const float expected = 1.5f / std::sqrt(2.f);
		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(move.m_Data.m_Position.x == Approx(expected));
		CHECK(move.m_Data.m_Position.y == Approx(expected)); // up intent -> positive world Y (Y-up)
		const pg::SetSpriteAnimationRequestOneFrameComponent& anim = pg::World::GetRegistryDirect().get<pg::SetSpriteAnimationRequestOneFrameComponent>(ent);
		CHECK(anim.m_Row == ROW_RIGHT);
	}

	// ---------------------------------------------------------------------------
	// Movement is relative: the request adds to the character's current position.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::MovementAddsToCurrentPosition")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::CharacterControlSystem>());

		MakeInput(pg::World::GetRegistryDirect()).m_KeysPressed[pg::PG_KEY_RIGHT] = 1;
		pg::ecs::Entity ent = MakeCharacter(pg::World::GetRegistryDirect(), { 2.f, 3.f, 0.f });

		world.Update(pg::Timestep(1000));

		const sbx::CharacterTransformRequestOneFrameComponent& move = pg::World::GetRegistryDirect().get<sbx::CharacterTransformRequestOneFrameComponent>(ent);
		CHECK(FLOAT_EQ(move.m_Data.m_Position.x, 3.5f));
		CHECK(FLOAT_EQ(move.m_Data.m_Position.y, 3.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.CharacterControlSystem::DeclareAccessIsCorrect")
	{
		sbx::CharacterControlSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::CharacterTagComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::CharacterTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SetSpriteAnimationRequestOneFrameComponent))) > 0);
	}
} // namespace CatchTestsetFail
