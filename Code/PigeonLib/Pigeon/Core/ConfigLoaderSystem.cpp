#include "pch.h"
#include "Pigeon/Core/ConfigLoaderSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/EWindowMode.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/Core/FileUtils.h"
#include "Pigeon/ECS/World.h"

namespace
{
	// Applies any engine-config entries present in jsonObject onto component.
	// Every field is optional: absent keys leave the existing value untouched, so
	// the same routine seeds the base engine config and overlays the savedata
	// overrides on top of it.
	void ApplyConfigEntries(pg::EngineConfigSingletonComponent& component, const json& jsonObject)
	{
		if (jsonObject.contains("defaultQuadShader") && jsonObject["defaultQuadShader"].is_string())
		{
			component.m_DefaultQuadShaderID = pg::UUID(jsonObject["defaultQuadShader"].get<std::string>());
		}
		if (jsonObject.contains("defaultTextShader") && jsonObject["defaultTextShader"].is_string())
		{
			component.m_DefaultTextShaderID = pg::UUID(jsonObject["defaultTextShader"].get<std::string>());
		}
		if (jsonObject.contains("defaultFont") && jsonObject["defaultFont"].is_string())
		{
			component.m_DefaultFontID = pg::UUID(jsonObject["defaultFont"].get<std::string>());
		}
		if (jsonObject.contains("model3DShader") && jsonObject["model3DShader"].is_string())
		{
			component.m_Model3DShaderID = pg::UUID(jsonObject["model3DShader"].get<std::string>());
		}
		if (jsonObject.contains("render3DTarget") && jsonObject["render3DTarget"].is_string())
		{
			component.m_Render3DTargetID = pg::UUID(jsonObject["render3DTarget"].get<std::string>());
		}
		if (jsonObject.contains("masterVolume") && jsonObject["masterVolume"].is_number())
		{
			component.m_MasterVolume = jsonObject["masterVolume"].get<float>();
		}
		if (jsonObject.contains("soundVolume") && jsonObject["soundVolume"].is_number())
		{
			component.m_SoundVolume = jsonObject["soundVolume"].get<float>();
		}
		if (jsonObject.contains("musicVolume") && jsonObject["musicVolume"].is_number())
		{
			component.m_MusicVolume = jsonObject["musicVolume"].get<float>();
		}
		if (jsonObject.contains("windowWidth") && jsonObject["windowWidth"].is_number_unsigned())
		{
			component.m_WindowWidth = jsonObject["windowWidth"].get<unsigned int>();
		}
		if (jsonObject.contains("windowHeight") && jsonObject["windowHeight"].is_number_unsigned())
		{
			component.m_WindowHeight = jsonObject["windowHeight"].get<unsigned int>();
		}
		if (jsonObject.contains("windowMode") && jsonObject["windowMode"].is_string())
		{
			component.m_WindowMode = pg::WindowModeFromString(jsonObject["windowMode"].get<std::string>());
		}
		if (jsonObject.contains("uiReferenceWidth") && jsonObject["uiReferenceWidth"].is_number())
		{
			component.m_UIReferenceWidth = jsonObject["uiReferenceWidth"].get<float>();
		}
		if (jsonObject.contains("uiReferenceHeight") && jsonObject["uiReferenceHeight"].is_number())
		{
			component.m_UIReferenceHeight = jsonObject["uiReferenceHeight"].get<float>();
		}
		if (jsonObject.contains("uiMatchFactor") && jsonObject["uiMatchFactor"].is_number())
		{
			component.m_UIMatchFactor = jsonObject["uiMatchFactor"].get<float>();
		}
	}
}

pg::SystemAccessDecl pg::ConfigLoaderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::EngineConfigSingletonComponent)),
	};
	return decl;
}

void pg::ConfigLoaderSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	if (accessor.View<const pg::EngineConfigSingletonComponent>().empty())
	{
		pg::EngineConfigSingletonComponent component;
		pg::ecs::Entity ent = accessor.Create();
		std::string configStr = pg::ReadFileToString("Assets/Engine/Config.json");
		json jsonObject = json::parse(configStr);

		PG_CORE_EXCEPT(jsonObject.contains("defaultQuadShader") && jsonObject["defaultQuadShader"].is_string(), "Missing defaultQuadShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultTextShader") && jsonObject["defaultTextShader"].is_string(), "Missing defaultTextShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultFont") && jsonObject["defaultFont"].is_string(), "Missing defaultFont in engine config");

		// Audio volumes are optional; absent keys keep the full-volume defaults.
		ApplyConfigEntries(component, jsonObject);

		// Savedata override: entries in <savedataPath>/Config.json override the
		// matching engine config values. The file is optional — when it is absent
		// the engine config values stand unchanged.
		if (jsonObject.contains("savedataPath") && jsonObject["savedataPath"].is_string())
		{
			// Record the savedata directory so the runtime knows where to persist config changes.
			component.m_SavedataPath = jsonObject["savedataPath"].get<std::string>();

			const std::string savedataConfigPath = component.m_SavedataPath + "/Config.json";
			if (pg::FileExists(savedataConfigPath))
			{
				const json savedataJson = json::parse(pg::ReadFileToString(savedataConfigPath));
				ApplyConfigEntries(component, savedataJson);
			}
		}

		accessor.EmplaceDeferred<pg::EngineConfigSingletonComponent>(ent, std::move(component));
	}
}
