#pragma once
#include <catch2/catch.hpp>
#include "Utils/TestApp.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/DebugControlsSingletonComponent.h"
#include "Sandbox/QuadAnimationSystem.h"
#include "Sandbox/QuadComponent.h"
#include "Sandbox/SpinComponent.h"

#include <cmath>

namespace
{
	// SpinComponent and QuadComponent are added in production by QuadSpawnSystem (a different
	// system), so an animation test may create them directly as the precondition the animation
	// system then modifies in place.
	pg::ecs::Entity MakeQuad(pg::ecs::Registry& registry, const sbx::SpinComponent& spin)
	{
		pg::ecs::Entity ent = registry.create();
		registry.emplace<sbx::SpinComponent>(ent, spin);
		registry.emplace<sbx::QuadComponent>(ent);
		return ent;
	}
}

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// Happy path: accumulated time advances by the frame delta (default 1.0 multiplier).
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::AdvancesElapsedByDelta")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());

		sbx::SpinComponent spin;
		pg::ecs::Entity ent = MakeQuad(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000)); // 1 second

		const sbx::SpinComponent& result = pg::World::GetRegistryDirect().get<sbx::SpinComponent>(ent);
		CHECK(std::fabs(result.m_Elapsed - 1.f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a static spin (no speeds) places the quad at its anchor, scaled.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::StaticQuadSitsAtAnchor")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());

		sbx::SpinComponent spin;
		spin.m_Anchor = { 1.f, 2.f, 3.f };
		spin.m_Scale = { 0.5f, 0.25f, 1.f };
		spin.m_BaseColor = { 0.3f, 0.6f, 0.9f };
		pg::ecs::Entity ent = MakeQuad(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000));

		const sbx::QuadComponent& quad = pg::World::GetRegistryDirect().get<sbx::QuadComponent>(ent);
		// Translation column carries the anchor (z selects the render layer).
		CHECK(std::fabs(quad.m_Transform[3][0] - 1.f) < 1e-4f);
		CHECK(std::fabs(quad.m_Transform[3][1] - 2.f) < 1e-4f);
		CHECK(std::fabs(quad.m_Transform[3][2] - 3.f) < 1e-4f);
		// No rotation -> diagonal carries the scale.
		CHECK(std::fabs(quad.m_Transform[0][0] - 0.5f) < 1e-4f);
		CHECK(std::fabs(quad.m_Transform[1][1] - 0.25f) < 1e-4f);
		// No colour cycle -> base colour preserved (all channels).
		CHECK(quad.m_Color == glm::vec3(0.3f, 0.6f, 0.9f));
	}

	// ---------------------------------------------------------------------------
	// Happy path: orbit displaces the quad's position around the anchor.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::OrbitDisplacesPosition")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());

		sbx::SpinComponent spin;
		spin.m_Anchor = { 0.f, 0.f, 0.f };
		spin.m_OrbitRadius = 1.f;
		spin.m_OrbitSpeed = 1.f;
		pg::ecs::Entity ent = MakeQuad(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000)); // elapsed = 1 -> angle 1 rad

		const sbx::QuadComponent& quad = pg::World::GetRegistryDirect().get<sbx::QuadComponent>(ent);
		CHECK(std::fabs(quad.m_Transform[3][0] - std::cos(1.f)) < 1e-3f);
		CHECK(std::fabs(quad.m_Transform[3][1] - std::sin(1.f)) < 1e-3f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: a non-zero colour-cycle speed changes the colour away from the base.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::ColorCycleChangesColour")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());

		sbx::SpinComponent spin;
		spin.m_BaseColor = { 0.f, 0.f, 0.f };
		spin.m_ColorCycleSpeed = 1.f;
		pg::ecs::Entity ent = MakeQuad(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000));

		const sbx::QuadComponent& quad = pg::World::GetRegistryDirect().get<sbx::QuadComponent>(ent);
		CHECK(quad.m_Color != glm::vec3(0.f, 0.f, 0.f));
		// First channel: 0.5 + 0.5*sin(1).
		CHECK(std::fabs(quad.m_Color.r - (0.5f + 0.5f * std::sin(1.f))) < 1e-3f);
	}

	// ---------------------------------------------------------------------------
	// Happy path: the DebugControls multiplier scales how fast time accumulates.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::DebugMultiplierScalesElapsed")
	{
		pg::World& world = pg::World::Create();
		world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());

		// DebugControls is added in production by DebugPanelSystem (a different system).
		pg::ecs::Entity ctrlEnt = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<sbx::DebugControlsSingletonComponent>(ctrlEnt).m_AnimationSpeed = 2.f;

		sbx::SpinComponent spin;
		pg::ecs::Entity ent = MakeQuad(pg::World::GetRegistryDirect(), spin);

		world.Update(pg::Timestep(1000));

		const sbx::SpinComponent& result = pg::World::GetRegistryDirect().get<sbx::SpinComponent>(ent);
		CHECK(std::fabs(result.m_Elapsed - 2.f) < 1e-4f);
	}

	// ---------------------------------------------------------------------------
	// DeclareAccess: verify declared sets match the system's actual access.
	// ---------------------------------------------------------------------------
	TEST_CASE("Sandbox.QuadAnimationSystem::DeclareAccessIsCorrect")
	{
		sbx::QuadAnimationSystem sys;
		pg::SystemAccessDecl decl = sys.DeclareAccess();

		CHECK(decl.readSet.count(std::type_index(typeid(sbx::DebugControlsSingletonComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::SpinComponent))) > 0);
		CHECK(decl.writeSet.count(std::type_index(typeid(sbx::QuadComponent))) > 0);
	}

} // namespace CatchTestsetFail
