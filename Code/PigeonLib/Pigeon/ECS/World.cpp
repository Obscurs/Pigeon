#include "pch.h"

#include "World.h"

#include "Pigeon/ECS/System.h"
#include "Pigeon/ECS/CheckedRegistryAccessor.h"

pig::U_Ptr<pig::World> pig::World::s_Instance = nullptr;

void pig::CheckedRegistryAccessor::pushDeferredRequest(
	entt::entity e, void* payload,
	void(*apply)(entt::registry&, entt::entity, void*),
	void(*destroy)(void*))
{
	pig::World::Get().PushDeferredRequest({ e, payload, apply, destroy });
}

void pig::CheckedRegistryAccessor::pushDeferredDestroy(const entt::entity& e)
{
	pig::World::Get().PushDeferredDestroy(e);
}

// ---- World::GetRegistry ----
pig::CheckedRegistryAccessor pig::World::GetRegistry()
{
	PG_CORE_ASSERT(s_Instance->m_ActiveSystem != nullptr,
		"GetRegistry() called outside of a system Update().");

	// Find the decl for the active system (linear search; m_Systems is small).
	for (auto& entry : s_Instance->m_Systems)
	{
		if (entry.system.get() == s_Instance->m_ActiveSystem)
			return CheckedRegistryAccessor(s_Instance->m_Registry, entry.decl); // copies decl
	}

	PG_CORE_ASSERT(false, "Active system not found in m_Systems");
	// Unreachable; return a dummy checked accessor with an empty decl so the compiler is satisfied.
	static SystemAccessDecl s_EmptyDecl;
	return CheckedRegistryAccessor(s_Instance->m_Registry, s_EmptyDecl);
}

entt::registry& pig::World::GetRegistryDirect()
{
	return s_Instance->m_Registry;
}

void pig::World::PushDeferredRequest(pig::DeferredRequest op)
{
	m_DeferredRequests.push_back(std::move(op));
}

void pig::World::PushDeferredDestroy(const entt::entity& entity)
{
	m_DeferredDestroys.push_back(entity);
}

// ---- World::Update ----
void pig::World::Update(const pig::Timestep& ts)
{
	if (!m_Sorted)
	{
		SortSystems();
		m_Sorted = true;
	}

	m_Dispatcher.update();

	for (auto& entry : m_Systems)
	{
		m_ActiveSystem = entry.system.get();
		entry.system->Update(ts);
		m_ActiveSystem = nullptr;
	}
	ClearEvents();
	FlushDeferredRequests();
}

// ---- World::RegisterSystem ----
void pig::World::RegisterSystem(std::unique_ptr<pig::System> system)
{
	// Guard: registering a system during Update would invalidate m_Systems iterators.
	PG_CORE_ASSERT(!m_ActiveSystem, "RegisterSystem called during Update — forbidden");

	const std::type_index typeIdx = std::type_index(typeid(*system.get()));
	PG_CORE_ASSERT(m_SystemTypes.find(typeIdx) == m_SystemTypes.end(), "System already registered");

	SystemAccessDecl decl = system->DeclareAccess();

	// Enforce one-writer-per-component: for each type in this system's writeSet or addSet,
	// no previously registered system may also write or add it.
	for (const auto& entry : m_Systems)
	{
		for (const auto& t : decl.writeSet)
		{
			PG_CORE_ASSERT(!entry.decl.writeSet.count(t),
				"One-writer rule violated: two systems write the same component type");
		}
		for (const auto& t : decl.addSet)
		{
			PG_CORE_ASSERT(!entry.decl.addSet.count(t),
				"One-writer rule violated: two systems add the same component type");
		}
	}

	m_SystemTypes.insert(typeIdx);
	m_Systems.push_back({ std::move(system), std::move(decl) });
	m_Sorted = false; // force re-sort before next Update
}

// ---- World::SortSystems ----
void pig::World::SortSystems()
{
	// addSet components are visible next frame only; writeSet components are visible same frame.
	// Only writeSet generates ordering edges.
	//
	// Edge A -> B means: A must run before B.
	// An edge exists when system A has type T in its writeSet, and system B has T in its readSet.

	const int N = static_cast<int>(m_Systems.size());
	if (N == 0) return;

	// Build adjacency list and in-degree counts.
	// Use a per-source edge set to deduplicate: if system A writes {X, Y} and system B reads
	// {X, Y}, we must only add one edge A->B (not one per shared component type).
	std::vector<std::vector<int>> adj(N);
	std::vector<int> inDegree(N, 0);
	std::vector<std::unordered_set<int>> edgeSet(N); // tracks already-added destinations per source

	for (int a = 0; a < N; ++a)
	{
		for (const auto& t : m_Systems[a].decl.writeSet)
		{
			for (int b = 0; b < N; ++b)
			{
				if (a == b) continue;
				if (m_Systems[b].decl.readSet.count(t))
				{
					// Only add edge A->B once regardless of how many shared component types exist.
					if (edgeSet[a].insert(b).second)
					{
						adj[a].push_back(b);
						++inDegree[b];
					}
				}
			}
		}
	}

	// Kahn's algorithm (BFS). Process nodes in insertion order as tiebreaker.
	std::vector<int> queue;
	queue.reserve(N);
	for (int i = 0; i < N; ++i)
	{
		if (inDegree[i] == 0)
			queue.push_back(i);
	}

	std::vector<SystemEntry> sorted;
	sorted.reserve(N);
	int head = 0;

	while (head < static_cast<int>(queue.size()))
	{
		int cur = queue[head++];
		sorted.push_back(std::move(m_Systems[cur]));

		for (int nb : adj[cur])
		{
			if (--inDegree[nb] == 0)
				queue.push_back(nb);
		}
	}

	PG_CORE_ASSERT(static_cast<int>(sorted.size()) == N,
		"Cycle detected in system dependency graph — cannot determine valid execution order");

	m_Systems = std::move(sorted);
}

void pig::World::FlushDeferredRequests()
{
	// Must not be called while a system Update() is in progress.
	PG_CORE_ASSERT(m_ActiveSystem == nullptr, "FlushDeferredRequests called during Update");

	for (auto& op : m_DeferredRequests)
	{
		if (m_Registry.valid(op.entity))
			op.apply(m_Registry, op.entity, op.payload);
		op.destroy(op.payload);
	}
	m_DeferredRequests.clear();

	for (const entt::entity& entity : m_DeferredDestroys)
	{
		if (m_Registry.valid(entity))
			m_Registry.destroy(entity);
	}
	m_DeferredDestroys.clear();
}

void pig::World::ClearEvents()
{
	auto view = m_Registry.view<const pig::EventComponent>();
	for (auto ent : view)
	{
		m_Registry.destroy(ent);
	}
}

// ---- World::Init ----
void pig::World::Init()
{
	// Guard against mid-frame teardown.
	PG_CORE_ASSERT(!m_ActiveSystem, "Init called during Update");

	// Clean up any outstanding deferred payloads to avoid leaks.
	for (auto& op : m_DeferredRequests)
		op.destroy(op.payload);
	m_DeferredRequests.clear();

	m_Sorted = false;

	// Clear duplicate-registration guard so systems can be re-registered after Init.
	m_SystemTypes.clear();

	m_Systems.clear();
	m_Registry.clear();
	m_Dispatcher.clear();
}
