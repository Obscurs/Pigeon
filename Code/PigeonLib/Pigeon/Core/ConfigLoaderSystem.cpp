#include "pch.h"
#include "ConfigLoaderSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/EngineConfigSingletonComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	std::string ReadJSONFileToString(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open()) {
			PG_CORE_ASSERT(false, "Could not open file");
			return "";
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		file.close();
		return ss.str();
	}
}

pig::SystemAccessDecl pig::ConfigLoaderSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(pig::EngineConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::EngineConfigSingletonComponent)),
	};
	return decl;
}

void pig::ConfigLoaderSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	if (accessor.view<const pig::EngineConfigSingletonComponent>().empty())
	{
		pig::EngineConfigSingletonComponent component;
		entt::entity ent = accessor.create();
		std::string configStr = ReadJSONFileToString("Assets/Engine/Config.json");
		json jsonObject = json::parse(configStr);

		PG_CORE_EXCEPT(jsonObject.contains("defaultQuadShader") && jsonObject["defaultQuadShader"].is_string(), "Missing defaultQuadShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultTextShader") && jsonObject["defaultTextShader"].is_string(), "Missing defaultTextShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultFont") && jsonObject["defaultFont"].is_string(), "Missing defaultFont in engine config");
		
		component.m_DefaultFontID = pig::UUID(jsonObject["defaultFont"].get<std::string>());
		component.m_DefaultQuadShaderID = pig::UUID(jsonObject["defaultQuadShader"].get<std::string>());
		component.m_DefaultTextShaderID = pig::UUID(jsonObject["defaultTextShader"].get<std::string>());
		accessor.emplace_deferred<pig::EngineConfigSingletonComponent>(ent, std::move(component));
	}
}
