#pragma once

#include "Pigeon/ECS/System.h"

namespace sbx
{
	// ImGui "Window" panel: a preset-resolution dropdown + a windowed/fullscreen radio + an Apply
	// button. On Apply it emits a SetWindowResolutionRequestOneFrameComponent, which the engine
	// WindowConfigSystem applies to the live window and persists to the savedata config.
	class WindowDebugPanelSystem : public pg::System
	{
	public:
		WindowDebugPanelSystem() = default;
		~WindowDebugPanelSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
