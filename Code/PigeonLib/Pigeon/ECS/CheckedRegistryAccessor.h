#pragma once
#include <entt/entt.hpp>
#include <typeindex>

#include "Pigeon/Core/Core.h"
#include "Pigeon/ECS/System.h"

// IMPORTANT: CheckedRegistryAccessor is a per-call transient. Do NOT store it across frames.
// Obtain it each time via pig::World::GetRegistry() (inside a system Update()).
// Non-system callers (Layers, tooling) use pig::World::GetRegistryDirect().

namespace pig
{
	// Forward declaration so emplace_deferred can call World::Get().PushDeferredAdd(...)
	struct DeferredAdd;
	class World;

	class CheckedRegistryAccessor
	{
	public:
		// Checked constructor — used by GetRegistry(); decl is copied to avoid dangling into m_Systems.
		// Systems that do not override DeclareAccess() will receive an empty decl and assertions will
		// fire on any component access — this forces all systems to declare their access explicitly.
		CheckedRegistryAccessor(entt::registry& reg, const SystemAccessDecl& decl)
			: m_Registry(reg), m_Decl(decl)
		{}

		// View path — asserts all template args are in readSet union writeSet.
		template<typename... Components>
		auto view()
		{
			// Use initializer-list expansion trick for C++17 fold over pack
			(void)std::initializer_list<int>
			{
				(assertReadOrWrite<std::remove_const_t<Components>>(), 0)...
			};
			return m_Registry.view<Components...>();
		}

		// View path with exclusion filter.
		template<typename... Components, typename... Exclude>
		auto view(entt::exclude_t<Exclude...> excl)
		{
			(void)std::initializer_list<int>
			{
				(assertReadOrWrite<std::remove_const_t<Components>>(), 0)...
			};
			return m_Registry.view<Components...>(excl);
		}

		// Single-component get — asserts Component is in readSet or writeSet.
		template<typename Component>
		decltype(auto) get(entt::entity e)
		{
			assertReadOrWrite<std::remove_const_t<Component>>();
			return m_Registry.get<Component>(e);
		}

		// Deferred add — asserts Component is in addSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void emplace_deferred(entt::entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			// Allocate the component value on the heap (unavoidable for type-erasure).
			auto* payload = new Component(std::forward<Args>(args)...);

			// Static template instantiations — no per-call heap allocation for the trampolines.
			// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
			pushDeferredAdd(e, payload,
				+[](entt::registry& reg, entt::entity ent, void* p)
				{
					// Double-add assertion lives here where Component is in scope.
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// patch — asserts Component is in writeSet.
		template<typename Component, typename... Func>
		void patch(entt::entity e, Func&&... func)
		{
			PG_CORE_ASSERT(
				m_Decl.writeSet.count(std::type_index(typeid(Component))),
				"System attempted to patch a component not in writeSet");
			m_Registry.patch<Component>(e, std::forward<Func>(func)...);
		}

		// replace — asserts Component is in writeSet.
		template<typename Component>
		Component& replace(entt::entity e, const Component& value)
		{
			PG_CORE_ASSERT(
				m_Decl.writeSet.count(std::type_index(typeid(Component))),
				"System attempted to replace a component not in writeSet");
			return m_Registry.replace<Component>(e, value);
		}

		// erase — asserts Component is in writeSet.
		template<typename Component>
		void erase(entt::entity e)
		{
			PG_CORE_ASSERT(
				m_Decl.writeSet.count(std::type_index(typeid(Component))),
				"System attempted to erase a component not in writeSet");
			m_Registry.erase<Component>(e);
		}

		// remove — asserts Component is in writeSet. remove<> returns the number of removed components.
		template<typename Component>
		size_t remove(entt::entity e)
		{
			PG_CORE_ASSERT(
				m_Decl.writeSet.count(std::type_index(typeid(Component))),
				"System attempted to remove a component not in writeSet");
			return m_Registry.remove<Component>(e);
		}

		// Entity lifecycle — no component access restriction.
		entt::entity create()
		{
			return m_Registry.create();
		}

		void destroy(entt::entity e)
		{
			m_Registry.destroy(e);
		}

		// Non-access queries — pass through with no assertion.
		bool valid(entt::entity e) const
		{
			return m_Registry.valid(e);
		}

		template<typename... Components>
		bool any_of(entt::entity e) const
		{
			return m_Registry.any_of<Components...>(e);
		}

		template<typename... Components>
		bool all_of(entt::entity e) const
		{
			return m_Registry.all_of<Components...>(e);
		}

		// Expose the raw registry so system code can pass it to free helper functions that
		// need registry access but are not systems themselves.
		entt::registry& GetInternalRegistry() { return m_Registry; }

	private:
		template<typename Component>
		void assertReadOrWrite()
		{
			const std::type_index idx(typeid(Component));
			PG_CORE_ASSERT(
				m_Decl.readSet.count(idx) || m_Decl.writeSet.count(idx),
				"System accessed a component not declared in readSet or writeSet");
		}

		// Defined out-of-line in World.cpp to avoid circular include with World.h.
		void pushDeferredAdd(entt::entity e, void* payload,
			void(*apply)(entt::registry&, entt::entity, void*),
			void(*destroy)(void*));

		entt::registry&  m_Registry;
		SystemAccessDecl m_Decl;
	};
}
