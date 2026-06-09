#pragma once

#include <string>

namespace pg
{
	enum class EWindowMode
	{
		eWindowed,
		eFullscreen
	};

	// Serialised form used in Config.json. Unknown / absent values map to windowed.
	inline const char* WindowModeToString(EWindowMode mode)
	{
		return mode == EWindowMode::eFullscreen ? "fullscreen" : "windowed";
	}

	inline EWindowMode WindowModeFromString(const std::string& value)
	{
		return value == "fullscreen" ? EWindowMode::eFullscreen : EWindowMode::eWindowed;
	}
}
