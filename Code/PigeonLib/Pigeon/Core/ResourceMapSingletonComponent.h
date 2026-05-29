#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/Renderer/Texture.h"

namespace pig
{
	struct MappedTexture
	{
		pig::S_Ptr<pig::Texture2D> m_Texture;
		EMappedTextureType m_TextureType;
	};

	struct ResourceMapSingletonComponent
	{
		ResourceMapSingletonComponent() {};
		ResourceMapSingletonComponent(const ResourceMapSingletonComponent&) = default;

		std::unordered_map<pig::UUID, MappedTexture> m_TextureMap;
		std::unordered_map<pig::UUID, pig::S_Ptr<pig::Shader>> m_ShaderMap;
		std::unordered_map<pig::UUID, pig::S_Ptr<pig::Font>> m_FontMap;
		std::unordered_map<pig::UUID, std::string> m_UILayoutMap;

		pig::UUID m_DefaultTexture = pig::UUID::Generate();
	};
}