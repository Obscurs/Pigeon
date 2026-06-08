#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Transform/LocalBoundsComponent.h"
#include "Pigeon/Transform/PositionComponent.h"
#include "Pigeon/Transform/RotationComponent.h"
#include "Pigeon/Transform/ScaleComponent.h"
#include "Pigeon/Transform/TransformComposeSystem.h"
#include "Pigeon/Transform/WorldTransformComponent.h"

#include <glm/glm.hpp>

namespace CatchTestsetFail
{
	namespace
	{
		pg::ecs::Entity MakeTransformedEntity(pg::ecs::Registry& registry, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& boundsMin, const glm::vec3& boundsMax)
		{
			pg::ecs::Entity ent = registry.create();
			registry.emplace<pg::PositionComponent>(ent).m_Position = pos;
			registry.emplace<pg::RotationComponent>(ent);
			registry.emplace<pg::ScaleComponent>(ent).m_Scale = scale;
			pg::LocalBoundsComponent& bounds = registry.emplace<pg::LocalBoundsComponent>(ent);
			bounds.m_Min = boundsMin;
			bounds.m_Max = boundsMax;
			return ent;
		}
	}

	// ---------------------------------------------------------------------------
	// A full set of transform components composes a WorldTransform whose matrix
	// translates by the position (identity rotation and scale).
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformComposeSystem::ComposesTranslation")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformComposeSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = MakeTransformedEntity(registry, glm::vec3(2.f, 3.f, 4.f), glm::vec3(1.f), glm::vec3(0.f), glm::vec3(1.f, 1.f, 0.f));

		world.Update(pg::Timestep(0));

		REQUIRE(registry.all_of<pg::WorldTransformComponent>(ent));
		const glm::mat4& m = registry.get<pg::WorldTransformComponent>(ent).m_Matrix;
		CHECK(m[3].x == Approx(2.f));
		CHECK(m[3].y == Approx(3.f));
		CHECK(m[3].z == Approx(4.f));
	}

	// ---------------------------------------------------------------------------
	// Scale is baked into the matrix.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformComposeSystem::AppliesScale")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformComposeSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = MakeTransformedEntity(registry, glm::vec3(0.f), glm::vec3(2.f, 5.f, 1.f), glm::vec3(0.f), glm::vec3(1.f, 1.f, 0.f));

		world.Update(pg::Timestep(0));

		const glm::mat4& m = registry.get<pg::WorldTransformComponent>(ent).m_Matrix;
		// Local corner (1,1,0) maps to (2,5,0) under pure scale.
		const glm::vec4 corner = m * glm::vec4(1.f, 1.f, 0.f, 1.f);
		CHECK(corner.x == Approx(2.f));
		CHECK(corner.y == Approx(5.f));
	}

	// ---------------------------------------------------------------------------
	// The sort key is the world-space Y of the local bottom edge (bounds min).
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformComposeSystem::SortKeyIsWorldBottomEdge")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformComposeSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = MakeTransformedEntity(registry, glm::vec3(2.f, 3.f, 0.f), glm::vec3(1.f), glm::vec3(0.f, -0.5f, 0.f), glm::vec3(1.f, 0.5f, 0.f));

		world.Update(pg::Timestep(0));

		// World Y of local min (0,-0.5,0) translated by (2,3,0) is 2.5.
		CHECK(registry.get<pg::WorldTransformComponent>(ent).m_SortKey == Approx(2.5f));
	}

	// ---------------------------------------------------------------------------
	// An existing WorldTransform is overwritten in place on a subsequent frame.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformComposeSystem::OverwritesExistingWorldTransform")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<pg::TransformComposeSystem>());

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity ent = MakeTransformedEntity(registry, glm::vec3(1.f, 1.f, 0.f), glm::vec3(1.f), glm::vec3(0.f), glm::vec3(1.f, 1.f, 0.f));

		world.Update(pg::Timestep(0));
		registry.get<pg::PositionComponent>(ent).m_Position = glm::vec3(9.f, 9.f, 0.f);
		world.Update(pg::Timestep(0));

		const glm::mat4& m = registry.get<pg::WorldTransformComponent>(ent).m_Matrix;
		CHECK(m[3].x == Approx(9.f));
		CHECK(m[3].y == Approx(9.f));

		auto view = registry.view<pg::WorldTransformComponent>();
		CHECK(view.size() == 1);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: reads the canonical components, sole adder/writer of WorldTransform.
	// ---------------------------------------------------------------------------
	TEST_CASE("Transform.TransformComposeSystem::DeclareAccessIsCorrect")
	{
		pg::TransformComposeSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(pg::PositionComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::RotationComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::ScaleComponent))) > 0);
		CHECK(decl.readSet.count(std::type_index(typeid(pg::LocalBoundsComponent))) > 0);

		CHECK(decl.writeSet.count(std::type_index(typeid(pg::WorldTransformComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(pg::WorldTransformComponent))) > 0);
	}

} // namespace CatchTestsetFail
