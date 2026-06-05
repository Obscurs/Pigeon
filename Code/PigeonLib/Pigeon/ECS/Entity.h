#pragma once
#include <entt/entt.hpp>

// The ECS abstraction layer. This header is the only place in the engine that names the
// underlying ECS library's (EnTT) public types. Engine and game code use the pg::ecs names
// below exclusively, so the library can be wrapped, audited, or swapped at this single seam.
// See docs/adr/0002-pg-ecs-entity-abstraction-over-entt.md.
namespace pg::ecs
{
	// Handle to an entity.
	using Entity = entt::entity;

	// Sentinel value representing "no entity".
	inline constexpr entt::null_t null = entt::null;

	// Owns entities and their components.
	using Registry = entt::registry;

	// Delivers signals/events between systems.
	using Dispatcher = entt::dispatcher;

	// Filter type that removes entities owning the listed components from a view.
	template<typename... Type>
	using exclude_t = entt::exclude_t<Type...>;

	// Ready-made exclusion filter, e.g. registry.view<A>(pg::ecs::exclude<B>).
	template<typename... Type>
	inline constexpr entt::exclude_t<Type...> exclude = entt::exclude<Type...>;
}
