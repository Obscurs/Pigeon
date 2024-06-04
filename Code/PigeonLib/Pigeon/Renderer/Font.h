#pragma once

#include <filesystem>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Texture.h"

namespace msdf_atlas
{
	class GlyphGeometry;
}

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

		glm::dvec2 GetCharacterAdvance(char c1, char c2, float kerning, float linespacing) const;
		bool IsCharacterDrawable(char c) const;
		bool IsCharacterNewLine(char c) const;

		const msdf_atlas::GlyphGeometry* GetGlyph(char c) const;

		glm::vec4 GetCharacterVertexQuad(char c, const glm::dvec2& offset) const;
		glm::vec4 GetCharacterTexCoordsQuad(char c, const pig::Texture2D& fontAtlas) const;

		glm::mat4 GetCharacterTransform(const glm::vec4& charQuad, const glm::mat4& stringTransform) const;

	private:
		double GetFsScale() const;
		MSDFData* m_Data;
		pig::UUID m_FontID;
	};
}