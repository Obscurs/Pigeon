#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/ResolvedTransformRequestOneFrameComponent.h"
#include "Sandbox/AnimationTransformRequestOneFrameComponent.h"
#include "Sandbox/QuadSpawnTransformRequestOneFrameComponent.h"
#include "Sandbox/SceneTransformRequestOneFrameComponent.h"
#include "Sandbox/TransformResolveSystem.h"

#include <glm/glm.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// A single app request is aggregated into one engine-typed resolved request
	// carrying the same channels.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TransformResolveSystem::AggregatesSingleRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		sbx::SceneTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(4.f, 5.f, 6.f);
		registry.emplace<sbx::SceneTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::ResolvedTransformRequestOneFrameComponent>(ent));
		const pg::TransformRequestData& data = registry.get<pg::ResolvedTransformRequestOneFrameComponent>(ent).m_Data;
		CHECK(data.m_SetPosition);
		CHECK(data.m_Position.x == Approx(4.f));
		CHECK(data.m_Position.y == Approx(5.f));
	}

	// ---------------------------------------------------------------------------
	// Two different app request types targeting the same entity merge their
	// channels into one resolved request.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TransformResolveSystem::MergesChannelsAcrossRequestTypes")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();

		sbx::QuadSpawnTransformRequestOneFrameComponent spawn;
		spawn.m_Data.m_SetScale = true;
		spawn.m_Data.m_Scale = glm::vec3(2.f, 2.f, 1.f);
		registry.emplace<sbx::QuadSpawnTransformRequestOneFrameComponent>(ent, spawn);

		sbx::AnimationTransformRequestOneFrameComponent anim;
		anim.m_Data.m_SetPosition = true;
		anim.m_Data.m_Position = glm::vec3(1.f, 2.f, 0.f);
		registry.emplace<sbx::AnimationTransformRequestOneFrameComponent>(ent, anim);

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::ResolvedTransformRequestOneFrameComponent>(ent));
		const pg::TransformRequestData& data = registry.get<pg::ResolvedTransformRequestOneFrameComponent>(ent).m_Data;
		CHECK(data.m_SetScale);
		CHECK(data.m_Scale.x == Approx(2.f));
		CHECK(data.m_SetPosition);
		CHECK(data.m_Position.y == Approx(2.f));
	}

	// ---------------------------------------------------------------------------
	// Two entities each with their own request produce two resolved requests.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TransformResolveSystem::EmitsOnePerEntity")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity a = registry.create();
		pg::ecs::Entity b = registry.create();
		sbx::SceneTransformRequestOneFrameComponent ra;
		ra.m_Data.m_SetPosition = true;
		registry.emplace<sbx::SceneTransformRequestOneFrameComponent>(a, ra);
		sbx::QuadSpawnTransformRequestOneFrameComponent rb;
		rb.m_Data.m_SetPosition = true;
		registry.emplace<sbx::QuadSpawnTransformRequestOneFrameComponent>(b, rb);

		world.Update(pg::Timestep(0));

		auto view = registry.view<pg::ResolvedTransformRequestOneFrameComponent>();
		CHECK(view.size() == 2);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: reads the three app request types, adds the resolved request.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.TransformResolveSystem::DeclareAccessIsCorrect")
	{
		sbx::TransformResolveSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::SceneTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::QuadSpawnTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(sbx::AnimationTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ResolvedTransformRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
