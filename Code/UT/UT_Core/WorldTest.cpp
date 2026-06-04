#pragma once
#include <catch2/catch.hpp>
#include <cstdlib>
#include "Utils/TestApp.h"

#include <Pigeon/ECS/World.h>
#include <Pigeon/ECS/System.h>

#define FLOAT_EQ(x, y) (std::fabs((x) - (y)) < 1e-6)

// Test-local components used by Task B tests.
// Defined in anonymous namespace to avoid linker collisions.
namespace
{
	struct CompA { int value = 0; };
	struct CompB { int value = 0; };

	static uint64_t s_DeltaReceived = 0.f;

	class TestSystem : public pg::System
	{
	public:
		TestSystem() = default;
		~TestSystem() = default;
		pg::SystemAccessDecl DeclareAccess() const override { return pg::SystemAccessDecl{}; }
		void Update(const pg::Timestep& ts) override
		{
			s_DeltaReceived += ts.AsMilliseconds();
		}
	};

	// ---- Test 1 helpers: writer before reader ----

	// Written value that WriterSystem puts into every CompA.
	static constexpr int k_WrittenValue = 42;

	// Last CompA value seen by ReaderSystem after World::Update().
	static int s_ReaderSawValue = -1;

	class WriterSystem : public pg::System
	{
	public:
		pg::SystemAccessDecl DeclareAccess() const override
		{
			pg::SystemAccessDecl decl;
			decl.writeSet.insert(std::type_index(typeid(CompA)));
			return decl;
		}
		void Update(const pg::Timestep&) override
		{
			auto accessor = pg::World::GetRegistry();
			for (auto ent : accessor.view<CompA>())
			{
				accessor.get<CompA>(ent).value = k_WrittenValue;
			}
		}
	};

	class ReaderSystem : public pg::System
	{
	public:
		pg::SystemAccessDecl DeclareAccess() const override
		{
			pg::SystemAccessDecl decl;
			decl.readSet.insert(std::type_index(typeid(CompA)));
			return decl;
		}
		void Update(const pg::Timestep&) override
		{
			auto accessor = pg::World::GetRegistry();
			for (auto ent : accessor.view<CompA>())
			{
				s_ReaderSawValue = accessor.get<CompA>(ent).value;
			}
		}
	};

	// ---- Test 2 helpers: deferred add visibility ----

	static int s_CheckerSawCount = -1;

	class AdderSystem : public pg::System
	{
	public:
		pg::SystemAccessDecl DeclareAccess() const override
		{
			pg::SystemAccessDecl decl;
			decl.addSet.insert(std::type_index(typeid(CompB)));
			return decl;
		}
		void Update(const pg::Timestep&) override
		{
			// Only add one entity total across all frames.
			if (!m_Added)
			{
				auto accessor = pg::World::GetRegistry();
				entt::entity e = accessor.create();
				accessor.emplace_deferred<CompB>(e);
				m_Added = true;
			}
		}
		bool m_Added = false;
	};

	class CheckerSystem : public pg::System
	{
	public:
		pg::SystemAccessDecl DeclareAccess() const override
		{
			pg::SystemAccessDecl decl;
			decl.readSet.insert(std::type_index(typeid(CompB)));
			return decl;
		}
		void Update(const pg::Timestep&) override
		{
			auto accessor = pg::World::GetRegistry();
			s_CheckerSawCount = 0;
			for (auto ent : accessor.view<CompB>())
			{
				(void)ent;
				++s_CheckerSawCount;
			}
		}
	};

	// ---- Test 5 helpers: writeSet and addSet may overlap ----

	static bool s_OverlapSystemRan = false;

