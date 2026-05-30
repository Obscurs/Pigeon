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
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		auto viewBefore = pig::World::GetRegistryDirect().view<pig::InputStateSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pig::Timestep(0));

		auto viewAfter = pig::World::GetRegistryDirect().view<pig::InputStateSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyPressedEventComponent -> key appears in m_KeysPressed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyPressedEventAddsToKeysPressed")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		// Seed InputStateSingletonComponent.
		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);

		// Create a key-pressed event entity.
		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyPressedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pig::PG_KEY_A;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pig::PG_KEY_A) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyReleasedEventComponent -> key moves to m_KeysReleased
	// and is removed from m_KeysPressed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyReleasedEventMovesToKeysReleased")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);
		// Pre-populate as if key was already pressed.
		state.m_KeysPressed[pig::PG_KEY_A] = 3;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyReleasedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::KeyReleasedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pig::PG_KEY_A;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& stateAfter =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysPressed.count(pig::PG_KEY_A) == 0);
		CHECK(stateAfter.m_KeysReleased.count(pig::PG_KEY_A) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: KeyTypedEventComponent -> key appears in m_KeysTyped
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::KeyTypedEventAddsToKeysTyped")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::KeyTypedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::KeyTypedEventComponent>(evtEnt);
		evtComp.m_KeyCode = pig::PG_KEY_B;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		REQUIRE(!state.m_KeysTyped.empty());
		CHECK(state.m_KeysTyped[0] == pig::PG_KEY_B);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseMovedEventComponent -> m_MousePos updated
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseMovedEventUpdatesMousePos")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::MouseMovedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::MouseMovedEventComponent>(evtEnt);
		evtComp.m_MouseX = 123.f;
		evtComp.m_MouseY = 456.f;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_MousePos.x == 123.f);
		CHECK(state.m_MousePos.y == 456.f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseButtonPressedEventComponent -> key added to m_KeysPressed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseButtonPressedAddsToKeysPressed")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::MouseButtonPressedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::MouseButtonPressedEventComponent>(evtEnt);
		evtComp.m_Button = pig::PG_MOUSE_BUTTON_LEFT;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pig::PG_MOUSE_BUTTON_LEFT) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: MouseButtonReleasedEventComponent -> button moves to released
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MouseButtonReleasedAddsToKeysReleased")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);
		state.m_KeysPressed[pig::PG_MOUSE_BUTTON_LEFT] = 2;

		entt::entity evtEnt = pig::World::GetRegistryDirect().create();
		pig::MouseButtonReleasedEventComponent& evtComp =
			pig::World::GetRegistryDirect().emplace<pig::MouseButtonReleasedEventComponent>(evtEnt);
		evtComp.m_Button = pig::PG_MOUSE_BUTTON_LEFT;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& stateAfter =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysPressed.count(pig::PG_MOUSE_BUTTON_LEFT) == 0);
		CHECK(stateAfter.m_KeysReleased.count(pig::PG_MOUSE_BUTTON_LEFT) > 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: keys released from previous frame are cleared each Update()
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::ReleasedKeysClearedEachFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);
		// Simulate a release that was set in a previous frame.
		state.m_KeysReleased[pig::PG_KEY_A] = 1;

		// No new release event — released map should be cleared.
		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& stateAfter =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(stateAfter.m_KeysReleased.empty());
	}

	// ---------------------------------------------------------------------------
	// Edge case: key held multiple frames -> counter increments each frame
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::HeldKeyCounterIncrementsEachFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);
		// Seed the key as already pressed (counter = 1).
		state.m_KeysPressed[pig::PG_KEY_A] = 1;

		// Two frames with no new press event -> counter increments.
		world.Update(pig::Timestep(0));
		int frameTwo = pig::World::GetRegistryDirect()
			.get<pig::InputStateSingletonComponent>(stateEnt)
			.m_KeysPressed[pig::PG_KEY_A];
		CHECK(frameTwo == 2);

		world.Update(pig::Timestep(0));
		int frameThree = pig::World::GetRegistryDirect()
			.get<pig::InputStateSingletonComponent>(stateEnt)
			.m_KeysPressed[pig::PG_KEY_A];
		CHECK(frameThree == 3);
	}

	// ---------------------------------------------------------------------------
	// Edge case: multiple events in one frame all processed
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::MultipleEventComponentsInOneFrame")
	{
		pig::World& world = pig::World::Create();
		world.RegisterSystem(std::make_unique<pig::InputSystem>());

		entt::entity stateEnt = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::InputStateSingletonComponent>(stateEnt);

		// Two key pressed events.
		entt::entity e1 = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(e1).m_KeyCode = pig::PG_KEY_A;

		entt::entity e2 = pig::World::GetRegistryDirect().create();
		pig::World::GetRegistryDirect().emplace<pig::KeyPressedEventComponent>(e2).m_KeyCode = pig::PG_KEY_B;

		// One mouse-moved event.
		entt::entity e3 = pig::World::GetRegistryDirect().create();
		pig::MouseMovedEventComponent& moved = pig::World::GetRegistryDirect().emplace<pig::MouseMovedEventComponent>(e3);
		moved.m_MouseX = 77.f;
		moved.m_MouseY = 88.f;

		world.Update(pig::Timestep(0));

		const pig::InputStateSingletonComponent& state =
			pig::World::GetRegistryDirect().get<pig::InputStateSingletonComponent>(stateEnt);
		CHECK(state.m_KeysPressed.count(pig::PG_KEY_A) > 0);
		CHECK(state.m_KeysPressed.count(pig::PG_KEY_B) > 0);
		CHECK(state.m_MousePos.x == 77.f);
		CHECK(state.m_MousePos.y == 88.f);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: all expected component types are declared
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.InputSystem::DeclareAccessIsCorrect")
	{
		pig::InputSystem sys;
		pig::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pig::KeyPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::KeyReleasedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::KeyTypedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::MouseMovedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::MouseButtonPressedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::MouseButtonReleasedEventComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pig::MouseScrolledEventComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pig::InputStateSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pig::InputStateSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
