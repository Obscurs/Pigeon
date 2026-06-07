#pragma once

namespace sbx
{
	// Marks the single world-space label whose text InputReadoutSystem rewrites each frame from
	// the live input state.
	struct InputReadoutTagComponent
	{
		InputReadoutTagComponent() = default;
		InputReadoutTagComponent(const InputReadoutTagComponent&) = default;

		bool m_Dummy = true;
	};
}
