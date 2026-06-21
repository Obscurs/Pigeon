#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <memory>

#include "Pigeon/Core/EventComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionBackend.h"
#include "Pigeon/Diffusion/DiffusionBackendSingletonComponent.h"
#include "Pigeon/Diffusion/DiffusionJob.h"
#include "Pigeon/Diffusion/DiffusionJobSingletonComponent.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/Diffusion/RasterizeOpenPoseHintRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ImageGenDemoIds.h"
#include "Sandbox/ImageGenDemoStateSingletonComponent.h"
#include "Sandbox/ImageGenDemoSystem.h"

namespace
{
	// Seeds the engine diffusion singletons the demo reads (added by DiffusionSystem / ResourceManager
	// in production). The backend is "loaded" when withLoadedBackend so the demo will start.
	void SeedDiffusionSingletons(bool withLoadedBackend)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		pg::ecs::Entity backendEnt = registry.create();
		pg::DiffusionBackendSingletonComponent& backend = registry.emplace<pg::DiffusionBackendSingletonComponent>(backendEnt);
		backend.m_Backend = pg::DiffusionBackend::Create();
		backend.m_LoadAttempted = true;
		if (withLoadedBackend)
		{
			backend.m_Backend->LoadCheckpoint("dummy.safetensors", "", "");
		}

		registry.emplace<pg::DiffusionJobSingletonComponent>(registry.create());
		registry.emplace<pg::ResourceMapSingletonComponent>(registry.create());
	}

	pg::DiffusionJobSingletonComponent& GetJob()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<pg::DiffusionJobSingletonComponent>();
		return view.get<pg::DiffusionJobSingletonComponent>(view.front());
	}

	pg::ResourceMapSingletonComponent& GetResources()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<pg::ResourceMapSingletonComponent>();
		return view.get<pg::ResourceMapSingletonComponent>(view.front());
	}

	const sbx::ImageGenDemoStateSingletonComponent& GetState()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<sbx::ImageGenDemoStateSingletonComponent>();
		return view.get<sbx::ImageGenDemoStateSingletonComponent>(view.front());
	}

	void SetJobRunning(bool running)
	{
		GetJob().m_ActiveJob = running ? std::make_shared<pg::DiffusionJob>() : nullptr;
	}

	void EmitGKey()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::KeyPressedEventComponent event;
		event.m_KeyCode = pg::PG_KEY_G;
		registry.emplace<pg::KeyPressedEventComponent>(ent, event);
		registry.emplace<pg::EventComponent>(ent);
	}

	// Drives the demo from idle to the eBackground step: seeds singletons + state, then a G press starts
	// the pipeline. Leaves the world with the background request emitted and step == eBackground.
	void DriveToBackground(pg::World& world)
	{
		SeedDiffusionSingletons(true);
		world.Update(pg::Timestep(0)); // seeds demo state, returns
		EmitGKey();
		world.Update(pg::Timestep(0)); // starts the pipeline -> eBackground
	}

	// Simulates a generation step finishing: the job is seen running for a frame, then idle.
	void RunStepToCompletion(pg::World& world)
	{
		SetJobRunning(true);
		world.Update(pg::Timestep(0)); // observes the job running
		SetJobRunning(false);
		world.Update(pg::Timestep(0)); // observes it finished -> advances
	}
}

TEST_CASE("Sandbox.ImageGenDemoSystem::DeclareAccessIsCorrect")
{
	sbx::ImageGenDemoSystem sys;
	pg::SystemAccessDecl decl = sys.DeclareAccess();

	CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyPressedEventComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::DiffusionBackendSingletonComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::DiffusionJobSingletonComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(sbx::ImageGenDemoStateSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(sbx::ImageGenDemoStateSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::RasterizeOpenPoseHintRequestOneFrameComponent))) > 0);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::NoOpWithoutDiffusionSingletons")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	EmitGKey();
	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<sbx::ImageGenDemoStateSingletonComponent>().empty());
	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().empty());
}

