#include "pch.h"
#include "Pigeon/Core/ResourceManagerSystem.h"

#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/ECS/World.h"

namespace
{
	struct ParsedResource
	{
		pg::UUID m_UUID;
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
		result.m_UUID = pg::UUID(jsonObject["id"].get<std::string>());
		return result;
	}

	void LoadFontFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		pg::TextureData textureData;
		component.m_FontMap[resource.m_UUID] = std::move(std::make_shared<pg::Font>(resource.m_Path, textureData));
		component.m_TextureMap[textureData.m_TextureID] = pg::MappedTexture{ textureData.m_Texture, textureData.m_TextureType};
	}

	void LoadTextureFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_TextureMap[resource.m_UUID] = pg::MappedTexture{ std::move(pg::Texture2D::Create(resource.m_Path)), pg::EMappedTextureType::eQuad };
	}

	void LoadUIFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_UILayoutMap[resource.m_UUID] = resource.m_Path;
	}

	void LoadShaderFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_ShaderMap[resource.m_UUID] = std::move(pg::Shader::Create(resource.m_Path));
	}

	void LoadSoundFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_SoundMap[resource.m_UUID] = pg::SoundClip::Create(resource.m_Path);
	}

	void LoadJSONFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_JSONMap[resource.m_UUID] = ReadJSONFileToString(resource.m_Path);
	}

	void LoadModelFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_ModelMap[resource.m_UUID] = pg::Model::Create(resource.m_Path);
	}

	// Render targets are pathless: an {id, width, height} entry. The created target is stored in the
	// render-target map and its colour buffer is registered in the texture map under the same UUID, so
	// the 2D pass can sample the rendered image like any other texture.
	void LoadRenderTargetFromResource(pg::ResourceMapSingletonComponent& component, const json& jsonObject)
	{
		PG_CORE_EXCEPT(jsonObject.contains("id") && jsonObject["id"].is_string(), "could not parse id from render target manifest entry");
		PG_CORE_EXCEPT(jsonObject.contains("width") && jsonObject["width"].is_number_unsigned(), "could not parse width from render target manifest entry");
		PG_CORE_EXCEPT(jsonObject.contains("height") && jsonObject["height"].is_number_unsigned(), "could not parse height from render target manifest entry");

		const pg::UUID uuid(jsonObject["id"].get<std::string>());
		const unsigned int width = jsonObject["width"].get<unsigned int>();
		const unsigned int height = jsonObject["height"].get<unsigned int>();

		pg::S_Ptr<pg::RenderTarget> renderTarget = pg::RenderTarget::Create(width, height);
		component.m_RenderTargetMap[uuid] = renderTarget;
		component.m_TextureMap[uuid] = pg::MappedTexture{ renderTarget->GetColorTexture(), pg::EMappedTextureType::eQuad };
	}

	void LoadResourcesFromPath(pg::ResourceMapSingletonComponent& component, const std::string& filePath, const std::string& prefix)
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
		if (jsonObject.contains("sounds"))
		{
			PG_CORE_EXCEPT(jsonObject["sounds"].is_array(), "could not parse sounds from resource manifest");
			for (json child : jsonObject["sounds"])
			{
				LoadSoundFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Audio"));
			}
		}
		if (jsonObject.contains("json"))
		{
			PG_CORE_EXCEPT(jsonObject["json"].is_array(), "could not parse json from resource manifest");
			for (json child : jsonObject["json"])
			{
				LoadJSONFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "JSON"));
			}
		}
		if (jsonObject.contains("models"))
		{
			PG_CORE_EXCEPT(jsonObject["models"].is_array(), "could not parse models from resource manifest");
			for (json child : jsonObject["models"])
			{
				LoadModelFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Models"));
			}
		}
		if (jsonObject.contains("renderTargets"))
		{
			PG_CORE_EXCEPT(jsonObject["renderTargets"].is_array(), "could not parse renderTargets from resource manifest");
			for (json child : jsonObject["renderTargets"])
			{
				LoadRenderTargetFromResource(component, child);
			}
		}
	}
}

pg::SystemAccessDecl pg::ResourceManagerSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
	};
	decl.addSet = {
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
	};
	return decl;
}

void pg::ResourceManagerSystem::Update(const pg::Timestep& ts)
{
	auto accessor = pg::World::GetRegistry();

	if (accessor.View<const pg::ResourceMapSingletonComponent>().empty())
	{
		pg::ResourceMapSingletonComponent component;
		pg::ecs::Entity ent = accessor.Create();

		std::vector<unsigned char> data(2 * 2 * 4, 255);
		pg::MappedTexture mappedTexture = { std::move(pg::Texture2D::Create(2, 2, 4, data.data())), pg::EMappedTextureType::eQuad };
		component.m_TextureMap[component.m_DefaultTexture] = std::move(mappedTexture);

		LoadResourcesFromPath(component, "Assets/Engine/ResourcesManifest.json", "Engine");
#ifdef TESTS_ENABLED
		LoadResourcesFromPath(component, "Assets/UT/ResourcesManifest.json", "UT");
#else
		LoadResourcesFromPath(component, "Assets/App/ResourcesManifest.json", "App");
#endif
		accessor.EmplaceDeferred<pg::ResourceMapSingletonComponent>(ent, std::move(component));
	}
}
