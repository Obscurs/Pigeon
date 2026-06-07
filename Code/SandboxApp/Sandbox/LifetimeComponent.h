#pragma once

namespace sbx
{
	// Seconds of life remaining for a spawned entity. LifetimeSystem counts this down each frame
	// and deferred-destroys the entity once it reaches zero.
	struct LifetimeComponent
	{
		LifetimeComponent() = default;
		LifetimeComponent(const LifetimeComponent&) = default;

		float m_Remaining{ 0.f };
	};
}
