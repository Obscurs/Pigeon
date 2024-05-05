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
static pig::S_Ptr<pig::Font> DefaultFont;

template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
void CreateAndCacheAtlas(const std::string& fontName, float fontSize, const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
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
	pig::Renderer2D::AddTexture(bitmap.width, bitmap.height, 3, data, fontName);
}

pig::Font::Font(const std::filesystem::path& filepath)
	: m_Data(new pig::MSDFData())
{
	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
	PG_CORE_ASSERT(ft, "Failed to initialize freetype");

	m_FontID = filepath.string();

	msdfgen::FontHandle* font = msdfgen::loadFont(ft, m_FontID.c_str());
	if (!font)
	{
		PG_CORE_ERROR("Failed to load font: {}", m_FontID);
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
	
	CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(m_FontID, (float)emSize, m_Data->Glyphs, m_Data->FontGeometry, width, height);

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

pig::S_Ptr<pig::Font> pig::Font::GetDefault()
{
	if (!DefaultFont)
		DefaultFont = std::make_shared<pig::Font>("Assets/Fonts/opensans/OpenSans-Regular.ttf");
	return DefaultFont;
}