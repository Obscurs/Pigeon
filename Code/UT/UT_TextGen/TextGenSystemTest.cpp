#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include <chrono>
#include <thread>

#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/TextGen/GenerateTextRequestOneFrameComponent.h"
#include "Pigeon/TextGen/TextGenBackendSingletonComponent.h"
#include "Pigeon/TextGen/TextGenJobSingletonComponent.h"
#include "Pigeon/TextGen/TextGenResultSingletonComponent.h"
#include "Pigeon/TextGen/TextGenSystem.h"
#include "Platform/Testing/TestingTextGenBackend.h"

namespace
{
	const pg::UUID k_TargetText("bbbbbbbb-0000-4000-8000-000000000001");
	const pg::UUID k_ModelID("bbbbbbbb-0000-4000-8000-000000000002");

	// Seeds the singletons TextGenSystem only reads (added by ConfigLoader / ResourceManager in
	// production), with small generation defaults and a model path so the backend loads.
	void SeedConfigAndResources(bool withModel)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();

		pg::EngineConfigSingletonComponent config;
		config.m_TextGenMaxTokens = 32;
		config.m_TextGenTemperature = 0.5f;
		config.m_TextGenTopP = 0.9f;
		config.m_TextGenGpuLayers = 24; // distinct from the engine default (999) to prove it is config-driven
		registry.emplace<pg::EngineConfigSingletonComponent>(registry.create(), config);

		pg::ResourceMapSingletonComponent resources;
		if (withModel)
		{
			resources.m_LanguageModelMap[k_ModelID] = "Assets/App/TextGeneration/model.gguf";
		}
		registry.emplace<pg::ResourceMapSingletonComponent>(registry.create(), resources);
	}

	pg::TextGenBackendSingletonComponent* GetBackend()
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		auto view = registry.view<pg::TextGenBackendSingletonComponent>();
		if (view.empty())
		{
			return nullptr;
		}
		return &view.get<pg::TextGenBackendSingletonComponent>(view.front());
	}

	// Spins the engine forward until the active job reports done (bounded). The mock backend completes
	// near-instantly, so this resolves in a few milliseconds.
	bool WaitForActiveJobDone(int maxMilliseconds)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		for (int i = 0; i < maxMilliseconds; ++i)
		{
			auto view = registry.view<pg::TextGenJobSingletonComponent>();
			if (!view.empty())
			{
				const pg::TextGenJobSingletonComponent& job = view.get<pg::TextGenJobSingletonComponent>(view.front());
				if (job.m_ActiveJob != nullptr && job.m_ActiveJob->m_State.load() == pg::ETextGenJobState::eDone)
				{
					return true;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return false;
	}
}

TEST_CASE("TextGen.TextGenSystem::DeclareAccessIsCorrect")
{
	pg::TextGenSystem sys;
	pg::SystemAccessDecl decl = sys.DeclareAccess();

	CHECK(decl.readSet.count(std::type_index(typeid(pg::GenerateTextRequestOneFrameComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::ResourceMapSingletonComponent))) > 0);
	CHECK(decl.readSet.count(std::type_index(typeid(pg::EngineConfigSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(pg::TextGenBackendSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(pg::TextGenJobSingletonComponent))) > 0);
	CHECK(decl.writeSet.count(std::type_index(typeid(pg::TextGenResultSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::TextGenBackendSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::TextGenJobSingletonComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::TextGenResultSingletonComponent))) > 0);
}

TEST_CASE("TextGen.TextGenSystem::GuardsWhenConfigAndResourcesAbsent")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());

	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenBackendSingletonComponent>().empty());
	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenJobSingletonComponent>().empty());
	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenResultSingletonComponent>().empty());
}

TEST_CASE("TextGen.TextGenSystem::CreatesBackendJobAndResultSingletons")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(true);

	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenBackendSingletonComponent>().size() == 1);
	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenJobSingletonComponent>().size() == 1);
	CHECK(pg::World::GetRegistryDirect().view<pg::TextGenResultSingletonComponent>().size() == 1);
}

TEST_CASE("TextGen.TextGenSystem::LoadsResidentModelFromResourceMap")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(true);

	world.Update(pg::Timestep(0)); // create singletons
	world.Update(pg::Timestep(0)); // load model

	pg::TextGenBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	REQUIRE(backend->m_Backend != nullptr);
	CHECK(backend->m_Backend->IsLoaded());

	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);
	CHECK(mock->GetModelPath().find("model.gguf") != std::string::npos);
}

