#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/Core/EventComponent.h"
#include "Pigeon/Core/KeyCodes.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Diffusion/GenerateImageRequestOneFrameComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ImageGenDemoIds.h"
#include "Sandbox/ImageGenDemoSystem.h"

namespace
{
	void EmitKeyPressed(int keyCode)
	{
		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::KeyPressedEventComponent event;
		event.m_KeyCode = keyCode;
		registry.emplace<pg::KeyPressedEventComponent>(ent, event);
		registry.emplace<pg::EventComponent>(ent);
	}
}

TEST_CASE("Sandbox.ImageGenDemoSystem::DeclareAccessIsCorrect")
{
	sbx::ImageGenDemoSystem sys;
	pg::SystemAccessDecl decl = sys.DeclareAccess();

	CHECK(decl.readSet.count(std::type_index(typeid(pg::KeyPressedEventComponent))) > 0);
	CHECK(decl.addSet.count(std::type_index(typeid(pg::GenerateImageRequestOneFrameComponent))) > 0);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::DoesNotRequestWithoutKey")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().empty());
}

TEST_CASE("Sandbox.ImageGenDemoSystem::IgnoresOtherKeys")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	EmitKeyPressed(pg::PG_KEY_H);
	world.Update(pg::Timestep(0));

	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().empty());
}

TEST_CASE("Sandbox.ImageGenDemoSystem::EmitsRequestOnKeyPress")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	EmitKeyPressed(pg::PG_KEY_G);
	world.Update(pg::Timestep(0));

	auto view = pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>();
	REQUIRE(view.size() == 1);
	const pg::GenerateImageRequestOneFrameComponent& request =
		view.get<pg::GenerateImageRequestOneFrameComponent>(view.front());
	CHECK(request.m_TargetTextureID == sbx::k_GeneratedTextureID);
	CHECK(request.m_ControlSkeletonID == sbx::k_DiffusionSkeletonID);
	REQUIRE(request.m_Loras.size() == 1);
	CHECK(request.m_Loras[0].m_LoraID == sbx::k_DiffusionLoraID);
}

TEST_CASE("Sandbox.ImageGenDemoSystem::ReEmitsOnEachKeyPress")
{
	pg::World& world = pg::World::Create();
	world.RegisterSystem(std::make_unique<sbx::ImageGenDemoSystem>());

	// First press emits; the key event is cleared at end of frame (tagged EventComponent).
	EmitKeyPressed(pg::PG_KEY_G);
	world.Update(pg::Timestep(0));
	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().size() == 1);

	// A later, separate press emits again (no permanent latch) — so the user can retry after load.
	EmitKeyPressed(pg::PG_KEY_G);
	world.Update(pg::Timestep(0));
	CHECK(pg::World::GetRegistryDirect().view<pg::GenerateImageRequestOneFrameComponent>().size() == 1);
}
