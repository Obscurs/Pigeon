#pragma once
#include <typeindex>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/EventComponent.h"
#include "Pigeon/ECS/Entity.h"
#include "Pigeon/ECS/System.h"

namespace pg
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
		CheckedRegistryAccessor(pg::ecs::Registry& reg, const SystemAccessDecl& decl)
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
		auto view(pg::ecs::exclude_t<Exclude...> excl)
		{
			(void)std::initializer_list<int>
			{
				(assertReadOrWrite<std::remove_const_t<Components>>(), 0)...
			};
			return m_Registry.view<Components...>(excl);
		}

		// Single-component get — asserts Component is in readSet or writeSet.
		template<typename Component>
		decltype(auto) get(pg::ecs::Entity e)
		{
			assertReadOrWrite<std::remove_const_t<Component>>();
			return m_Registry.get<Component>(e);
		}

		// Deferred add — asserts Component is in addSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void emplace_deferred(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			// Allocate the component value on the heap (unavoidable for type-erasure).
			auto* payload = new Component(std::forward<Args>(args)...);

			// Static template instantiations — no per-call heap allocation for the trampolines.
			// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
			pushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					// Double-add assertion lives here where Component is in scope.
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Deferred add — asserts Component is in addSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void emplace_oneframe(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			// Allocate the component value on the heap (unavoidable for type-erasure).
			auto* payload = new Component(std::forward<Args>(args)...);

			// Static template instantiations — no per-call heap allocation for the trampolines.
			// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
			pushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					auto* comp = static_cast<Component*>(p);

					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");

					reg.emplace<Component>(ent, std::move(*comp));

					auto* payload2 = new Component(*comp);

					// Static template instantiations — no per-call heap allocation for the trampolines.
					// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
					self->pushDeferredOneFrameRequest(ent, payload2, self,
						+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg2, pg::ecs::Entity ent2, void*)
						{
							reg2.remove<Component>(ent2);
						},
						+[](void* p2) { delete static_cast<Component*>(p2); });

				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Deferred add — asserts Component is in inframeAddSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void emplace_inframe(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.inframeAddSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			
			PG_CORE_ASSERT(!m_Registry.all_of<Component>(ent),
				"Deferred add: entity already has component");

			auto* payload = new Component(std::forward<Args>(args)...);
			m_Registry.emplace<Component>(e, std::move(*static_cast<Component*>(payload)));
		}

		void destroy_deferred(const pg::ecs::Entity& e)
		{
			pushDeferredDestroy(e);
		}

		// Deferred remove — buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void remove_deferred(pg::ecs::Entity e, Args&&... args)
		{
			auto* payload = new Component(std::forward<Args>(args)...);

			pushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					reg.remove<Component>(ent);
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Deferred add — asserts Component is in addSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void EmplaceEvent(Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			// Allocate the component value on the heap (unavoidable for type-erasure).
			auto* payload = new Component(std::forward<Args>(args)...);

			pg::ecs::Entity e = m_Registry.create();
			// Static template instantiations — no per-call heap allocation for the trampolines.
			// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
			pushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					// Double-add assertion lives here where Component is in scope.
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<pg::EventComponent>(ent);
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Deferred add — asserts Component is in inframeAddSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void EmplaceInframeEvent(Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.inframeAddSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			auto* payload = new Component(std::forward<Args>(args)...);

			pg::ecs::Entity e = m_Registry.create();
			m_Registry.emplace<Component>(e, std::move(*static_cast<Component*>(payload)));
			m_Registry.emplace<pg::EventComponent>(e);
		}

		// Entity lifecycle — no component access restriction.
		pg::ecs::Entity create()
		{
			return m_Registry.create();
		}

		void destroy(pg::ecs::Entity e)
		{
			m_Registry.destroy(e);
		}

		// Non-access queries — pass through with no assertion.
		bool valid(pg::ecs::Entity e) const
		{
			return m_Registry.valid(e);
		}

		template<typename... Components>
		bool any_of(pg::ecs::Entity e) const
		{
			return m_Registry.any_of<Components...>(e);
		}

		template<typename... Components>
		bool all_of(pg::ecs::Entity e) const
		{
			return m_Registry.all_of<Components...>(e);
		}

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
		void pushDeferredRequest(pg::ecs::Entity e, void* payload, CheckedRegistryAccessor* self,
			void(*apply)(CheckedRegistryAccessor*, pg::ecs::Registry&, pg::ecs::Entity, void*),
			void(*destroy)(void*));

		void pushDeferredOneFrameRequest(pg::ecs::Entity e, void* payload, CheckedRegistryAccessor* self,
			void(*apply)(CheckedRegistryAccessor*, pg::ecs::Registry&, pg::ecs::Entity, void*),
			void(*destroy)(void*));

		void pushDeferredDestroy(const pg::ecs::Entity& e);

		pg::ecs::Registry&  m_Registry;
		SystemAccessDecl m_Decl;
	};
}
