#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// Text-generation demo panel: an ImGui window with a prompt box, temperature + max-tokens controls,
	// and a "Generate" button that emits a GenerateTextRequest (engine-typed) targeting the demo's
	// generated-text UUID. While a job runs the button is disabled and a busy note is shown; the latest
	// completion (read from TextGenResultSingletonComponent by the target UUID) is displayed below.
	// Mirrors the other ImGui debug panels: it seeds its editable state in ECS and guards every ImGui
	// call so the headless test build (no ImGui context) is a no-op.
	class TextGenDemoSystem : public pg::System
	{
	public:
		TextGenDemoSystem() = default;
		~TextGenDemoSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
