#pragma once

namespace sbx
{
	// Bookkeeping for QuadSpawnSystem. Its presence also acts as the "scene already seeded"
	// guard, so the persistent demo quads are created exactly once. Read by UIStatusSystem.
	struct SpawnerSingletonComponent
	{
		SpawnerSingletonComponent() = default;
		SpawnerSingletonComponent(const SpawnerSingletonComponent&) = default;

		int m_SpawnCount{ 0 };
	};
}
