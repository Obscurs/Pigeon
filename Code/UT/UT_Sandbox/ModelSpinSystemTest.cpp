#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/ModelSpinComponent.h"
#include "Sandbox/ModelSpinTransformRequestOneFrameComponent.h"
#include "Sandbox/ModelSpinSystem.h"

#include <cmath>

namespace
{
	// ModelSpinComponent is added in production by SceneSetupSystem (a different system), so the spin
	// test may create it directly as the precondition the spin system then modifies in place.
	pg::ecs::Entity MakeSpinningModel(pg::ecs::Registry& registry, const sbx::ModelSpinComponent& spin)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<sbx::ModelSpinComponent>(ent, spin);
		return ent;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: accumulated spin time advances by the frame delta.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ModelSpinSystem::AdvancesElapsedByDelta")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ModelSpinSystem>());

		sbx::ModelSpinComponent spin;
		spin.m_RotationSpeed = 1.f;
		pg::ecs::Entity ent = MakeSpinningModel(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000)); // 1 second

		const sbx::ModelSpinComponent& result = pg::World::GetRegistryDirect().get<sbx::ModelSpinComponent>(ent);
		CHECK(std::fabs(result.m_Elapsed - 1.f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the emitted request holds the anchor position and a Y-axis
	// rotation proportional to the spin speed and elapsed time.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ModelSpinSystem::RequestsYAxisRotationAtAnchor")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ModelSpinSystem>());

		sbx::ModelSpinComponent spin;
		spin.m_Anchor = { 1.f, 2.f, 3.f };
		spin.m_RotationSpeed = 2.f;
		pg::ecs::Entity ent = MakeSpinningModel(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000)); // elapsed = 1 -> angle = speed * elapsed = 2 rad

		REQUIRE(pg::World::GetRegistryDirect().all_of<sbx::ModelSpinTransformRequestOneFrameComponent>(ent));
		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<sbx::ModelSpinTransformRequestOneFrameComponent>(ent).m_Data;

		CHECK(data.m_SetPosition);
		CHECK(std::fabs(data.m_Position.x - 1.f) < 1e-4f);
		CHECK(std::fabs(data.m_Position.y - 2.f) < 1e-4f);
		CHECK(std::fabs(data.m_Position.z - 3.f) < 1e-4f);

		// A rotation of 2 rad about Y: quaternion (cos(1), 0, sin(1), 0).
		CHECK(data.m_SetRotation);
		CHECK(std::fabs(data.m_Rotation.w - std::cos(1.f)) < 1e-3f);
		CHECK(std::fabs(data.m_Rotation.y - std::sin(1.f)) < 1e-3f);
		CHECK(std::fabs(data.m_Rotation.x) < 1e-4f);
		CHECK(std::fabs(data.m_Rotation.z) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Edge: zero spin speed holds the identity orientation (no rotation requested
	// away from identity).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ModelSpinSystem::ZeroSpeedHoldsIdentity")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::ModelSpinSystem>());

		sbx::ModelSpinComponent spin;
		spin.m_RotationSpeed = 0.f;
		pg::ecs::Entity ent = MakeSpinningModel(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000));

		const pg::TransformRequestData& data = pg::World::GetRegistryDirect().get<sbx::ModelSpinTransformRequestOneFrameComponent>(ent).m_Data;
		CHECK(data.m_SetRotation);
		CHECK(std::fabs(data.m_Rotation.w - 1.f) < 1e-4f);
		CHECK(std::fabs(data.m_Rotation.y) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.ModelSpinSystem::DeclareAccessIsCorrect")
	{
		sbx::ModelSpinSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::ModelSpinComponent))) > 0);
		CHECK(decl.addSet.count(std::type_index(typeid(sbx::ModelSpinTransformRequestOneFrameComponent))) > 0);
	}

} // namespace CatchTestsetFail
