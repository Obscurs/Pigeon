#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ConfigLoaderSystem.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no SandboxConfigSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::CreatesConfigOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<sbx::SandboxConfigSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<sbx::SandboxConfigSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with SandboxConfigSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::DoesNotDuplicateWhenConfigExists")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		// First frame: the system itself creates the SandboxConfigSingletonComponent
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// Second frame: the config already exists, so the guard fires and no duplicate is added.
		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<sbx::SandboxConfigSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: every UUID field in the loaded config is populated
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::LoadedConfigHasNonNullUUIDs")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<sbx::SandboxConfigSingletonComponent>();
		REQUIRE(view.size() == 1);

		const sbx::SandboxConfigSingletonComponent& cfg =
			view.get<sbx::SandboxConfigSingletonComponent>(view.front());

		CHECK(!cfg.m_DefaultFontID.IsNull());
		CHECK(!cfg.m_BoldFontID.IsNull());
		CHECK(!cfg.m_SpriteTextureID.IsNull());
		CHECK(!cfg.m_TexturedQuadTextureID.IsNull());
		CHECK(!cfg.m_MainLayoutID.IsNull());
		CHECK(!cfg.m_ToggleButtonID.IsNull());
		CHECK(!cfg.m_TogglePanelID.IsNull());
		CHECK(!cfg.m_StatusTextID.IsNull());
		CHECK(!cfg.m_CloseButtonID.IsNull());
		CHECK(!cfg.m_CloseTargetID.IsNull());
		CHECK(!cfg.m_ButtonImageID.IsNull());
		CHECK(!cfg.m_ButtonHoverImageID.IsNull());
		CHECK(!cfg.m_ButtonPressedImageID.IsNull());
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ConfigLoaderSystem::DeclareAccessIsCorrect")
	{
		sbx::ConfigLoaderSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