TEST_CASE("Sandbox.ImageGenDemoSystem::SeedsDemoStateSingleton")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());
	SeedDiffusionSingletons(true);

	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<sbx::ImageGenDemoStateSingletonComponent>().size() == 1);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::StartsBackgroundRestyleOnGKey")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	DriveToBackground(world);

	auto view = pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::GenerateImageRequestOneFrameComponent& request =
		view.get<pg::GenerateImageRequestOneFrameComponent>(view.front());
	// Step 1 is an img2img restyle of the original photo: no LoRA, no ControlNet, just the input image.
	CHECK(request.m_TargetTextureID == sbx::k_BackgroundTextureID);
	CHECK(request.m_InputImageID == sbx::k_LivingRoomImageID);
	CHECK(request.m_ControlSkeletonID.IsNull());
	CHECK(request.m_Loras.empty());
	CHECK(request.m_DenoiseStrength > 0.f);
	CHECK(GetState().m_Step == sbx::EImageGenStep::eBackground);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::DoesNotStartWhenBackendNotLoaded")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());
	SeedDiffusionSingletons(false); // backend present but not loaded

	world.Update(pg::Timestep(0)); // seeds state
	EmitGKey();
	world.Update(pg::Timestep(0)); // G pressed but not ready -> no start

	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().empty());
	CHECK(GetState().m_Step == sbx::EImageGenStep::eIdle);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::AdvancesToHintWhenBackgroundJobCompletes")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	DriveToBackground(world);
	RunStepToCompletion(world); // background job runs then finishes

	auto view = pg::World::GetRegistryDirect().view<pg::RasterizeOpenPoseHintRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::RasterizeOpenPoseHintRequestOneFrameComponent& hint =
		view.get<pg::RasterizeOpenPoseHintRequestOneFrameComponent>(view.front());
	CHECK(hint.m_SkeletonID == sbx::k_DiffusionSkeletonID);
	CHECK(hint.m_TargetTextureID == sbx::k_HintTextureID);
	CHECK(GetState().m_Step == sbx::EImageGenStep::eHint);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::LaunchesCompositeWhenRestyledBackgroundIsReady")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	DriveToBackground(world);
	RunStepToCompletion(world); // -> eHint

	// The composite waits for the restyled background to be retained as an input image (feed-forward).
	pg::Image background;
	background.m_Width = 4;
	background.m_Height = 4;
	background.m_Pixels.assign(static_cast<size_t>(4) * 4 * 3, 90);
	GetResources().m_InputImageMap[sbx::k_BackgroundTextureID] = background;

	world.Update(pg::Timestep(0)); // background available -> launch composite

	auto view = pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::GenerateImageRequestOneFrameComponent& request =
		view.get<pg::GenerateImageRequestOneFrameComponent>(view.front());
	// Step 3 places the posed character over the restyled background: txt2img + OpenPose ControlNet + the
	// SDXL character LoRA generate the figure on a plain background, then it is composited onto the restyled
	// background through the skeleton's silhouette mask (m_CompositeWithSkeletonMask). img2img is NOT used —
	// img2img + ControlNet NaNs to flat grey on this checkpoint (ADR 0011).
	CHECK(request.m_TargetTextureID == sbx::k_CompositeTextureID);
	CHECK(request.m_InputImageID.IsNull());
	CHECK_FALSE(request.m_MaskFromSkeleton);
	CHECK(request.m_ControlSkeletonID == sbx::k_DiffusionSkeletonID);
	CHECK(request.m_BackgroundImageID == sbx::k_BackgroundTextureID);
	CHECK(request.m_CompositeWithSkeletonMask);
	// The character LoRA is applied by its resource UUID + weight, and its trigger word ("ff7t1f4") is in
	// the prompt (without it the LoRA does not express the character).
	REQUIRE(request.m_Loras.size() == 1);
	CHECK(request.m_Loras[0].m_LoraID == sbx::k_DiffusionLoraID);
	CHECK(request.m_Loras[0].m_Weight == Approx(0.8f));
	CHECK(request.m_Prompt.find("ff7t1f4") != std::string::npos);
	CHECK(GetState().m_Step == sbx::EImageGenStep::eComposite);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::CompletesWhenCompositeJobFinishes")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	DriveToBackground(world);
	RunStepToCompletion(world); // -> eHint

	pg::Image background;
	background.m_Width = 4;
	background.m_Height = 4;
	background.m_Pixels.assign(static_cast<size_t>(4) * 4 * 3, 90);
	GetResources().m_InputImageMap[sbx::k_BackgroundTextureID] = background;
	world.Update(pg::Timestep(0)); // -> eComposite

	RunStepToCompletion(world); // composite job runs then finishes

	CHECK(GetState().m_Step == sbx::EImageGenStep::eDone);
}
