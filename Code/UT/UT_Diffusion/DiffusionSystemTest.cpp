#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <chrono>
#include <thread>

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionBackendSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionJobSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionSystem.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/Diffusion/OpenPoseSkeleton.h"
#include "Pigeon/Diffusion/RegisterGeneratedTextureRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Platform/Testing/TestingDiffusionBackend.h"

namespace
{
	const pg::UUID k_TargetTexture("aaaaaaaa-0000-4000-8000-000000000001");
	const pg::UUID k_CheckpointID("aaaaaaaa-0000-4000-8000-000000000002");
	const pg::UUID k_LoraID("aaaaaaaa-0000-4000-8000-000000000003");
	const pg::UUID k_SkeletonID("aaaaaaaa-0000-4000-8000-000000000004");
	const pg::UUID k_InputImageID("aaaaaaaa-0000-4000-8000-000000000005");

	// Seeds the singletons DiffusionSystem only reads (added by ConfigLoader / ResourceManager in
	// production), with a small generation size and a checkpoint path so the backend loads.
	void SeedConfigAndResources(bool withCheckpoint)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		pg::EngineConfigSingletonComponent config;
		config.m_DiffusionWidth = 64;
		config.m_DiffusionHeight = 64;
		registry.emplace<pg::EngineConfigSingletonComponent>(registry.create(), config);

		pg::ResourceMapSingletonComponent resources;
		if (withCheckpoint)
		{
			resources.m_CheckpointMap[k_CheckpointID] = "Assets/App/ImageGeneration/checkpoint.safetensors";
		}
		resources.m_LoraMap[k_LoraID] = "Assets/App/ImageGeneration/lora.safetensors";
		resources.m_OpenPoseSkeletonMap[k_SkeletonID] = pg::OpenPoseSkeleton::CreateFromJsonString(
			"[{\"people\": [{\"pose_keypoints_2d\": [256.0, 100.0, 1.0, 256.0, 180.0, 1.0]}], \"canvas_width\": 512, \"canvas_height\": 512}]");
		// A small input image (8x8) the img2img test references; the system resizes it to the gen size.
		pg::Image input;
		input.m_Width = 8;
		input.m_Height = 8;
		input.m_Pixels.assign(static_cast<size_t>(8) * 8 * 3, 64);
		resources.m_InputImageMap[k_InputImageID] = input;
		registry.emplace<pg::ResourceMapSingletonComponent>(registry.create(), resources);
	}

	pg::DiffusionBackendSingletonComponent* GetBackend()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<pg::DiffusionBackendSingletonComponent>();
		if (view.empty())
		{
			return nullptr;
		}
		return &view.get<pg::DiffusionBackendSingletonComponent>(view.front());
	}

	// Spins the engine forward until the active job reports done (bounded). The mock backend completes
	// near-instantly, so this resolves in a few milliseconds.
	bool WaitForActiveJobDone(int maxMilliseconds)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		for (int i = 0; i < maxMilliseconds; ++i)
		{
			auto view = registry.view<pg::DiffusionJobSingletonComponent>();
			if (!view.empty())
			{
				const pg::DiffusionJobSingletonComponent& job = view.get<pg::DiffusionJobSingletonComponent>(view.front());
				if (job.m_ActiveJob != nullptr && job.m_ActiveJob->m_State.load() == pg::EDiffusionJobState::eDone)
				{
					return true;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return false;
	}
}

TEST_CASE("Diffusion.DiffusionSystem::DeclareAccessIsCorrect")
{
	pg::DiffusionSystem sys;
	pg::SystemAccessDecl decl = sys.DeclareAccess();

	CHECK(decl.readSet.count(std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(pg::DiffusionBackendSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(pg::DiffusionJobSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::DiffusionBackendSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::DiffusionJobSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::RegisterGeneratedTextureRequestOneFrameComponent))) > 0);
}

TEST_CASE("Diffusion.DiffusionSystem::GuardsWhenConfigAndResourcesAbsent")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());

	world.Update(pg::Timestep(0));

	// No config/resources -> the system returns before creating any singleton.
	CHECK(pg::World::GetRegistryDirect().view<pg::DiffusionBackendSingletonComponent>().empty());
	CHECK(pg::World::GetRegistryDirect().view<pg::DiffusionJobSingletonComponent>().empty());
}

TEST_CASE("Diffusion.DiffusionSystem::CreatesBackendAndJobSingletons")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<pg::DiffusionBackendSingletonComponent>().size() == 1);
	CHECK(pg::World::GetRegistryDirect().view<pg::DiffusionJobSingletonComponent>().size() == 1);
}

TEST_CASE("Diffusion.DiffusionSystem::LoadsResidentCheckpointFromResourceMap")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	world.Update(pg::Timestep(0)); // create singletons
	world.Update(pg::Timestep(0)); // load checkpoint

	pg::DiffusionBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->m_Backend != nullptr);
	CHECK(backend->m_Backend->IsLoaded());

	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetCheckpointPath().find("checkpoint.safetensors") != std::string::npos);
}

TEST_CASE("Diffusion.DiffusionSystem::DoesNotLoadWhenNoCheckpointDeclared")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(false);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));

	pg::DiffusionBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	CHECK(backend->m_Backend->IsLoaded() == false);
}

