#pragma once

#include <filesystem>

#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/UUID.h"
#include "Pigeon/Renderer/Texture.h"

namespace msdf_atlas
{
	class GlyphGeometry;
}

namespace pg 
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& font, pg::TextureData& textureData);
		~Font();

		const MSDFData* GetMSDFData() const { return m_Data; }
		const pg::UUID& GetFontID() const { return m_FontID; };

		glm::dvec2 GetCharacterAdvance(char c1, char c2, float kerning, float linespacing) const;
		bool IsCharacterDrawable(char c) const;
		bool IsCharacterNewLine(char c) const;

		const msdf_atlas::GlyphGeometry* GetGlyph(char c) const;

		glm::vec4 GetCharacterVertexQuad(char c, const glm::dvec2& offset) const;
		glm::vec4 GetCharacterTexCoordsQuad(char c, const pg::Texture2D& fontAtlas) const;

		glm::mat4 GetCharacterTransform(const glm::vec4& charQuad, const glm::mat4& stringTransform) const;
		glm::vec2 GetStringBounds(std::string string, float kerning, float linespacing) const;

		// Greedy word-wrap for fixed-size body text. Returns a copy of the string with select spaces
		// converted to newlines so each line fits within maxWidth (logical-canvas units) when rendered at
		// fixedFontSize. The result has the same length as the input (only spaces become '\n'), so a
		// reveal index into the source maps unchanged onto the wrapped output. Existing newlines are kept;
		// a single word wider than maxWidth overflows rather than breaking mid-word. maxWidth <= 0 (or a
		// non-positive font size) returns the input unchanged.
		std::string WrapString(const std::string& string, float fixedFontSize, float kerning, float linespacing, float maxWidth) const;

	private:
		double GetFsScale() const;
		MSDFData* m_Data;
		pg::UUID m_FontID;
	};
}