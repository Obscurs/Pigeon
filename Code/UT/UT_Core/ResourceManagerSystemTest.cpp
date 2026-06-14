#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/ResourceManagerSystem.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/RegisterGeneratedTextureRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: first Update() with no ResourceMapSingletonComponent ->
	// system creates one via deferred add. Visible next frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::CreatesResourceMapOnFirstFrame")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		auto viewBefore = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		CHECK(viewBefore.size() == 0);

		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Guard: second Update() with ResourceMapSingletonComponent already present ->
	// system does nothing (no duplicate created)
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DoesNotDuplicateWhenMapExists")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		// First frame: the system itself creates the ResourceMapSingletonComponent
		// (the test must not pre-create a component the system adds).
		world.Update(pg::Timestep(0));

		// Second frame: the map already exists, so the guard fires and no duplicate is added.
		world.Update(pg::Timestep(0));

		auto viewAfter = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		CHECK(viewAfter.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has a default texture entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasDefaultTexture")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		// Default texture must always be present in the texture map.
		CHECK(map.m_TextureMap.find(map.m_DefaultTexture) != map.m_TextureMap.end());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has at least one shader entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasShaders")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		CHECK(!map.m_ShaderMap.empty());
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has at least one sound clip entry
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasSounds")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		REQUIRE(!map.m_SoundMap.empty());
		// Every loaded clip exposes its resolved path.
		for (const std::pair<const pg::UUID, pg::S_Ptr<pg::SoundClip>>& entry : map.m_SoundMap)
		{
			CHECK(entry.second != nullptr);
			CHECK(!entry.second->GetPath().empty());
		}
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has the JSON asset declared in the manifest,
	// keyed by its UUID and holding the parsed file content.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasJSONAsset")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		REQUIRE(!map.m_JSONMap.empty());

		std::unordered_map<pg::UUID, nlohmann::json>::const_iterator it =
			map.m_JSONMap.find(pg::UUID("d4000000-0000-4000-8000-000000000001"));
		REQUIRE(it != map.m_JSONMap.end());

		const nlohmann::json& asset = it->second;
		CHECK(asset["name"].get<std::string>() == "test-asset");
		CHECK(asset["value"].get<int>() == 42);
		CHECK(asset["nested"]["flag"].get<bool>() == true);
	}

	// ---------------------------------------------------------------------------
	// Happy path: loaded resource map has the 3D model declared in the manifest,
	// keyed by its UUID and holding parsed triangle geometry.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasModel")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		REQUIRE(!map.m_ModelMap.empty());

		std::unordered_map<pg::UUID, pg::S_Ptr<pg::Model>>::const_iterator it =
			map.m_ModelMap.find(pg::UUID("e5000000-0000-4000-8000-000000000001"));
		REQUIRE(it != map.m_ModelMap.end());
		REQUIRE(it->second != nullptr);
		// The test quad is one face -> four vertices, fan-triangulated into six indices.
		CHECK(it->second->GetVertices().size() == 4);
		CHECK(it->second->GetIndices().size() == 6);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the engine render target declared in the manifest is created and
	// its colour buffer is registered in the texture map under the same UUID, so 2D
	// draws can sample the 3D image as a texture.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasRenderTarget")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		const pg::UUID renderTargetID("c3000000-0000-4000-8000-000000000002");

		std::unordered_map<pg::UUID, pg::S_Ptr<pg::RenderTarget>>::const_iterator rt =
			map.m_RenderTargetMap.find(renderTargetID);
		REQUIRE(rt != map.m_RenderTargetMap.end());
		REQUIRE(rt->second != nullptr);
		CHECK(rt->second->GetColorTexture() != nullptr);

		// The same UUID resolves to the render target's colour buffer in the texture map.
		std::unordered_map<pg::UUID, pg::MappedTexture>::const_iterator tex =
			map.m_TextureMap.find(renderTargetID);
		REQUIRE(tex != map.m_TextureMap.end());
		CHECK(tex->second.m_Texture != nullptr);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the diffusion model assets declared in the manifest are recorded
	// as resolved paths (checkpoint / LoRA / ControlNet are loaded by the backend,
	// not the engine, so the map only holds their paths).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasDiffusionModelPaths")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		std::unordered_map<pg::UUID, std::string>::const_iterator checkpoint =
			map.m_CheckpointMap.find(pg::UUID("f6000000-0000-4000-8000-000000000001"));
		REQUIRE(checkpoint != map.m_CheckpointMap.end());
		CHECK(checkpoint->second.find("dummy_checkpoint.safetensors") != std::string::npos);

		std::unordered_map<pg::UUID, std::string>::const_iterator lora =
			map.m_LoraMap.find(pg::UUID("f6000000-0000-4000-8000-000000000002"));
		REQUIRE(lora != map.m_LoraMap.end());
		CHECK(lora->second.find("dummy_lora.safetensors") != std::string::npos);

		std::unordered_map<pg::UUID, std::string>::const_iterator controlNet =
			map.m_ControlNetMap.find(pg::UUID("f6000000-0000-4000-8000-000000000003"));
		REQUIRE(controlNet != map.m_ControlNetMap.end());
		CHECK(controlNet->second.find("dummy_control.safetensors") != std::string::npos);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the OpenPose skeleton declared in the manifest is parsed into the
	// skeleton map, keyed by UUID, with its canvas size populated.
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasOpenPoseSkeleton")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		std::unordered_map<pg::UUID, pg::S_Ptr<pg::OpenPoseSkeleton>>::const_iterator skeleton =
			map.m_OpenPoseSkeletonMap.find(pg::UUID("f6000000-0000-4000-8000-000000000004"));
		REQUIRE(skeleton != map.m_OpenPoseSkeletonMap.end());
		REQUIRE(skeleton->second != nullptr);
		CHECK(skeleton->second->GetCanvasWidth() == Approx(512.0f));
		CHECK(skeleton->second->GetKeypoints()[static_cast<size_t>(pg::EOpenPoseJoint::Nose)].m_Confidence == Approx(1.0f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: the GGUF language model declared in the manifest is recorded as a
	// resolved path (the inference backend loads the weights, not the engine, so the
	// map only holds the path).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::LoadedMapHasLanguageModelPath")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);

		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		std::unordered_map<pg::UUID, std::string>::const_iterator model =
			map.m_LanguageModelMap.find(pg::UUID("f6000000-0000-4000-8000-000000000005"));
		REQUIRE(model != map.m_LanguageModelMap.end());
		CHECK(model->second.find("dummy_model.gguf") != std::string::npos);
		CHECK(model->second.find("TextGeneration") != std::string::npos);
	}

	// ---------------------------------------------------------------------------
	// Generated-texture registration: a RegisterGeneratedTextureRequest is drained
	// into the texture map under its caller-assigned UUID (a generated image becomes
	// an ordinary texture).
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::RegistersGeneratedTexture")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::ResourceManagerSystem>());

		// Frame 1 creates the resource map (deferred); frame 2 makes it visible.
		world.Update(pg::Timestep(0));
		world.Update(pg::Timestep(0));

		const pg::UUID generatedID("f7000000-0000-4000-8000-000000000001");
		pg::RegisterGeneratedTextureRequestOneFrameComponent registration;
		registration.m_TextureID = generatedID;
		registration.m_Image.m_Width = 4;
		registration.m_Image.m_Height = 4;
		registration.m_Image.m_Pixels.assign(static_cast<size_t>(4) * 4 * 3, 200);
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		registry.emplace<pg::RegisterGeneratedTextureRequestOneFrameComponent>(registry.create(), registration);

		world.Update(pg::Timestep(0));

		auto view = pg::World::GetRegistryDirect().view<pg::ResourceMapSingletonComponent>();
		REQUIRE(view.size() == 1);
		const pg::ResourceMapSingletonComponent& map =
			view.get<pg::ResourceMapSingletonComponent>(view.front());

		std::unordered_map<pg::UUID, pg::MappedTexture>::const_iterator tex = map.m_TextureMap.find(generatedID);
		REQUIRE(tex != map.m_TextureMap.end());
		CHECK(tex->second.m_Texture != nullptr);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access
	// ---------------------------------------------------------------------------
	TEST_CASE("Core.ResourceManagerSystem::DeclareAccessIsCorrect")
	{
		pg::ResourceManagerSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::RegisterGeneratedTextureRequestOneFrameComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
	}

} // namespace CatchTestsetFail
