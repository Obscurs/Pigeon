#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Renderer/ModelComponent.h"
#include "Pigeon/Renderer/OrthographicCameraComponent.h"
#include "Pigeon/Renderer/PerspectiveCameraComponent.h"
#include "Pigeon/Renderer/SpriteAnimationComponent.h"
#include "Pigeon/Renderer/SpriteComponent.h"
#include "Pigeon/Renderer/SpriteSheet.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/CharacterTagComponent.h"
#include "Sandbox/InputReadoutTagComponent.h"
#include "Sandbox/LabelComponent.h"
#include "Sandbox/ModelSpinComponent.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"
#include "Sandbox/SceneReadySingletonComponent.h"
#include "Sandbox/SceneSetupSystem.h"

namespace
{
	void SeedConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		sbx::SandboxConfigSingletonComponent& cfg = registry.emplace<sbx::SandboxConfigSingletonComponent>(cfgEnt);
		cfg.m_MainLayoutID = pg::UUID::Generate();
		cfg.m_CharacterTextureID = pg::UUID::Generate();
		cfg.m_MonkeyModelID = pg::UUID::Generate();
	}

	void SeedEngineConfig(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity cfgEnt = registry.create();
		pg::EngineConfigSingletonComponent& cfg = registry.emplace<pg::EngineConfigSingletonComponent>(cfgEnt);
		cfg.m_Render3DTargetID = pg::UUID::Generate();
	}

	void SeedResourceMap(pg::ecs::Registry& registry)
	{
		pg::ecs::Entity resEnt = registry.create();
		registry.emplace<pg::ResourceMapSingletonComponent>(resEnt);
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Guard: without the config the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::NoOpWithoutConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SceneReadySingletonComponent>().size() == 0);
		CHECK(pg::World::GetRegistryDirect().view<pg::OrthographicCameraComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: without the resource map the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::NoOpWithoutResourceMap")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedEngineConfig(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SceneReadySingletonComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Guard: without the engine config (render target id) the system does nothing.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::NoOpWithoutEngineConfig")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<sbx::SceneReadySingletonComponent>().size() == 0);
	}

	// ---------------------------------------------------------------------------
	// Happy path: with config, engine config, and resource map present, the scene
	// is built.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::BuildsSceneWhenReady")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedEngineConfig(pg::World::GetRegistryDirect());
		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		auto cameraView = pg::World::GetRegistryDirect().view<pg::OrthographicCameraComponent>();
		REQUIRE(cameraView.size() == 1);
		const pg::OrthographicCameraComponent& camera = cameraView.get<pg::OrthographicCameraComponent>(cameraView.front());
		CHECK(std::fabs(camera.m_AspectRatio - 1280.f / 720.f) < 1e-4f);

		// Six sprites: the static demo sprite, the animated character, the 3D display sprite, and the
		// three text-to-image pipeline display sprites (restyled background, pose hint, composite).
		CHECK(pg::World::GetRegistryDirect().view<pg::SpriteComponent>().size() == 6);
		// One 3D camera looking at the spinning model.
		CHECK(pg::World::GetRegistryDirect().view<pg::PerspectiveCameraComponent>().size() == 1);
		CHECK(pg::World::GetRegistryDirect().view<sbx::LabelComponent>().size() == 3);
		CHECK(pg::World::GetRegistryDirect().view<sbx::InputReadoutTagComponent>().size() == 1);
		CHECK(pg::World::GetRegistryDirect().view<sbx::SceneReadySingletonComponent>().size() == 1);
		CHECK(pg::World::GetRegistryDirect().view<pg::ui::LoadLayoutEvent>().size() == 1);
	}

	// ---------------------------------------------------------------------------
	// The animated character is created: one tagged entity with a sprite using the
	// configured character texture, plus an 8x8 sprite-sheet animation that starts
	// idle.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::CreatesAnimatedCharacter")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedEngineConfig(pg::World::GetRegistryDirect());
		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		const sbx::SandboxConfigSingletonComponent& cfg = registry.view<sbx::SandboxConfigSingletonComponent>().get<sbx::SandboxConfigSingletonComponent>(registry.view<sbx::SandboxConfigSingletonComponent>().front());

		CHECK(registry.view<sbx::CharacterTagComponent>().size() == 1);
		CHECK(registry.view<pg::SpriteAnimationComponent>().size() == 1);

		auto characterView = registry.view<sbx::CharacterTagComponent, pg::SpriteComponent, pg::SpriteAnimationComponent>();
		int characterCount = 0;
		for (pg::ecs::Entity ent : characterView)
		{
			++characterCount;
			const pg::SpriteComponent& sprite = characterView.get<pg::SpriteComponent>(ent);
			const pg::SpriteAnimationComponent& animation = characterView.get<pg::SpriteAnimationComponent>(ent);
			CHECK(sprite.m_TextureID == cfg.m_CharacterTextureID);
			CHECK(animation.m_Sheet.GetColumns() == 8);
			CHECK(animation.m_Sheet.GetRows() == 8);
			CHECK(animation.m_FrameCount == 8);
			CHECK(animation.m_Playing == false);
		}
		CHECK(characterCount == 1);
	}

	// ---------------------------------------------------------------------------
	// A 3D model entity is created referencing the configured model resource by
	// UUID and carrying a spin so the 3D pass renders a rotating model. It is not a
	// 2D object: it has no Sprite next to its Model.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::CreatesSpinningModelEntity")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedEngineConfig(pg::World::GetRegistryDirect());
		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		const sbx::SandboxConfigSingletonComponent& cfg = registry.view<sbx::SandboxConfigSingletonComponent>().get<sbx::SandboxConfigSingletonComponent>(registry.view<sbx::SandboxConfigSingletonComponent>().front());

		auto modelView = registry.view<pg::ModelComponent>();
		REQUIRE(modelView.size() == 1);
		const pg::ecs::Entity modelEnt = modelView.front();
		const pg::ModelComponent& model = modelView.get<pg::ModelComponent>(modelEnt);
		CHECK(model.m_ModelID == cfg.m_MonkeyModelID);

		// The model spins and lives only in the 3D scene (no Sprite on the same entity).
		CHECK(registry.all_of<sbx::ModelSpinComponent>(modelEnt));
		CHECK_FALSE(registry.all_of<pg::SpriteComponent>(modelEnt));
		CHECK(registry.view<sbx::ModelSpinComponent>().size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Edge case: the system runs only once (no duplicate scene on a second frame).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::RunsOnlyOnce")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());

		SeedConfig(pg::World::GetRegistryDirect());
		SeedEngineConfig(pg::World::GetRegistryDirect());
		SeedResourceMap(pg::World::GetRegistryDirect());

		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::OrthographicCameraComponent>().size() == 1);
		CHECK(pg::World::GetRegistryDirect().view<pg::PerspectiveCameraComponent>().size() == 1);
		CHECK(pg::World::GetRegistryDirect().view<sbx::LabelComponent>().size() == 3);
		CHECK(pg::World::GetRegistryDirect().view<sbx::SceneReadySingletonComponent>().size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.SceneSetupSystem::DeclareAccessIsCorrect")
	{
		sbx::SceneSetupSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SandboxConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SceneReadySingletonComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::OrthographicCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::PerspectiveCameraComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SpriteComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SpriteAnimationComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ModelComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::ModelSpinComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::CharacterTagComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::LabelComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::InputReadoutTagComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ui::LoadLayoutEvent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::SceneReadySingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
