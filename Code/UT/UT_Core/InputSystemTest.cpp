#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/InputSystem.h>
#include <Pigeon/Core/InputStateSingletonComponent.h>
#include <Pigeon/Core/KeyCodes.h>
#include <Pigeon/Core/KeyPressedEventComponent.h>
#include <Pigeon/Core/KeyReleasedEventComponent.h>
#include <Pigeon/Core/KeyTypedEventComponent.h>
#include <Pigeon/Core/MouseButtonPressedEventComponent.h>
#include <Pigeon/Core/MouseButtonReleasedEventComponent.h>
#include <Pigeon/Core/MouseMovedEventComponent.h>
#include <Pigeon/Core/MouseScrolledEventComponent.h>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no InputStateSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::CreatesInputStateOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::InputStateSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::InputStateSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyPressedEventComponent -> key appears in m_KeysPressed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyPressedEventAddsToKeysPressed")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		// Seed InputStateSingletonComponent.
		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);

		// Create a key-pressed event entity.
		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyPressedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pg::PG_KEY_A;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pg::PG_KEY_A) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyReleasedEventComponent -> key moves to m_KeysReleased
	// and is removed from m_KeysPressed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyReleasedEventMovesToKeysReleased")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);
		// Pre-populate as if key was already pressed.
		state.m_KeysPressed[pg::PG_KEY_A] = 3;

		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyReleasedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::KeyReleasedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pg::PG_KEY_A;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& stateAfter =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysPressed.count(pg::PG_KEY_A) == 0);
		CHECK(stateAfter.m_KeysReleased.count(pg::PG_KEY_A) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyTypedEventComponent -> key appears in m_KeysTyped
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyTypedEventAddsToKeysTyped")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::KeyTypedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::KeyTypedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pg::PG_KEY_B;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		REQUIRE(!state.m_KeysTyped.empty());
		CHECK(state.m_KeysTyped[0] == pg::PG_KEY_B);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseMovedEventComponent -> m_MousePos updated
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseMovedEventUpdatesMousePos")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::MouseMovedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::MouseMovedEventComponent>(evtEnt);
		evtComp.m_MouseX = 123.f;
		evtComp.m_MouseY = 456.f;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_MousePos.x == 123.f);
		CHECK(state.m_MousePos.y == 456.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseButtonPressedEventComponent -> key added to m_KeysPressed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseButtonPressedAddsToKeysPressed")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::MouseButtonPressedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::MouseButtonPressedEventComponent>(evtEnt);
		evtComp.m_Button = pg::PG_MOUSE_BUTTON_LEFT;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pg::PG_MOUSE_BUTTON_LEFT) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseButtonReleasedEventComponent -> button moves to released
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseButtonReleasedAddsToKeysReleased")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);
		state.m_KeysPressed[pg::PG_MOUSE_BUTTON_LEFT] = 2;

		entt::entity evtEnt = pg::World::GetRegistryDirect().create();
		pg::MouseButtonReleasedEventComponent& evtComp =
			pg::World::GetRegistryDirect().emplace<pg::MouseButtonReleasedEventComponent>(evtEnt);
		evtComp.m_Button = pg::PG_MOUSE_BUTTON_LEFT;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& stateAfter =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysPressed.count(pg::PG_MOUSE_BUTTON_LEFT) == 0);
		CHECK(stateAfter.m_KeysReleased.count(pg::PG_MOUSE_BUTTON_LEFT) > 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: keys released from previous frame are cleared each Update()
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::ReleasedKeysClearedEachFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);
		// Simulate a release that was set in a previous frame.
		state.m_KeysReleased[pg::PG_KEY_A] = 1;

		// No new release event — released map should be cleared.
		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& stateAfter =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysReleased.empty());
	}

	// ---------------------------------------------------------------------------
	// Edge case: key held multiple frames -> counter increments each frame
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::HeldKeyCounterIncrementsEachFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);
		// Seed the key as already pressed (counter = 1).
		state.m_KeysPressed[pg::PG_KEY_A] = 1;

		// Two frames with no new press event -> counter increments.
		world.Update(pg::Timestep(0));
		int frameTwo = pg::World::GetRegistryDirect()
			.get<pg::InputStateSingletonComponent>(stateEnt)
			.m_KeysPressed[pg::PG_KEY_A];
		CHECK(frameTwo == 2);

		world.Update(pg::Timestep(0));
		int frameThree = pg::World::GetRegistryDirect()
			.get<pg::InputStateSingletonComponent>(stateEnt)
			.m_KeysPressed[pg::PG_KEY_A];
		CHECK(frameThree == 3);
	}

	// ---------------------------------------------------------------------------
	// Edge case: multiple events in one frame all processed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MultipleEventComponentsInOneFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::InputSystem>());

		entt::entity stateEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::InputStateSingletonComponent>(stateEnt);

		// Two key pressed events.
		entt::entity e1 = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(e1).m_KeyCode = pg::PG_KEY_A;

		entt::entity e2 = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::KeyPressedEventComponent>(e2).m_KeyCode = pg::PG_KEY_B;

		// One mouse-moved event.
		entt::entity e3 = pg::World::GetRegistryDirect().create();
		pg::MouseMovedEventComponent& moved = pg::World::GetRegistryDirect().emplace<pg::MouseMovedEventComponent>(e3);
		moved.m_MouseX = 77.f;
		moved.m_MouseY = 88.f;

		world.Update(pg::Timestep(0));

		const pg::InputStateSingletonComponent& state =
			pg::World::GetRegistryDirect().get<pg::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pg::PG_KEY_A) > 0);
		CHECK(state.m_KeysPressed.count(pg::PG_KEY_B) > 0);
		CHECK(state.m_MousePos.x == 77.f);
		CHECK(state.m_MousePos.y == 88.f);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: all expected component types are declared
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::DeclareAccessIsCorrect")
	{
		pg::InputSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyReleasedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyTypedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseMovedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseButtonPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseButtonReleasedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::MouseScrolledEventComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
