#include "pch.h"
#include "ResourceManagerSystem.h"

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	struct ParsedResource
	{
		pig::UUID m_UUID;
		std::string m_Path;
	};

	json ReadJSONFileToString(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open()) {
			PG_CORE_ASSERT(false, "Could not open file");
			return "";
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		file.close();
		return json::parse(ss.str());
	}
	ParsedResource GetParsedResourceFromJsonItem(const json& jsonObject, const std::string& projectPrefix, const std::string& pathPrefix)
	{
		ParsedResource result;
		PG_CORE_EXCEPT(jsonObject.contains("id") && jsonObject["id"].is_string(), "could not parse id from resource manifest");
		PG_CORE_EXCEPT(jsonObject.contains("path") && jsonObject["path"].is_string(), "could not parse path from resource manifest");
		result.m_Path = "Assets/" + projectPrefix + "/" + pathPrefix + "/" + jsonObject["path"].get<std::string>();
		result.m_UUID = pig::UUID(jsonObject["id"].get<std::string>());
		return result;
	}

	void LoadFontFromResource(pig::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		pig::TextureData textureData;
		component.m_FontMap[resource.m_UUID] = std::move(std::make_shared<pig::Font>(resource.m_Path, textureData));
		component.m_TextureMap[textureData.m_TextureID] = pig::MappedTexture{ textureData.m_Texture, textureData.m_TextureType};
	}

	void LoadTextureFromResource(pig::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_TextureMap[resource.m_UUID] = pig::MappedTexture{ std::move(pig::Texture2D::Create(resource.m_Path)), pig::EMappedTextureType::eQuad };
	}

	void LoadUIFromResource(pig::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_UILayoutMap[resource.m_UUID] = resource.m_Path;
	}

	void LoadShaderFromResource(pig::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_ShaderMap[resource.m_UUID] = std::move(pig::Shader::Create(resource.m_Path));
	}

	void LoadResourcesFromPath(pig::ResourceMapSingletonComponent& component, const std::string& filePath, const std::string& prefix)
	{
		json jsonObject = ReadJSONFileToString(filePath);

		if (jsonObject.contains("ui"))
		{
			PG_CORE_EXCEPT(jsonObject["ui"].is_array(), "could not parse ui from resource manifest");
			for (json child : jsonObject["ui"])
			{
				LoadUIFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "UI"));
			}
		}
		if (jsonObject.contains("textures"))
		{
			PG_CORE_EXCEPT(jsonObject["textures"].is_array(), "could not parse textures from resource manifest");
			for (json child : jsonObject["textures"])
			{
				LoadTextureFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Textures"));
			}
		}
		if (jsonObject.contains("fonts"))
		{
			PG_CORE_EXCEPT(jsonObject["fonts"].is_array(), "could not parse fonts from resource manifest");
			for (json child : jsonObject["fonts"])
			{
				LoadFontFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Fonts"));
			}
		}
		if (jsonObject.contains("shaders"))
		{
			PG_CORE_EXCEPT(jsonObject["shaders"].is_array(), "could not parse shaders from resource manifest");
			for (json child : jsonObject["shaders"])
			{
				LoadShaderFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Shaders"));
			}
		}
	}
}

pig::SystemAccessDecl pig::ResourceManagerSystem::DeclareAccess() const
{
	pig::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(pig::ResourceMapSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pig::ResourceMapSingletonComponent)),
	};
	return decl;
}

void pig::ResourceManagerSystem::Update(const pig::Timestep& ts)
{
	auto accessor = pig::World::GetRegistry();

	if (accessor.view<const pig::ResourceMapSingletonComponent>().empty())
	{
		pig::ResourceMapSingletonComponent component;
		entt::entity ent = accessor.create();

		std::vector<unsigned char> data(2 * 2 * 4, 255);
		pig::MappedTexture mappedTexture = { std::move(pig::Texture2D::Create(2, 2, 4, data.data())), pig::EMappedTextureType::eQuad };
		component.m_TextureMap[component.m_DefaultTexture] = std::move(mappedTexture);

		LoadResourcesFromPath(component, "Assets/Engine/ResourcesManifest.json", "Engine");
#ifdef TESTS_ENABLED
		LoadResourcesFromPath(component, "Assets/UT/ResourcesManifest.json", "UT");
#else
		LoadResourcesFromPath(component, "Assets/App/ResourcesManifest.json", "App");
#endif
		accessor.emplace_deferred<pig::ResourceMapSingletonComponent>(ent, std::move(component));
	}
}
