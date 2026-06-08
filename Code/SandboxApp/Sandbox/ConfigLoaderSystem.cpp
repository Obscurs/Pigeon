#include "Sandbox/ConfigLoaderSystem.h"

#include "Pigeon/ECS/World.h"
#include "Sandbox/SandboxConfigSingletonComponent.h"

namespace
{
	std::string ReadJSONFileToString(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			PG_CORE_ASSERT(false, "Could not open file");
			return "";
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		file.close();
		return ss.str();
	}

	pg::UUID ParseRequiredUUID(const json& jsonObject, const char* key)
	{
		PG_CORE_EXCEPT(jsonObject.contains(key) && jsonObject[key].is_string(), "Missing string field in app config");
		return pg::UUID(jsonObject[key].get<std::string>());
	}
}

pg::SystemAccessDecl sbx::ConfigLoaderSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(sbx::SandboxConfigSingletonComponent)),
	};
	return decl;
}

void sbx::ConfigLoaderSystem::Update(const pg::Timestep& ts)
{
	pg::CheckedRegistryAccessor accessor = pg::World::GetRegistry();

	if (!accessor.View<const sbx::SandboxConfigSingletonComponent>().empty())
	{
		return;
	}

	const std::string configStr = ReadJSONFileToString("Assets/App/Config.json");
	const json jsonObject = json::parse(configStr);

	sbx::SandboxConfigSingletonComponent component;
	component.m_DefaultFontID = ParseRequiredUUID(jsonObject, "defaultFont");
	component.m_BoldFontID = ParseRequiredUUID(jsonObject, "boldFont");
	component.m_SpriteTextureID = ParseRequiredUUID(jsonObject, "spriteTexture");
	component.m_TexturedQuadTextureID = ParseRequiredUUID(jsonObject, "texturedQuadTexture");
	component.m_MainLayoutID = ParseRequiredUUID(jsonObject, "mainLayoutId");
	component.m_ToggleButtonID = ParseRequiredUUID(jsonObject, "toggleButton");
	component.m_TogglePanelID = ParseRequiredUUID(jsonObject, "togglePanel");
	component.m_StatusTextID = ParseRequiredUUID(jsonObject, "statusText");
	component.m_CloseButtonID = ParseRequiredUUID(jsonObject, "closeButton");
	component.m_CloseTargetID = ParseRequiredUUID(jsonObject, "closeTarget");
	component.m_ButtonImageID = ParseRequiredUUID(jsonObject, "buttonImage");
	component.m_ButtonHoverImageID = ParseRequiredUUID(jsonObject, "buttonHoverImage");
	component.m_ButtonPressedImageID = ParseRequiredUUID(jsonObject, "buttonPressedImage");
	component.m_SampleSoundID = ParseRequiredUUID(jsonObject, "sampleSound");
	component.m_SampleMusicID = ParseRequiredUUID(jsonObject, "sampleMusic");

	pg::ecs::Entity ent = accessor.Create();
	accessor.EmplaceDeferred<sbx::SandboxConfigSingletonComponent>(ent, std::move(component));
}