	class OverlapSystem : public pg::System
	{
	public:
		pg::SystemAccessDecl DeclareAccess() const override
		{
			pg::SystemAccessDecl decl;
			decl.writeSet.insert(std::type_index(typeid(CompA)));
			decl.addSet.insert(std::type_index(typeid(CompA)));
			return decl;
		}
		void Update(const pg::Timestep&) override
		{
			auto accessor = pg::World::GetRegistry();
			// Mutate existing CompA entities via the write path.
			for (auto ent : accessor.view<CompA>())
			{
				accessor.get<CompA>(ent).value = 99;
			}
			// Deferred-add a new entity with CompA — only once across all frames.
			if (!m_Added)
			{
				entt::entity e = accessor.create();
				accessor.emplace_deferred<CompA>(e);
				m_Added = true;
			}
			s_OverlapSystemRan = true;
		}
		bool m_Added = false;
	};
}
namespace CatchTestsetFail
{
	TEST_CASE("Core.ECS::World")
	{
		pg::World& world = pg::World::Create();

		pg::U_Ptr<TestSystem> testSystem = std::make_unique<TestSystem>();
		world.RegisterSystem(std::move(testSystem));
		pg::Timestep ts(3000);
		world.Update(ts);
		CHECK(s_DeltaReceived == 3000);
	}

	// Test 1: topological sort runs WriterSystem before ReaderSystem even when
	// ReaderSystem is registered first, because WriterSystem declares writeSet={CompA}
	// and ReaderSystem declares readSet={CompA}.
	TEST_CASE("Core.ECS.SystemOrdering::WriterRunsBeforeReader")
	{
		s_ReaderSawValue = -1;

		pg::World& world = pg::World::Create();

		// Seed an entity with CompA so both systems have something to operate on.
		entt::entity e = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<CompA>(e);

		// Register ReaderSystem FIRST — ordering must be determined by dependency, not insertion order.
		world.RegisterSystem(std::make_unique<ReaderSystem>());
		world.RegisterSystem(std::make_unique<WriterSystem>());

		world.Update(pg::Timestep(0));

		// ReaderSystem must have seen the value written by WriterSystem this same frame.
		CHECK(s_ReaderSawValue == k_WrittenValue);
	}

	// Test 2: a component added via emplace_deferred is not visible to CheckerSystem
	// in the same frame it is emplaced, but is visible in the next frame.
	TEST_CASE("Core.ECS.DeferredAdd::NotVisibleSameFrame")
	{
		s_CheckerSawCount = -1;

		pg::World& world = pg::World::Create();

		// Register AdderSystem first so it runs before CheckerSystem within one frame
		// (no dependency ordering needed here; the deferred-visibility rule applies regardless).
		world.RegisterSystem(std::make_unique<AdderSystem>());
		world.RegisterSystem(std::make_unique<CheckerSystem>());

		// Frame 1: AdderSystem emplaces one CompB deferred. CheckerSystem must see 0.
		world.Update(pg::Timestep(0));
		CHECK(s_CheckerSawCount == 0);

		// Frame 2: the deferred add was flushed after frame 1. CheckerSystem must see 1.
		world.Update(pg::Timestep(0));
		CHECK(s_CheckerSawCount == 1);
	}

	// Test 5: a system declaring both writeSet={CompA} and addSet={CompA} registers
	// successfully and can both mutate existing CompA components and deferred-add new ones
	// in the same Update() without any assertions firing.
	TEST_CASE("Core.ECS.AccessControl::WriteSetAndAddSetMayOverlap")
	{
		s_OverlapSystemRan = false;

		pg::World& world = pg::World::Create();

		// Seed one entity with CompA so the write path has something to mutate.
		entt::entity e = pg::World::GetRegistryDirect().create();
		pg::World::GetRegistryDirect().emplace<CompA>(e);

		// RegisterSystem must not assert.
		world.RegisterSystem(std::make_unique<OverlapSystem>());

		// Update must not assert (write path + deferred add both declared).
		world.Update(pg::Timestep(0));

		REQUIRE(s_OverlapSystemRan);

		// After frame 1 the deferred add is flushed — now 2 entities have CompA.
		world.Update(pg::Timestep(0));
		int count = 0;
		for (auto ent : pg::World::GetRegistryDirect().view<CompA>())
		{
			(void)ent;
			++count;
		}
		CHECK(count == 2);
	}
} // End namespace: CatchTestsetFail

