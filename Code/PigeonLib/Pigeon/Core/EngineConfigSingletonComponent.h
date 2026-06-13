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

		// 3D pass: the flat-unlit model shader and the offscreen render target the 3D scene draws into.
		pg::UUID m_Model3DShaderID;
		pg::UUID m_Render3DTargetID;

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

		// Text-to-image generation defaults, overridable per Generate Image Request. Read from
		// Config.json; absent keys keep these defaults. The sampler is a backend-parsed name (kept a
		// plain string so Core carries no dependency on the Diffusion module's sampler enum).
		int m_DiffusionSteps = 20;
		float m_DiffusionCfgScale = 7.0f;
		std::string m_DiffusionSampler = "euler_a";
		unsigned int m_DiffusionWidth = 512;
		unsigned int m_DiffusionHeight = 512;
		int m_DiffusionClipSkip = 1;

		// Directory holding the savedata override Config.json (the savedataPath key), recorded so the
		// runtime knows where to persist changes. Empty when no savedata path is configured.
		std::string m_SavedataPath;
	};
}