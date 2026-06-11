#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// Typewriter dialogue demo. An ImGui "Dialogue" panel edits a line of text and its reveal speed;
	// each frame the system advances the reveal and drives a fixed-size word-wrapped UI text element
	// (via UIUpdateTextRevealOneFrameComponent) so the line types itself out. Editing the text restarts
	// the reveal from the first character.
	class DialogueDemoSystem : public pg::System
	{
	public:
		DialogueDemoSystem() = default;
		~DialogueDemoSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
