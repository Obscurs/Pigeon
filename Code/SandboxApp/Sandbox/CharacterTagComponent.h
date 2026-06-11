#pragma once

namespace sbx
{
	// Marks the single arrow-key-controlled showcase character. CharacterControlSystem reads it to find
	// the entity it should move and animate from the live input state.
	struct CharacterTagComponent
	{
		CharacterTagComponent() = default;
		CharacterTagComponent(const CharacterTagComponent&) = default;

		bool m_Dummy = true;
	};
}
