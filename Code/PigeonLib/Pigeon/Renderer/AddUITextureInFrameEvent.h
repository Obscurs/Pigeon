#pragma once
#include <string>

#include "Pigeon/Renderer/Texture.h"

namespace pig
{
	struct AddUITextureInFrameEvent
	{
		AddUITextureInFrameEvent() {};
		AddUITextureInFrameEvent(const AddUITextureInFrameEvent&) = default;

		bool m_IsVirtual = false;
		std::string m_Path = "";
		pig::TextureData m_TextureData;
	};
}