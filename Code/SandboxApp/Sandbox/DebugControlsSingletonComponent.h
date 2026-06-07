#pragma once

namespace sbx
{
	// Live tuning values driven by the ImGui debug panel (DebugPanelSystem) and read by other
	// systems so an ImGui widget can feed the ECS without bypassing it.
	struct DebugControlsSingletonComponent
	{
		DebugControlsSingletonComponent() = default;
		DebugControlsSingletonComponent(const DebugControlsSingletonComponent&) = default;

		float m_AnimationSpeed{ 1.f }; // global multiplier on quad animation speed
	};
}
