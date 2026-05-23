#pragma once
#include <entt/entt.hpp>
#include <typeindex>
#include <vector>
#include <unordered_set>

#include "Pigeon/Core/EventComponent.h"
#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/CheckedRegistryAccessor.h"

namespace pig
{
	struct DeferredRequest
	{
		entt::entity entity;
		void*        payload;                                // heap-allocated component value
		void(*apply)(entt::registry&, entt::entity, void*); // typed trampoline, no virtual dispatch
		void(*destroy)(void*);                               // cleanup if entity is invalid at flush time
	};

	class World
	{
	public:
		World() = default;
		static World& Create()
		{
			s_Instance = std::make_unique<World>();
			s_Instance->Init();
			return s_Instance->Get();
		}

		void Update(const pig::Timestep& ts);
		void RegisterSystem(std::unique_ptr<pig::System> system);

		inline static World& Get() { return *s_Instance; }

		// Returns a checked accessor — asserts m_ActiveSystem != nullptr.
		// Call only from within a system Update().
		static CheckedRegistryAccessor GetRegistry();

		// For non-System callers (Layers, tooling). Does not enforce access declarations.
		static entt::registry& GetRegistryDirect();

		inline static entt::dispatcher& GetDispatcher() { return s_Instance->m_Dispatcher; }

		void PushDeferredRequest(DeferredRequest op);
		void PushDeferredDestroy(const entt::entity& entity);

		// Deferred add — asserts Component is in addSet, buffers the operation until end-of-frame.
		template<typename Component, typename... Args>
		void EmplaceExternalEvent(Args&&... args)
		{
			// Allocate the component value on the heap (unavoidable for type-erasure).
			auto* payload = new Component(std::forward<Args>(args)...);

			entt::entity e = m_Registry.create();
			// Static template instantiations — no per-call heap allocation for the trampolines.
			// Non-capturing lambdas are implicitly convertible to function pointers in C++17.
			PushDeferredRequest({ e, payload,
				+[](entt::registry& reg, entt::entity ent, void* p)
				{
					// Double-add assertion lives here where Component is in scope.
					PG_CORE_ASSERT(!reg.all_of<Component>(ent),
						"Deferred add: entity already has component");
					reg.emplace<pig::EventComponent>(ent, std::move(*static_cast<pig::EventComponent*>(p)));
					reg.emplace<Component>(ent, std::move(*static_cast<Component*>(p)));
				},
				+[](void* p) { delete static_cast<Component*>(p); } });
		}

	private:
		void Init();
		void SortSystems();
		void FlushDeferredRequests();
		void ClearEvents();

		struct SystemEntry
		{
			pig::U_Ptr<pig::System> system;
			SystemAccessDecl        decl;
		};

		entt::registry m_Registry;
		entt::dispatcher m_Dispatcher;
		std::vector<SystemEntry> m_Systems;
		std::unordered_set<std::type_index> m_SystemTypes;

		// Active system pointer — set around each system's Update() call.
		const System* m_ActiveSystem = nullptr;

		// True once SortSystems() has been called and m_Systems is in dependency order.
		bool m_Sorted = false;

		// Buffer for deferred component adds (flushed after all systems Update()).
		std::vector<DeferredRequest> m_DeferredRequests;
		std::vector<entt::entity> m_DeferredDestroys;
		static pig::U_Ptr<World> s_Instance;
	};
}
