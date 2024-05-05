#pragma once

#include <filesystem>

#include "Pigeon/Core/Core.h"
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
		const std::string& GetFontID() const { return m_FontID; };

		static pig::S_Ptr<Font> GetDefault();
	private:
		MSDFData* m_Data;
		std::string m_FontID;
	};
}