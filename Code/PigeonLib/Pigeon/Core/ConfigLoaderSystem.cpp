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

	if (accessor.view<const pg::EngineConfigSingletonComponent>().empty())
	{
		pg::EngineConfigSingletonComponent component;
		entt::entity ent = accessor.create();
		std::string configStr = ReadJSONFileToString("Assets/Engine/Config.json");
		json jsonObject = json::parse(configStr);

		PG_CORE_EXCEPT(jsonObject.contains("defaultQuadShader") && jsonObject["defaultQuadShader"].is_string(), "Missing defaultQuadShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultTextShader") && jsonObject["defaultTextShader"].is_string(), "Missing defaultTextShader in engine config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultFont") && jsonObject["defaultFont"].is_string(), "Missing defaultFont in engine config");
		
		component.m_DefaultFontID = pg::UUID(jsonObject["defaultFont"].get<std::string>());
		component.m_DefaultQuadShaderID = pg::UUID(jsonObject["defaultQuadShader"].get<std::string>());
		component.m_DefaultTextShaderID = pg::UUID(jsonObject["defaultTextShader"].get<std::string>());
		accessor.emplace_deferred<pg::EngineConfigSingletonComponent>(ent, std::move(component));
	}
}
