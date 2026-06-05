#pragma once
#include <catch2/catch.hpp>
#include <entt/entt.hpp>
#include <type_traits>

#include <Pigeon/ECS/Entity.h>
#include <Pigeon/ECS/World.h>

// These tests pin the pg::ecs abstraction layer over the underlying ECS library.
// Engine and game code must use the pg::ecs names; entt:: is named only here (to
// prove the aliasing) and inside the ECS abstraction header itself.
namespace
{
	struct EntityTestComp { int value = 0; };
}

namespace CatchTestsetFail
{
	// The abstraction must expose Entity / null / Registry / Dispatcher / exclude,
	// each backed by the corresponding entt type with zero overhead (type aliases).
	TEST_CASE("Core.ECS.Entity::AliasesBackingTypes")
	{
		STATIC_REQUIRE(std::is_same_v<pg::ecs::Entity, entt::entity>);
		STATIC_REQUIRE(std::is_same_v<pg::ecs::Registry, entt::registry>);
		STATIC_REQUIRE(std::is_same_v<pg::ecs::Dispatcher, entt::dispatcher>);
	}

	// A default pg::ecs::Entity initialised from pg::ecs::null must compare equal to it,
	// and an entity created by a registry must not.
	TEST_CASE("Core.ECS.Entity::NullSentinel")
	{
		pg::ecs::Entity none{ pg::ecs::null };
		// Extra parens: keep Catch's expression decomposition from clashing with
		// the entity-vs-null operator overload (ambiguous otherwise).
		CHECK((none == pg::ecs::null));

		pg::ecs::Registry registry;
		pg::ecs::Entity created = registry.create();
		CHECK((created != pg::ecs::null));
	}

	// The Registry alias must behave like the real registry: create, emplace, get.
	TEST_CASE("Core.ECS.Entity::RegistryRoundTrip")
	{
		pg::ecs::Registry registry;
		pg::ecs::Entity e = registry.create();
		registry.emplace<EntityTestComp>(e, EntityTestComp{ 7 });

		CHECK(registry.get<EntityTestComp>(e).value == 7);
		CHECK(registry.valid(e));
	}

	// The World public API must speak pg::ecs::Entity / pg::ecs::Registry / pg::ecs::Dispatcher.
	TEST_CASE("Core.ECS.Entity::WorldApiUsesPgEcsTypes")
	{
		pg::World::Create();

		pg::ecs::Registry& registry = pg::World::GetRegistryDirect();
		pg::ecs::Entity e = registry.create();
		registry.emplace<EntityTestComp>(e, EntityTestComp{ 3 });
		CHECK(registry.get<EntityTestComp>(e).value == 3);

		pg::ecs::Dispatcher& dispatcher = pg::World::GetDispatcher();
		(void)dispatcher;
	}

	// The pg::ecs::exclude filter must be usable to build an excluding view.
	TEST_CASE("Core.ECS.Entity::ExcludeFilter")
	{
		pg::ecs::Registry registry;
		pg::ecs::Entity withBoth = registry.create();
		registry.emplace<EntityTestComp>(withBoth, EntityTestComp{ 1 });
		registry.emplace<int>(withBoth, 5);

		pg::ecs::Entity onlyComp = registry.create();
		registry.emplace<EntityTestComp>(onlyComp, EntityTestComp{ 2 });

		int seen = 0;
		pg::ecs::Entity lastSeen{ pg::ecs::null };
		for (pg::ecs::Entity ent : registry.view<EntityTestComp>(pg::ecs::exclude<int>))
		{
			++seen;
			lastSeen = ent;
		}
		CHECK(seen == 1);
		CHECK((lastSeen == onlyComp));
	}
}
