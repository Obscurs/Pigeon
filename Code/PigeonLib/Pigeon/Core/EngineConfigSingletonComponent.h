#pragma once
#include <string>

#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/UUID.h"

namespace pg
{
	struct EngineConfigSingletonComponent
	{
		EngineConfigSingletonComponent() {};
		EngineConfigSingletonComponent(const EngineConfigSingletonComponent&) = default;

		pg::UUID m_DefaultQuadShaderID;
		pg::UUID m_DefaultTextShaderID;
		pg::UUID m_DefaultFontID;

		// Audio mix levels read from Config.json; default to full volume when the keys are absent.
		float m_MasterVolume = 1.0f;
		float m_SoundVolume = 1.0f;
		float m_MusicVolume = 1.0f;

		// Default window resolution + display mode read from Config.json; absent keys keep these defaults.
		unsigned int m_WindowWidth = 1280;
		unsigned int m_WindowHeight = 720;
		pg::EWindowMode m_WindowMode = pg::EWindowMode::eWindowed;

		// UI authoring canvas (reference resolution) and the Unity-style match factor that derives the
		// UI scale factor from window-vs-reference (0 = match width, 1 = match height). Read from
		// Config.json;
		float m_UIReferenceWidth = 1920.f;
		float m_UIReferenceHeight = 1080.f;
		float m_UIMatchFactor = 0.5f;

		// Directory holding the savedata override Config.json (the savedataPath key), recorded so the
		// runtime knows where to persist changes. Empty when no savedata path is configured.
		std::string m_SavedataPath;
	};
}