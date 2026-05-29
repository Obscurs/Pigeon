#include "pch.h"
#include "ConfigLoaderSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/SampleUIConfigSingletonComponent.h"

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

pig::SystemAccessDecl sbx::ConfigLoaderSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(sbx::SampleUIConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::SampleUIConfigSingletonComponent)),
	};
	return decl;
}

void sbx::ConfigLoaderSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	if (accessor.view<const sbx::SampleUIConfigSingletonComponent>().empty())
	{
		sbx::SampleUIConfigSingletonComponent component;
		entt::entity ent = accessor.create();
		std::string configStr = ReadJSONFileToString("Assets/App/Config.json");
		json jsonObject = json::parse(configStr);

		PG_CORE_EXCEPT(jsonObject.contains("uuidui1") && jsonObject["uuidui1"].is_string(), "Missing uuidui1 in app config");
		PG_CORE_EXCEPT(jsonObject.contains("uuidui2") && jsonObject["uuidui2"].is_string(), "Missing uuidui2 in app config");
		PG_CORE_EXCEPT(jsonObject.contains("defaultFont") && jsonObject["defaultFont"].is_string(), "Missing defaultFont in app config");
		PG_CORE_EXCEPT(jsonObject.contains("mainLayoutId") && jsonObject["mainLayoutId"].is_string(), "Missing mainLayoutId in app config");

		component.m_DefaultFontID = pig::UUID(jsonObject["defaultFont"].get<std::string>());
		component.m_UUIDUI1 = pig::UUID(jsonObject["uuidui1"].get<std::string>());
		component.m_UUIDUI2 = pig::UUID(jsonObject["uuidui2"].get<std::string>());
		component.m_MainLayoutID = pig::UUID(jsonObject["mainLayoutId"].get<std::string>());
		accessor.emplace_deferred<sbx::SampleUIConfigSingletonComponent>(ent, std::move(component));
	}
}
