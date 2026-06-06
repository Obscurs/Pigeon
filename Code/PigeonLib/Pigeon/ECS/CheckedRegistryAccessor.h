#pragma once
#include <typeindex>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/EventComponent.h"
#include "Pigeon/ECS/Entity.h"
#include "Pigeon/ECS/System.h"

namespace pg
{
	// Forward declaration so the deferred-push helpers can reach World in the .cpp.
	struct DeferredAdd;
	class World;

	class CheckedRegistryAccessor
	{
	public:
		// decl is copied to avoid dangling into World's system list. A system that does not
		// override DeclareAccess() receives an empty decl, so every component access asserts.
		// This forces all systems to declare their access explicitly.
		CheckedRegistryAccessor(pg::ecs::Registry& reg, const SystemAccessDecl& decl)
			: m_Registry(reg), m_Decl(decl)
		{}

		template<typename... Components>
		auto View()
		{
			// initializer-list expansion folds AssertReadOrWrite over the pack (C++17, no fold expr).
			(void)std::initializer_list<int>
			{
				(AssertReadOrWrite<std::remove_const_t<Components>>(), 0)...
			};
			return m_Registry.view<Components...>();
		}

		template<typename... Components, typename... Exclude>
		auto View(pg::ecs::exclude_t<Exclude...> excl)
		{
			(void)std::initializer_list<int>
			{
				(AssertReadOrWrite<std::remove_const_t<Components>>(), 0)...
			};
			return m_Registry.view<Components...>(excl);
		}

		template<typename Component>
		decltype(auto) Get(pg::ecs::Entity e)
		{
			AssertReadOrWrite<std::remove_const_t<Component>>();
			return m_Registry.get<Component>(e);
		}

		// Buffers the add until end-of-frame; the component becomes visible next frame.
		template<typename Component, typename... Args>
		void EmplaceDeferred(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			// Heap-allocated so the value survives until the deferred flush; freed by the destroy trampoline.
			Component* payload = new Component(std::forward<Args>(args)...);

			// Non-capturing lambdas decay to function pointers, so the trampolines need no per-call heap.
			PushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor*, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					// Double-add check lives here, where Component is still in scope.
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Buffers the add until end-of-frame; visible next frame, then auto-removed at the end of that frame.
		template<typename Component, typename... Args>
		void EmplaceOneframe(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of a component not in addSet");
			Component* payload = new Component(std::forward<Args>(args)...);

			PushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor* self, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					Component* comp = static_cast<Component*>(p);

					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");

					reg.emplace<Component>(ent, std::move(*comp));

					// Queue the matching remove so the component lives exactly one frame.
					Component* removePayload = new Component(*comp);
					self->PushDeferredOneFrameRequest(ent, removePayload, self,
						+[](CheckedRegistryAccessor*, pg::ecs::Registry& reg2, pg::ecs::Entity ent2, void*)
						{
							reg2.remove<Component>(ent2);
						},
						+[](void* p2) { delete static_cast<Component*>(p2); });
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Applied immediately; visible to systems that run later in the same frame.
		template<typename Component, typename... Args>
		void EmplaceInframe(pg::ecs::Entity e, Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.inframeAddSet.count(std::type_index(typeid(Component))),
				"System attempted in-frame add of a component not in inframeAddSet");
			PG_CORE_ASSERT(!m_Registry.all_of<Component>(e),
				"In-frame add: entity already has component");
			m_Registry.emplace<Component>(e, std::forward<Args>(args)...);
		}

		void DestroyDeferred(const pg::ecs::Entity& e)
		{
			PushDeferredDestroy(e);
		}

		// Buffers the remove until end-of-frame; the component is gone next frame.
		template<typename Component>
		void RemoveDeferred(pg::ecs::Entity e)
		{
			PushDeferredRequest(e, nullptr, this,
				+[](CheckedRegistryAccessor*, pg::ecs::Registry& reg, pg::ecs::Entity ent, void*)
				{
					reg.remove<Component>(ent);
				},
				+[](void*) {});
		}

		// Buffers an event entity until end-of-frame; visible next frame, then destroyed by ClearEvents.
		template<typename Component, typename... Args>
		void EmplaceEvent(Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.addSet.count(std::type_index(typeid(Component))),
				"System attempted deferred add of an event not in addSet");
			Component* payload = new Component(std::forward<Args>(args)...);

			pg::ecs::Entity e = m_Registry.create();
			PushDeferredRequest(e, payload, this,
				+[](CheckedRegistryAccessor*, pg::ecs::Registry& reg, pg::ecs::Entity ent, void* p)
				{
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<pg::EventComponent>(ent);
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); });
		}

		// Applied immediately; visible to systems that run later this frame, then destroyed by ClearEvents.
		template<typename Component, typename... Args>
		void EmplaceInframeEvent(Args&&... args)
		{
			PG_CORE_ASSERT(
				m_Decl.inframeAddSet.count(std::type_index(typeid(Component))),
				"System attempted in-frame add of an event not in inframeAddSet");
			pg::ecs::Entity e = m_Registry.create();
			m_Registry.emplace<Component>(e, std::forward<Args>(args)...);
			m_Registry.emplace<pg::EventComponent>(e);
		}

		// Entity lifecycle - no component-access restriction.
		pg::ecs::Entity Create()
		{
			return m_Registry.create();
		}

		void Destroy(pg::ecs::Entity e)
		{
			m_Registry.destroy(e);
		}

		// Membership queries - no component-access restriction.
		bool Valid(pg::ecs::Entity e) const
		{
			return m_Registry.valid(e);
		}

		template<typename... Components>
		bool AnyOf(pg::ecs::Entity e) const
		{
			return m_Registry.any_of<Components...>(e);
		}

		template<typename... Components>
		bool AllOf(pg::ecs::Entity e) const
		{
			return m_Registry.all_of<Components...>(e);
		}

	private:
		template<typename Component>
		void AssertReadOrWrite()
		{
			const std::type_index idx(typeid(Component));
			PG_CORE_ASSERT(
				m_Decl.readSet.count(idx) || m_Decl.writeSet.count(idx),
				"System accessed a component not declared in readSet or writeSet");
		}

		// Defined out-of-line in World.cpp to avoid a circular include with World.h.
		void PushDeferredRequest(pg::ecs::Entity e, void* payload, CheckedRegistryAccessor* self,
			void(*apply)(CheckedRegistryAccessor*, pg::ecs::Registry&, pg::ecs::Entity, void*),
			void(*destroy)(void*));

		void PushDeferredOneFrameRequest(pg::ecs::Entity e, void* payload, CheckedRegistryAccessor* self,
			void(*apply)(CheckedRegistryAccessor*, pg::ecs::Registry&, pg::ecs::Entity, void*),
			void(*destroy)(void*));

		void PushDeferredDestroy(const pg::ecs::Entity& e);

		pg::ecs::Registry& m_Registry;
		SystemAccessDecl m_Decl;
	};
}
