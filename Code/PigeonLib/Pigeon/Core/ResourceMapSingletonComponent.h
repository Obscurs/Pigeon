#pragma once
#include "Pigeon/Audio/SoundClip.h"
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Font.h"
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

		pg::UUID m_DefaultTexture = pg::UUID::Generate();
	};
}