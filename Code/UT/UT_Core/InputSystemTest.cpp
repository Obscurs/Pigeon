#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>
#include <Pigeon/Core/EventComponent.h>
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

namespace
{
	// The InputSystem owns the InputStateSingletonComponent (it is in the system's
	// addSet), so the test must let the system create it instead of seeding it. The
	// first Update with no events makes the system create the singleton via deferred
	// add; it is visible once the frame's deferred requests are flushed. Returns the
	// entity holding the freshly created state.
	pg::ecs::Entity PrimeInputState(pg::World& world)
	{
		world.Update(pg::Timestep(0));
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<pg::InputStateSingletonComponent>();
		REQUIRE(view.size() == 1);
		return view.front();
	}

	// Input events reach the system as dedicated entities tagged with EventComponent,
	// so World::ClearEvents() destroys them at the end of the frame and they are not
	// reprocessed on later frames. Each helper mirrors that production shape.
	void EmplaceKeyPressed(int keyCode)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		registry.emplace<pg::KeyPressedEventComponent>(evtEnt).m_KeyCode = keyCode;
	}

	void EmplaceKeyReleased(int keyCode)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		registry.emplace<pg::KeyReleasedEventComponent>(evtEnt).m_KeyCode = keyCode;
	}

	void EmplaceKeyTyped(int keyCode)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		registry.emplace<pg::KeyTypedEventComponent>(evtEnt).m_KeyCode = keyCode;
	}

	void EmplaceMouseMoved(float x, float y)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		pg::MouseMovedEventComponent& moved = registry.emplace<pg::MouseMovedEventComponent>(evtEnt);
		moved.m_MouseX = x;
		moved.m_MouseY = y;
	}

	void EmplaceMouseButtonPressed(int button)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		registry.emplace<pg::MouseButtonPressedEventComponent>(evtEnt).m_Button = button;
	}

	void EmplaceMouseButtonReleased(int button)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity evtEnt = registry.create();
		registry.emplace<pg::EventComponent>(evtEnt);
		registry.emplace<pg::MouseButtonReleasedEventComponent>(evtEnt).m_Button = button;
	}
} // namespace

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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		EmplaceKeyPressed(pg::PG_KEY_A);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		// Drive the key into the pressed state through the system, then release it.
		EmplaceKeyPressed(pg::PG_KEY_A);
		world.Update(pg::Timestep(0));

		EmplaceKeyReleased(pg::PG_KEY_A);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		EmplaceKeyTyped(pg::PG_KEY_B);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		EmplaceMouseMoved(123.f, 456.f);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		EmplaceMouseButtonPressed(pg::PG_MOUSE_BUTTON_LEFT);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		// Press the button through the system first, then release it.
		EmplaceMouseButtonPressed(pg::PG_MOUSE_BUTTON_LEFT);
		world.Update(pg::Timestep(0));

		EmplaceMouseButtonReleased(pg::PG_MOUSE_BUTTON_LEFT);
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		// Produce a release so m_KeysReleased holds an entry going into the next frame.
		EmplaceKeyPressed(pg::PG_KEY_A);
		world.Update(pg::Timestep(0));
		EmplaceKeyReleased(pg::PG_KEY_A);
		world.Update(pg::Timestep(0));

		// A frame with no events must clear the released map.
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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		// Press the key once (counter = 1), then hold it across frames with no events.
		EmplaceKeyPressed(pg::PG_KEY_A);
		world.Update(pg::Timestep(0));

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

		pg::ecs::Entity stateEnt = PrimeInputState(world);

		EmplaceKeyPressed(pg::PG_KEY_A);
		EmplaceKeyPressed(pg::PG_KEY_B);
		EmplaceMouseMoved(77.f, 88.f);

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
