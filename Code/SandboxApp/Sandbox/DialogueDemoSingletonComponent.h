#pragma once

#include <string>

namespace sbx
{
	// State for the typewriter dialogue demo. m_TextBuffer is the editable line (driven by the ImGui
	// widget); m_LastText is the line the reveal is currently playing, used to detect an edit and restart
	// the reveal from the start; m_RevealedChars is the reveal progress in characters (fractional so it
	// advances smoothly with frame time); m_CharsPerSecond is the reveal speed. Owned by
	// DialogueDemoSystem and seeded once with a default line.
	struct DialogueDemoSingletonComponent
	{
		DialogueDemoSingletonComponent() = default;
		DialogueDemoSingletonComponent(const DialogueDemoSingletonComponent&) = default;

		char m_TextBuffer[512] = "Welcome to Pigeon. Edit this line and watch it type itself out, one character at a time.";
		std::string m_LastText{};
		float m_RevealedChars{ 0.f };
		float m_CharsPerSecond{ 24.f };
	};
}
