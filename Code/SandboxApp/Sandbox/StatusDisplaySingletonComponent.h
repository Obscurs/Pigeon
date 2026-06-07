#pragma once

namespace sbx
{
	// Remembers the spawn count last written to the UI status text so UIStatusSystem only emits a
	// text update when the count actually changes. -1 forces the first update.
	struct StatusDisplaySingletonComponent
	{
		StatusDisplaySingletonComponent() = default;
		StatusDisplaySingletonComponent(const StatusDisplaySingletonComponent&) = default;

		int m_LastSpawnCount = -1;
	};
}
