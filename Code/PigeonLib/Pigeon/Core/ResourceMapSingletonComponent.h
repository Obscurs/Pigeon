#pragma once
#include <nlohmann/json.hpp>

#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Model.h"
#include "Pigeon/Renderer/RenderTarget.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

namespace pg
{
	struct MappedTexture
	{
		pg::S_Ptr<pg::Texture2D> m_Texture;
		EMappedTextureType m_TextureType;
	};

	struct ResourceMapSingletonComponent
	{
		ResourceMapSingletonComponent() {};
		ResourceMapSingletonComponent(const ResourceMapSingletonComponent&) = default;

		std::unordered_map<pg::UUID, MappedTexture> m_TextureMap;
		std::unordered_map<pg::UUID, pg::S_Ptr<pg::Shader>> m_ShaderMap;
		std::unordered_map<pg::UUID, pg::S_Ptr<pg::Font>> m_FontMap;
		std::unordered_map<pg::UUID, std::string> m_UILayoutMap;
		std::unordered_map<pg::UUID, pg::S_Ptr<pg::SoundClip>> m_SoundMap;
		std::unordered_map<pg::UUID, nlohmann::json> m_JSONMap;
		std::unordered_map<pg::UUID, pg::S_Ptr<pg::Model>> m_ModelMap;

		// Offscreen render targets keyed by UUID. Each target's colour buffer is also registered in
		// m_TextureMap under the same UUID, so 2D draws can sample the rendered image as a texture.
		std::unordered_map<pg::UUID, pg::S_Ptr<pg::RenderTarget>> m_RenderTargetMap;

		pg::UUID m_DefaultTexture = pg::UUID::Generate();
	};
}