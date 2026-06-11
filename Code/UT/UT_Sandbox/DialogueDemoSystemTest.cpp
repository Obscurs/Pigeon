#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/DialogueDemoSingletonComponent.h"
#include "Sandbox/DialogueDemoSystem.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"

#include <cstring>
#include <string>

namespace
{
	// SandboxConfigSingletonComponent is added in production by ConfigLoaderSystem (a different system),
	// so the test seeds it directly with the dialogue element id the demo targets.
	sbx::SandboxConfigSingletonComponent& SeedConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity ent = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(ent);
		cfg.m_DialogueTextID = pg::UUID::Generate();
		return cfg;
	}

	// BaseComponent is added in production by the engine UIControlSystem; seed the dialogue element
	// directly so the demo can resolve it by UUID.
	pg::ecs::Entity MakeDialogueElement(pg::ecs::Registry& registry, const pg::UUID& uuid)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::ui::BaseComponent>(ent).m_UUID = uuid;
		return ent;
	}

	sbx::DialogueDemoSingletonComponent& GetDemo(pg::ecs::Registry& registry)
	{
		auto view = registry.view<sbx::DialogueDemoSingletonComponent>();
		return view.get<sbx::DialogueDemoSingletonComponent>(view.front());
	}

	void SetBuffer(sbx::DialogueDemoSingletonComponent& demo, const char* text)
	{
		std::memcpy(demo.m_TextBuffer, text, std::strlen(text) + 1);
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without the sandbox config the system does nothing (no demo state).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::DialogueDemoSingletonComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: with the config present the system seeds its demo state once with a
	// default line and a positive reveal speed.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::SeedsDemoStateOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		SeedConfig(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::DialogueDemoSingletonComponent>();
		REQUIRE(view.size() == 1);
		const sbx::DialogueDemoSingletonComponent& demo = view.get<sbx::DialogueDemoSingletonComponent>(view.front());
		CHECK(demo.m_CharsPerSecond > 0.f);
		CHECK(std::strlen(demo.m_TextBuffer) > 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the reveal advances over time and is pushed onto the dialogue element
	// as a reveal command carrying the current text. 10 chars/sec over 0.5s -> 5 chars.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::AdvancesRevealAndDrivesElement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity element = MakeDialogueElement(pg::World::GetRegistryDirect(), cfg.m_DialogueTextID);

		world.Update(pg::Timestep(0)); // seeds the demo state

		sbx::DialogueDemoSingletonComponent& demo = GetDemo(pg::World::GetRegistryDirect());
		SetBuffer(demo, "ABCDEFGHIJ");
		demo.m_CharsPerSecond = 10.f;

		world.Update(pg::Timestep(500)); // 0.5s

		REQUIRE(pg::World::GetRegistryDirect().any_of<pg::ui::UIUpdateTextRevealOneFrameComponent>(element));
		const pg::ui::UIUpdateTextRevealOneFrameComponent& reveal =
			pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextRevealOneFrameComponent>(element);
		CHECK(reveal.m_Text == "ABCDEFGHIJ");
		CHECK(reveal.m_VisibleChars == 5);
	}

	// ---------------------------------------------------------------------------
	// Core behaviour: editing the text restarts the reveal from the first character.
	// After fully revealing one line, swapping the text drops the visible count back to
	// the freshly-advanced amount for the new line.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::RestartsRevealWhenTextChanges")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity element = MakeDialogueElement(pg::World::GetRegistryDirect(), cfg.m_DialogueTextID);

		world.Update(pg::Timestep(0)); // seed

		sbx::DialogueDemoSingletonComponent& demo = GetDemo(pg::World::GetRegistryDirect());
		SetBuffer(demo, "ABCDEFGHIJ");
		demo.m_CharsPerSecond = 10.f;

		world.Update(pg::Timestep(2000)); // 2s -> fully revealed (10 of 10)
		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextRevealOneFrameComponent>(element).m_VisibleChars == 10);

		// Edit the line: the reveal must restart from the beginning.
		SetBuffer(GetDemo(pg::World::GetRegistryDirect()), "KLMNOPQRST");

		world.Update(pg::Timestep(100)); // 0.1s -> 1 char of the new line

		const pg::ui::UIUpdateTextRevealOneFrameComponent& reveal =
			pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextRevealOneFrameComponent>(element);
		CHECK(reveal.m_Text == "KLMNOPQRST");
		CHECK(reveal.m_VisibleChars == 1);
	}

	// ---------------------------------------------------------------------------
	// Edge: the reveal never runs past the end of the line, even for a large time step.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::ClampsRevealToTextLength")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		const sbx::SandboxConfigSingletonComponent& cfg = SeedConfig(pg::World::GetRegistryDirect());
		pg::ecs::Entity element = MakeDialogueElement(pg::World::GetRegistryDirect(), cfg.m_DialogueTextID);

		world.Update(pg::Timestep(0)); // seed

		sbx::DialogueDemoSingletonComponent& demo = GetDemo(pg::World::GetRegistryDirect());
		SetBuffer(demo, "ABC");
		demo.m_CharsPerSecond = 1000.f;

		world.Update(pg::Timestep(1000)); // 1s, far more than enough to reveal 3 chars

		CHECK(pg::World::GetRegistryDirect().get<pg::ui::UIUpdateTextRevealOneFrameComponent>(element).m_VisibleChars == 3);
	}

	// ---------------------------------------------------------------------------
	// Guard: with the demo state seeded but no dialogue element present, no reveal
	// command is emitted (and the system does not crash).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::NoRevealWithoutElement")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::DialogueDemoSystem>());

		SeedConfig(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));  // seed demo state
		world.Update(pg::Timestep(16)); // no element to drive

		CHECK(pg::World::GetRegistryDirect().view<pg::ui::UIUpdateTextRevealOneFrameComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.DialogueDemoSystem::DeclareAccessIsCorrect")
	{
		sbx::DialogueDemoSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ui::BaseComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::DialogueDemoSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::DialogueDemoSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::UIUpdateTextRevealOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
