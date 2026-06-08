#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Audio/AudioVolumeSingletonComponent.h"
#include "Pigeon/Audio/SetAudioVolumeRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/AudioDebugPanelSystem.h"

namespace CatchTestsetFail
{
	// Guard: with no volume singleton present the system does nothing and emits no request.
	TEST_CASE("Sandbox.AudioDebugPanelSystem::NoOpWithoutVolume")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDebugPanelSystem>());

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetAudioVolumeRequestOneFrameComponent>().size() == 0);
	}

	// Guard: the test build has no ImGui context, so even with volumes present no request is emitted.
	TEST_CASE("Sandbox.AudioDebugPanelSystem::NoRequestWithoutImGuiContext")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::AudioDebugPanelSystem>());

		pg::ecs::Entity volEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<pg::AudioVolumeSingletonComponent>(volEnt);

		world.Update(pg::Timestep(0));

		CHECK(pg::World::GetRegistryDirect().view<pg::SetAudioVolumeRequestOneFrameComponent>().size() == 0);
	}

	// DeclareAccess: verify declared sets match the system's actual access.
	TEST_CASE("Sandbox.AudioDebugPanelSystem::DeclareAccessIsCorrect")
	{
		sbx::AudioDebugPanelSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::AudioVolumeSingletonComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::SetAudioVolumeRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
