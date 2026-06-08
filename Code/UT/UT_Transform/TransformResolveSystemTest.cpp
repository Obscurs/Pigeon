#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/LocalBoundsComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Pigeon/Transform/ResolvedTransformRequestOneFrameComponent.h"
#include "Pigeon/Transform/RotationComponent.h"
#include "Pigeon/Transform/ScaleComponent.h"
#include "Pigeon/Transform/TransformResolveSystem.h"

#include <glm/glm.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// A resolved request with the position channel set adds a PositionComponent
	// carrying the requested value.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::AddsPositionFromResolvedRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::ResolvedTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(2.f, 3.f, 4.f);
		registry.emplace<pg::ResolvedTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::PositionComponent>(ent));
		const pg::PositionComponent& pos = registry.get<pg::PositionComponent>(ent);
		CHECK(pos.m_Position.x == Approx(2.f));
		CHECK(pos.m_Position.y == Approx(3.f));
		CHECK(pos.m_Position.z == Approx(4.f));
	}

	// ---------------------------------------------------------------------------
	// When the component already exists, the resolver overwrites it in place.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::OverwritesExistingPosition")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		registry.emplace<pg::PositionComponent>(ent).m_Position = glm::vec3(1.f, 1.f, 1.f);

		pg::ResolvedTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(-5.f, 6.f, 0.f);
		registry.emplace<pg::ResolvedTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		const pg::PositionComponent& pos = registry.get<pg::PositionComponent>(ent);
		CHECK(pos.m_Position.x == Approx(-5.f));
		CHECK(pos.m_Position.y == Approx(6.f));
	}

	// ---------------------------------------------------------------------------
	// Only flagged channels are applied; an unset channel adds no component.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::IgnoresUnflaggedChannels")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::ResolvedTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(0.f, 0.f, 0.f);
		// rotation/scale/bounds left unflagged
		registry.emplace<pg::ResolvedTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		CHECK(registry.all_of<pg::PositionComponent>(ent));
		CHECK_FALSE(registry.all_of<pg::RotationComponent>(ent));
		CHECK_FALSE(registry.all_of<pg::ScaleComponent>(ent));
		CHECK_FALSE(registry.all_of<pg::LocalBoundsComponent>(ent));
	}

	// ---------------------------------------------------------------------------
	// Rotation, scale and bounds channels are applied when flagged.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::AppliesAllChannels")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::ResolvedTransformRequestOneFrameComponent request;
		request.m_Data.m_SetScale = true;
		request.m_Data.m_Scale = glm::vec3(2.f, 3.f, 1.f);
		request.m_Data.m_SetBounds = true;
		request.m_Data.m_BoundsMin = glm::vec3(-1.f, -2.f, 0.f);
		request.m_Data.m_BoundsMax = glm::vec3(1.f, 2.f, 0.f);
		registry.emplace<pg::ResolvedTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::ScaleComponent>(ent));
		REQUIRE(registry.all_of<pg::LocalBoundsComponent>(ent));
		CHECK(registry.get<pg::ScaleComponent>(ent).m_Scale.y == Approx(3.f));
		CHECK(registry.get<pg::LocalBoundsComponent>(ent).m_Min.y == Approx(-2.f));
		CHECK(registry.get<pg::LocalBoundsComponent>(ent).m_Max.x == Approx(1.f));
	}

	// ---------------------------------------------------------------------------
	// Engine-origin camera requests are applied just like the aggregated app request.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::AppliesCameraRequest")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformResolveSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = registry.create();
		pg::CameraTransformRequestOneFrameComponent request;
		request.m_Data.m_SetPosition = true;
		request.m_Data.m_Position = glm::vec3(7.f, 8.f, 0.f);
		registry.emplace<pg::CameraTransformRequestOneFrameComponent>(ent, request);

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::PositionComponent>(ent));
		CHECK(registry.get<pg::PositionComponent>(ent).m_Position.x == Approx(7.f));
		CHECK(registry.get<pg::PositionComponent>(ent).m_Position.y == Approx(8.f));
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: sole adder/writer of the canonical components, reads both requests.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformResolveSystem::DeclareAccessIsCorrect")
	{
		pg::TransformResolveSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::ResolvedTransformRequestOneFrameComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::CameraTransformRequestOneFrameComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::RotationComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::ScaleComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(pg::LocalBoundsComponent))) > 0);

		CHECK(decl.addSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::RotationComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::ScaleComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::LocalBoundsComponent))) > 0);
	}

} // namespace CatchTestsetFail
