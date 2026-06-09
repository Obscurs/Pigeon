#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	// Owns the live WindowConfigSingletonComponent: seeds it from the engine config (applying the
	// configured resolution to the live window on startup), and applies + persists runtime resolution
	// requests. The live OS window is reached through the Application host (which owns it); in the
	// Testing build no Application exists, so the window apply is skipped and the seed / request /
	// persist logic stays unit-testable without a real window.
	class WindowConfigSystem : public pg::System
	{
	public:
		WindowConfigSystem() = default;
		~WindowConfigSystem() = default;

		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
