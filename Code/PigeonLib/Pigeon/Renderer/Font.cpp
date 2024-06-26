#include "pch.h"

#include "Font.h"

#include "Pigeon/Renderer/MSDFData.h"
#include "Pigeon/Renderer/Renderer2D.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

static const double DEFAULT_ANGLE_THRESHOLD = 3.0;
static const long long LCG_MULTIPLIER = 6364136223846793005ull;
static const long long LCG_INCREMENT = 1442695040888963407ull;
static const int THREAD_COUNT = 8;

namespace
{
	struct CharsetRange
	{
		uint32_t Begin, End;
	};

	static const CharsetRange s_CharsetRanges[] =
	{
		{ 0x0020, 0x00FF }
	};
}

template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
pig::UUID CreateAndCacheAtlas(float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
	const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
{
	msdf_atlas::GeneratorAttributes attributes;
	attributes.config.overlapSupport = true;
	attributes.scanlinePass = true;

	msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
	generator.setAttributes(attributes);
	generator.setThreadCount(8);
	generator.generate(glyphs.data(), (int)glyphs.size());

	msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();
	const unsigned char* data = bitmap.pixels;
	return pig::Renderer2D::AddTexture(bitmap.width, bitmap.height, 3, data, pig::EMappedTextureType::eText);
}

pig::Font::Font(const std::filesystem::path& filepath)
	: m_Data(new pig::MSDFData())
{
	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
	PG_CORE_ASSERT(ft, "Failed to initialize freetype");

	msdfgen::FontHandle* font = msdfgen::loadFont(ft, filepath.string().c_str());
	if (!font)
	{
		PG_CORE_ERROR("Failed to load font: {}", filepath.string().c_str());
		return;
	}

	msdf_atlas::Charset charset;
	for (CharsetRange range : s_CharsetRanges)
	{
		for (uint32_t c = range.Begin; c <= range.End; c++)
			charset.add(c);
	}

	const double fontScale = 1.0;
	m_Data->FontGeometry = msdf_atlas::FontGeometry(&m_Data->Glyphs);
	const int glyphsLoaded = m_Data->FontGeometry.loadCharset(font, fontScale, charset);
	PG_CORE_INFO("Loaded {} glyphs from font (out of {})", glyphsLoaded, charset.size());

	double emSize = 40.0;

	msdf_atlas::TightAtlasPacker atlasPacker;
	// atlasPacker.setDimensionsConstraint()
	atlasPacker.setPixelRange(2.0);
	atlasPacker.setMiterLimit(1.0);
	atlasPacker.setPadding(0);
	atlasPacker.setScale(emSize);
	int remaining = atlasPacker.pack(m_Data->Glyphs.data(), (int)m_Data->Glyphs.size());
	PG_CORE_ASSERT(remaining == 0, "Failed to pak atlas data");

	int width, height;
	atlasPacker.getDimensions(width, height);
	emSize = atlasPacker.getScale();

	// if MSDF || MTSDF

	const uint64_t coloringSeed = 0;
	bool expensiveColoring = false;
	if (expensiveColoring)
	{
		msdf_atlas::Workload([&glyphs = m_Data->Glyphs, &coloringSeed](int i, int threadNo) -> bool 
			{
			unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
			glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			return true;
			}, m_Data->Glyphs.size()).finish(THREAD_COUNT);
	}
	else 
	{
		unsigned long long glyphSeed = coloringSeed;
		for (msdf_atlas::GlyphGeometry& glyph : m_Data->Glyphs)
		{
			glyphSeed *= LCG_MULTIPLIER;
			glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
		}
	}
	
	m_FontID = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>((float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

	//Uncomment to test single character
	/*
	msdfgen::Shape shape;
	if (msdfgen::loadGlyph(shape, font, 'C'))
	{
		shape.normalize();
		//                      max. angle
		msdfgen::edgeColoringSimple(shape, 3.0);
		//           image width, height
		msdfgen::Bitmap<float, 3> msdf(32, 32);
		//                     range, scale, translation
		msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
		msdfgen::savePng(msdf, "output.png");
	}
	*/

	msdfgen::destroyFont(font);
	msdfgen::deinitializeFreetype(ft);
}

pig::Font::~Font()
{
	delete m_Data;
}

glm::dvec2 pig::Font::GetCharacterAdvance(char c1, char c2, float kerning, float linespacing) const
{
	glm::dvec2 advance{ 0.0,0.0 };
	const auto& fontGeometry = m_Data->FontGeometry;
	const auto& metrics = fontGeometry.getMetrics();
	double fsScale = GetFsScale();
	if (c1 == '\n')
	{
		advance.y += fsScale * metrics.lineHeight + linespacing;
	}
	else if (c1 == '\t')
	{
		advance.x += 4.0f * (fsScale * fontGeometry.getGlyph(' ')->getAdvance() + kerning);
	}
	else if (auto glyph = fontGeometry.getGlyph(c1))
	{
		double glyphAdvance = glyph->getAdvance();
		fontGeometry.getAdvance(glyphAdvance, c1, c2);
		advance.x += fsScale * glyphAdvance + kerning;
	}

	return advance;
}

bool pig::Font::IsCharacterDrawable(char c) const
{
	return
		c != '\n' &&
		c != '\r' &&
		c != ' ' &&
		c != '\t' &&
		m_Data->FontGeometry.getGlyph(c);
}

bool pig::Font::IsCharacterNewLine(char c) const
{
	return c == '\n';
}

const msdf_atlas::GlyphGeometry* pig::Font::GetGlyph(char c) const
{
	const msdf_atlas::GlyphGeometry* glyph = m_Data->FontGeometry.getGlyph(c);
	if (!glyph)
		glyph = m_Data->FontGeometry.getGlyph('?');
	if (!glyph)
		return nullptr;
	return glyph;
}

glm::vec4 pig::Font::GetCharacterVertexQuad(char c, const glm::dvec2& offset) const
{
	glm::vec4 quad{};
	if (const msdf_atlas::GlyphGeometry* glyph = GetGlyph(c))
	{
		double fsScale = GetFsScale();
		double pl, pb, pr, pt;
		glyph->getQuadPlaneBounds(pl, pb, pr, pt);
		glm::vec2 quadMin((float)pl, 1.f - (float)pt);
		glm::vec2 quadMax((float)pr, 1.f - (float)pb);

		quadMin *= fsScale;
		quadMax *= fsScale;
		quadMin += offset;
		quadMax += offset;
		quad = glm::vec4( quadMin , quadMax );
	}
	return quad;
}

glm::vec4 pig::Font::GetCharacterTexCoordsQuad(char c, const pig::Texture2D& fontAtlas) const
{
	glm::vec4 texCoords{};
	if (const msdf_atlas::GlyphGeometry* glyph = GetGlyph(c))
	{
		double al, ab, ar, at;
		glyph->getQuadAtlasBounds(al, ab, ar, at);
		glm::vec2 texCoordMin((float)al, (float)at);
		glm::vec2 texCoordMax((float)ar, (float)ab);

		float texelWidth = 1.0f / fontAtlas.GetWidth();
		float texelHeight = 1.0f / fontAtlas.GetHeight();
		texCoordMin *= glm::vec2(texelWidth, texelHeight);
		texCoordMax *= glm::vec2(texelWidth, texelHeight);

		texCoords = glm::vec4(texCoordMin, texCoordMax);
	}
	return texCoords;
}

glm::mat4 pig::Font::GetCharacterTransform(const glm::vec4& charQuad, const glm::mat4& stringTransform) const
{
	glm::mat4 transform = glm::mat4(1.0f); // Identity matrix
	transform = glm::translate(transform, glm::vec3(charQuad.x, charQuad.y, 0.0f)); // Apply translation
	transform = glm::scale(transform, glm::vec3(charQuad.z - charQuad.x, charQuad.w - charQuad.y, 1.0f)); // Apply scaling
	transform = stringTransform * transform;
	return transform;
}

glm::vec2 pig::Font::GetStringBounds(std::string string, float kerning, float linespacing) const
{
	double maxWidth = 0.0;
	glm::dvec2 charOffset{ 0.0, 0.0 };

	const glm::vec3 originSprite(0.f, 0.0f, 0.f);

	for (size_t i = 0; i < string.size(); i++)
	{
		char character = string[i];

		if (IsCharacterNewLine(character))
		{
			charOffset.x = 0.0;
		}
		if (i < string.size() - 1)
		{
			glm::dvec2 charAdvance = GetCharacterAdvance(character, string[i + 1], kerning, linespacing);
			charOffset += charAdvance;
		}

		if (charOffset.x > maxWidth)
			maxWidth = charOffset.x;
	}

	return glm::vec2(maxWidth, charOffset.y);
}

double pig::Font::GetFsScale() const
{
	const auto& metrics = m_Data->FontGeometry.getMetrics();
	return 1.0 / (metrics.ascenderY - metrics.descenderY);
}

