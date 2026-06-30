#include "pch.h"
#include "Pigeon/Core/ResourceManagerSystem.h"

#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/Application.h"
#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/ResourceMapSingletonComponent.h"
#include "Pigeon/Diffusion/RegisterGeneratedTextureRequestOneFrameComponent.h"
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

	// Checkpoints/LoRAs/ControlNets are loaded by the diffusion backend, not the engine, so the
	// resource map only records their resolved file paths.
	void LoadCheckpointFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_CheckpointMap[resource.m_UUID] = resource.m_Path;
	}

	void LoadLoraFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_LoraMap[resource.m_UUID] = resource.m_Path;
	}

	void LoadControlNetFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_ControlNetMap[resource.m_UUID] = resource.m_Path;
	}

	void LoadVaeFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_VaeMap[resource.m_UUID] = resource.m_Path;
	}

	// GGUF language models are loaded by the inference backend (llama.cpp), not the engine, so the
	// resource map only records their resolved file paths.
	void LoadLanguageModelFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_LanguageModelMap[resource.m_UUID] = resource.m_Path;
	}

	// ONNX matting models are loaded by the matting backend (onnxruntime), not the engine, so the
	// resource map only records their resolved file paths (ADR 0012).
	void LoadMattingModelFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_MattingModelMap[resource.m_UUID] = resource.m_Path;
	}

	// Input images for img2img are decoded to CPU RGB pixels (the diffusion backend consumes pixels).
	void LoadInputImageFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_InputImageMap[resource.m_UUID] = pg::LoadImageFromFile(resource.m_Path);
	}

	void LoadOpenPoseSkeletonFromResource(pg::ResourceMapSingletonComponent& component, const ParsedResource& resource)
	{
		component.m_OpenPoseSkeletonMap[resource.m_UUID] = pg::OpenPoseSkeleton::Create(resource.m_Path);
	}

	// Registers a generated image (RGB) as a Texture2D under its caller-assigned UUID. The pixels are
	// expanded RGB -> RGBA (opaque) because the texture backend expects four channels. A zero-sized or
	// truncated image is ignored. The RGB pixels are also retained as an Input Image under the same UUID
	// so a later generation can re-feed this result as an img2img init / chroma background (ADR 0011).
	void RegisterGeneratedTexture(pg::ResourceMapSingletonComponent& component, const pg::RegisterGeneratedTextureRequestOneFrameComponent& request)
	{
		const pg::Image& image = request.m_Image;
		const size_t pixelCount = static_cast<size_t>(image.m_Width) * image.m_Height;
		if (pixelCount == 0 || image.m_Pixels.size() < pixelCount * 3)
		{
			return;
		}

		std::vector<unsigned char> rgba(pixelCount * 4);
		for (size_t i = 0; i < pixelCount; ++i)
		{
			rgba[i * 4 + 0] = image.m_Pixels[i * 3 + 0];
			rgba[i * 4 + 1] = image.m_Pixels[i * 3 + 1];
			rgba[i * 4 + 2] = image.m_Pixels[i * 3 + 2];
			rgba[i * 4 + 3] = 255;
		}

		component.m_TextureMap[request.m_TextureID] = pg::MappedTexture{ pg::Texture2D::Create(image.m_Width, image.m_Height, 4, rgba.data()), pg::EMappedTextureType::eQuad };
		component.m_InputImageMap[request.m_TextureID] = image;
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
		if (jsonObject.contains("checkpoints"))
		{
			PG_CORE_EXCEPT(jsonObject["checkpoints"].is_array(), "could not parse checkpoints from resource manifest");
			for (json child : jsonObject["checkpoints"])
			{
				LoadCheckpointFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("loras"))
		{
			PG_CORE_EXCEPT(jsonObject["loras"].is_array(), "could not parse loras from resource manifest");
			for (json child : jsonObject["loras"])
			{
				LoadLoraFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("controlNets"))
		{
			PG_CORE_EXCEPT(jsonObject["controlNets"].is_array(), "could not parse controlNets from resource manifest");
			for (json child : jsonObject["controlNets"])
			{
				LoadControlNetFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("openPoseSkeletons"))
		{
			PG_CORE_EXCEPT(jsonObject["openPoseSkeletons"].is_array(), "could not parse openPoseSkeletons from resource manifest");
			for (json child : jsonObject["openPoseSkeletons"])
			{
				LoadOpenPoseSkeletonFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("vae"))
		{
			PG_CORE_EXCEPT(jsonObject["vae"].is_array(), "could not parse vae from resource manifest");
			for (json child : jsonObject["vae"])
			{
				LoadVaeFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("languageModels"))
		{
			PG_CORE_EXCEPT(jsonObject["languageModels"].is_array(), "could not parse languageModels from resource manifest");
			for (json child : jsonObject["languageModels"])
			{
				LoadLanguageModelFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "TextGeneration"));
			}
		}
		if (jsonObject.contains("mattingModels"))
		{
			PG_CORE_EXCEPT(jsonObject["mattingModels"].is_array(), "could not parse mattingModels from resource manifest");
			for (json child : jsonObject["mattingModels"])
			{
				LoadMattingModelFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "ImageGeneration"));
			}
		}
		if (jsonObject.contains("inputImages"))
		{
			PG_CORE_EXCEPT(jsonObject["inputImages"].is_array(), "could not parse inputImages from resource manifest");
			for (json child : jsonObject["inputImages"])
			{
				// img2img inputs are ordinary photos and live alongside the app's other textures.
				LoadInputImageFromResource(component, GetParsedResourceFromJsonItem(child, prefix, "Textures"));
			}
		}
	}
}

pg::SystemAccessDecl pg::ResourceManagerSystem::DeclareAccess() const
{
	pg::SystemAccessDecl decl;

	decl.readSet = {
		std::type_index(typeid(pg::ResourceMapSingletonComponent)),
		std::type_index(typeid(pg::RegisterGeneratedTextureRequestOneFrameComponent)),
	};
	decl.writeSet = {
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
		// The app's asset folder name is configured per application (defaults to "App"); SandboxApp sets
		// "Sandbox". This is the sole place the engine resolves the app's folder under Data/Assets.
		const std::string& appFolder = pg::Application::Get().GetAppAssetsFolder();
		LoadResourcesFromPath(component, "Assets/" + appFolder + "/ResourcesManifest.json", appFolder);
#endif
		accessor.EmplaceDeferred<pg::ResourceMapSingletonComponent>(ent, std::move(component));
		return;
	}

	// Steady state: register any images generated this frame into the (existing) texture map, so a
	// Generated Texture becomes an ordinary entry sampled by sprites/UI under its caller-assigned UUID.
	auto mapView = accessor.View<pg::ResourceMapSingletonComponent>();
	pg::ResourceMapSingletonComponent& map = mapView.get<pg::ResourceMapSingletonComponent>(mapView.front());
	auto registrationView = accessor.View<const pg::RegisterGeneratedTextureRequestOneFrameComponent>();
	for (pg::ecs::Entity entity : registrationView)
	{
		RegisterGeneratedTexture(map, registrationView.get<const pg::RegisterGeneratedTextureRequestOneFrameComponent>(entity));
	}
}
