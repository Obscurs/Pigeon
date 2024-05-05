#include "pch.h"

#include "Font.h"

#undef INFINITE
#include "msdf-atlas-gen.h"
#include "msdfgen.h"

void pig::Font(/*const std::filesystem::path& filepath*/)
{
	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
	if (ft) 
	{
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, "C:\\Windows\\Fonts\\arialbd.ttf");
		if (font) 
		{
			msdfgen::Shape shape;
			if (msdfgen::loadGlyph(shape, font, 'A')) 
			{
				shape.normalize();
				// max. angle
				msdfgen::edgeColoringSimple(shape, 3.0);
				// output width, height
				msdfgen::Bitmap<float, 3> msdf(32, 32);
				// scale, translation
				msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
				msdfgen::savePng(msdf, "output.png");
			}
			msdfgen::destroyFont(font);
		}
		msdfgen::deinitializeFreetype(ft);
	}
}