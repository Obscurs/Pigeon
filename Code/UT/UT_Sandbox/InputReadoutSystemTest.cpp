#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/InputStateSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/InputReadoutSystem.h"
#include "Sandbox/InputReadoutTagComponent.h"
#include "Sandbox/LabelComponent.h"

#include <string>

namespace
{
	// InputStateSingletonComponent is added in production by InputSystem (a different system).
	pg::InputStateSingletonComponent& SeedInput(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity ent = registry.create();
		pg::InputStateSingletonComponent& input = registry.emplace<pg::InputStateSingletonComponent>(ent);
		input.m_MousePos = { 70.f, 80.f };
		input.m_MouseScroll = { 3.f, 9.f };
		input.m_KeysPressed[65] = 1;
		input.m_KeysPressed[66] = 1; // two keys held
		return input;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: the tagged label's text reflects the live input state.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.InputReadoutSystem::WritesInputStateToTaggedLabel")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::InputReadoutSystem>());

		SeedInput(pg::World::GetRegistryDirect());

		// LabelComponent + InputReadoutTagComponent are added in production by SceneSetupSystem.
		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(ent);
		pg::World::GetRegistryDirect().emplace<sbx::InputReadoutTagComponent>(ent);

		world.Update(pg::Timestep(0));

		const std::string& text = pg::World::GetRegistryDirect().get<sbx::LabelComponent>(ent).m_Text;
		CHECK(text.find("70") != std::string::npos);   // mouse x
		CHECK(text.find("80") != std::string::npos);   // mouse y
		CHECK(text.find("2") != std::string::npos);    // held-key count
		CHECK(text.find("Mouse") != std::string::npos);
		CHECK(text.find("Scroll") != std::string::npos);
	}

	// ---------------------------------------------------------------------------
	// Guard: without input state the label text is left untouched.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.InputReadoutSystem::NoOpWithoutInputState")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::InputReadoutSystem>());

		pg::ecs::Entity ent = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(ent);
		pg::World::GetRegistryDirect().emplace<sbx::InputReadoutTagComponent>(ent);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().get<sbx::LabelComponent>(ent).m_Text.empty());
	}

	// ---------------------------------------------------------------------------
	// Edge case: only labels carrying the readout tag are rewritten.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.InputReadoutSystem::IgnoresUntaggedLabels")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::InputReadoutSystem>());

		SeedInput(pg::World::GetRegistryDirect());

		pg::ecs::Entity untagged = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::LabelComponent>(untagged);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().get<sbx::LabelComponent>(untagged).m_Text.empty());
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.InputReadoutSystem::DeclareAccessIsCorrect")
	{
		sbx::InputReadoutSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::InputStateSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::InputReadoutTagComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::LabelComponent))) > 0);
	}

} // namespace CatchTestsetFail
