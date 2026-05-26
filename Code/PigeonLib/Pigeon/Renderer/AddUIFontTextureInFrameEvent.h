#pragma once
#include <string>

#include "Pigeon/Renderer/Texture.h"

namespace pig
{
	struct AddUIFontTextureInFrameEvent
	{
		AddUIFontTextureInFrameEvent() {};
		AddUIFontTextureInFrameEvent(const AddUIFontTextureInFrameEvent&) = default;

		pig::TextureData m_TextureData;
	};
}