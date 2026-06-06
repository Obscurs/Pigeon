#pragma once

#include <vector>

#undef INFINITE
#include "msdf-atlas-gen.h"

namespace pg 
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> m_Glyphs;
		msdf_atlas::FontGeometry m_FontGeometry;
	};
}