TEST_CASE("Diffusion.DiffusionSystem::GeneratesAndEmitsRegisterRequest")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = k_TargetTexture;
	request.m_Prompt = "a pigeon knight";
	request.m_Loras.push_back(pg::GenerateImageLoraRef{ k_LoraID, 0.7f });
	request.m_ControlSkeletonID = k_SkeletonID;
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateImageRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0)); // create singletons
	world.Update(pg::Timestep(0)); // load + launch job

	// Drop the request so the post-completion frame does not start a second job.
	registry.destroy(requestEntity);

	REQUIRE(WaitForActiveJobDone(2000));

	// One retaining frame lets the system reap the job and emit the registration (kept for inspection).
	world.UpdateRetainingEvents(pg::Timestep(0));

	auto view = pg::World::GetRegistryDirect().view<pg::RegisterGeneratedTextureRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::RegisterGeneratedTextureRequestOneFrameComponent& registration =
		view.get<pg::RegisterGeneratedTextureRequestOneFrameComponent>(view.front());
	CHECK(registration.m_TextureID == k_TargetTexture);
	CHECK(registration.m_Image.m_Width == 64);
	CHECK(registration.m_Image.m_Height == 64);
	CHECK(registration.m_Image.m_Pixels.size() == static_cast<size_t>(64) * 64 * 3);
}

TEST_CASE("Diffusion.DiffusionSystem::AssemblesParamsFromRequestDefaultsAndResources")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = k_TargetTexture;
	request.m_Prompt = "a pigeon knight";
	request.m_Loras.push_back(pg::GenerateImageLoraRef{ k_LoraID, 0.7f });
	request.m_ControlSkeletonID = k_SkeletonID;
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateImageRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));
	registry.destroy(requestEntity);
	REQUIRE(WaitForActiveJobDone(2000));

	pg::DiffusionBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);

	const pg::DiffusionJobParams& params = mock->GetLastParams();
	CHECK(params.m_Prompt == "a pigeon knight");
	// Resolution falls back to the engine Generation Config defaults seeded above.
	CHECK(params.m_Width == 64);
	CHECK(params.m_Height == 64);
	// LoRA UUID resolved to its path with the requested weight.
	REQUIRE(params.m_Loras.size() == 1);
	CHECK(params.m_Loras[0].m_Path.find("lora.safetensors") != std::string::npos);
	CHECK(params.m_Loras[0].m_Weight == Approx(0.7f));
	// OpenPose skeleton rasterized into a control hint of the generation size.
	CHECK(params.m_HasControlHint);
	CHECK(params.m_ControlHint.m_Width == 64);
	CHECK(params.m_ControlHint.m_Pixels.size() == static_cast<size_t>(64) * 64 * 3);
}

TEST_CASE("Diffusion.DiffusionSystem::Img2ImgResizesInitImageToGenerationSize")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = k_TargetTexture;
	request.m_Prompt = "a living room";
	request.m_InputImageID = k_InputImageID; // 8x8 seeded; generation size is 64x64
	request.m_DenoiseStrength = 0.6f;
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateImageRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));
	registry.destroy(requestEntity);
	REQUIRE(WaitForActiveJobDone(2000));

	pg::DiffusionBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);

	const pg::DiffusionJobParams& params = mock->GetLastParams();
	CHECK(params.m_HasInitImage);
	CHECK(params.m_InitImage.m_Width == 64);
	CHECK(params.m_InitImage.m_Height == 64);
	CHECK(params.m_InitImage.m_Pixels.size() == static_cast<size_t>(64) * 64 * 3);
	CHECK(params.m_DenoiseStrength == Approx(0.6f));
}

TEST_CASE("Diffusion.DiffusionSystem::MaskFromSkeletonBuildsInpaintMask")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = k_TargetTexture;
	request.m_Prompt = "lin in a living room";
	request.m_InputImageID = k_InputImageID;
	request.m_ControlSkeletonID = k_SkeletonID;
	request.m_MaskFromSkeleton = true;
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateImageRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));
	registry.destroy(requestEntity);
	REQUIRE(WaitForActiveJobDone(2000));

	pg::DiffusionBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	pg::TestingDiffusionBackend* mock = dynamic_cast<pg::TestingDiffusionBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);

	const pg::DiffusionJobParams& params = mock->GetLastParams();
	REQUIRE(params.m_HasMask);
	CHECK(params.m_Mask.m_Width == 64);
	CHECK(params.m_Mask.m_Height == 64);
	// The mask has a regenerate (white) region over the skeleton's location.
	bool anyWhite = false;
	for (uint8_t channel : params.m_Mask.m_Pixels)
	{
		if (channel == 255)
		{
			anyWhite = true;
			break;
		}
	}
	CHECK(anyWhite);
}

TEST_CASE("Diffusion.DiffusionSystem::ChromaCompositeReplacesKeyedPixelsWithBackground")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::DiffusionSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateImageRequestOneFrameComponent request;
	request.m_TargetTextureID = k_TargetTexture;
	request.m_Prompt = "lin on green";
	request.m_BackgroundImageID = k_InputImageID; // seeded 8x8, value 64, resized to the 64x64 gen size
	// The Testing backend outputs a solid mid-grey (128), so the auto-detected corner key is 128 and the
	// whole (uniform) foreground is keyed out to the background — proving the composite step ran.
	request.m_ChromaKeyThreshold = 0.6f;
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateImageRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));
	registry.destroy(requestEntity);
	REQUIRE(WaitForActiveJobDone(2000));

	world.UpdateRetainingEvents(pg::Timestep(0));

	auto view = pg::World::GetRegistryDirect().view<pg::RegisterGeneratedTextureRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::RegisterGeneratedTextureRequestOneFrameComponent& registration =
		view.get<pg::RegisterGeneratedTextureRequestOneFrameComponent>(view.front());
	REQUIRE(!registration.m_Image.m_Pixels.empty());
	// The keyed-out grey foreground is replaced by the background (value 64).
	CHECK(static_cast<int>(registration.m_Image.m_Pixels[0]) == 64);
}