TEST_CASE("TextGen.TextGenSystem::LoadsModelWithConfiguredGpuLayers")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(true);

	world.Update(pg::Timestep(0)); // create singletons
	world.Update(pg::Timestep(0)); // load model

	pg::TextGenBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);

	// The system must forward the engine config's Text Gen GPU Layers to LoadModel (ADR 0010), not a
	// hardcoded value.
	CHECK(mock->GetGpuLayers() == 24);
}

TEST_CASE("TextGen.TextGenSystem::DoesNotLoadWhenNoModelDeclared")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(false);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));

	pg::TextGenBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	CHECK(backend->m_Backend->IsLoaded() == false);
}

TEST_CASE("TextGen.TextGenSystem::GeneratesAndPublishesResult")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateTextRequestOneFrameComponent request;
	request.m_TargetTextID = k_TargetText;
	request.m_Prompt = "Tell me a story";
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateTextRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0)); // create singletons
	world.Update(pg::Timestep(0)); // load + launch job

	// Drop the request so the post-completion frame does not start a second job.
	registry.destroy(requestEntity);

	REQUIRE(WaitForActiveJobDone(2000));

	// One more frame lets the system reap the job and write the result into the result store.
	world.Update(pg::Timestep(0));

	auto view = pg::World::GetRegistryDirect().view<pg::TextGenResultSingletonComponent>();
	REQUIRE(view.size() == 1);
	const pg::TextGenResultSingletonComponent& results = view.get<pg::TextGenResultSingletonComponent>(view.front());
	std::unordered_map<pg::UUID, std::string>::const_iterator it = results.m_Results.find(k_TargetText);
	REQUIRE(it != results.m_Results.end());
	CHECK(it->second.find("Tell me a story") != std::string::npos);
}

TEST_CASE("TextGen.TextGenSystem::AssemblesParamsFromRequestDefaultsAndResources")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(true);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateTextRequestOneFrameComponent request;
	request.m_TargetTextID = k_TargetText;
	request.m_Prompt = "Hello";
	request.m_SystemPrompt = "You are a bard.";
	request.m_MaxTokens = 8; // overrides the config default (32)
	const pg::ecs::Entity requestEntity = registry.create();
	registry.emplace<pg::GenerateTextRequestOneFrameComponent>(requestEntity, request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));
	registry.destroy(requestEntity);
	REQUIRE(WaitForActiveJobDone(2000));

	pg::TextGenBackendSingletonComponent* backend = GetBackend();
	REQUIRE(backend != nullptr);
	pg::TestingTextGenBackend* mock = dynamic_cast<pg::TestingTextGenBackend*>(backend->m_Backend.get());
	REQUIRE(mock != nullptr);

	const pg::TextGenParams& params = mock->GetLastParams();
	CHECK(params.m_Prompt == "Hello");
	CHECK(params.m_SystemPrompt == "You are a bard.");
	// Overridden field honoured.
	CHECK(params.m_MaxTokens == 8);
	// Unset fields fall back to the engine Generation Config defaults seeded above.
	CHECK(params.m_Temperature == Approx(0.5f));
	CHECK(params.m_TopP == Approx(0.9f));
}

TEST_CASE("TextGen.TextGenSystem::IgnoresRequestWhenNoModelLoaded")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<pg::TextGenSystem>());
	SeedConfigAndResources(false);

	pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
	pg::GenerateTextRequestOneFrameComponent request;
	request.m_TargetTextID = k_TargetText;
	request.m_Prompt = "Hello";
	registry.emplace<pg::GenerateTextRequestOneFrameComponent>(registry.create(), request);

	world.Update(pg::Timestep(0));
	world.Update(pg::Timestep(0));

	auto jobView = pg::World::GetRegistryDirect().view<pg::TextGenJobSingletonComponent>();
	REQUIRE(jobView.size() == 1);
	const pg::TextGenJobSingletonComponent& job = jobView.get<pg::TextGenJobSingletonComponent>(jobView.front());
	CHECK(job.m_ActiveJob == nullptr);
}
