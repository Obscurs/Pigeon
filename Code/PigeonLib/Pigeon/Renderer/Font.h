#pragma once

#include <filesystem>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Texture.h"

namespace pig 
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& font);
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data; }
		const pig::UUID& GetFontID() const { return m_FontID; };
	private:
		MSDFData* m_Data;
		pig::UUID m_FontID;
	};
}