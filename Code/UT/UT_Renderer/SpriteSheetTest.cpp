#include <catch2/catch.hpp>

#include "Pigeon/Renderer/SpriteSheet.h"

#include <glm/glm.hpp>

namespace CatchTestsetFail
{
	// ---------------------------------------------------------------------------
	// A default 1x1 sheet maps its single cell to the whole texture.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::DefaultSheetIsWholeTexture")
	{
		pg::SpriteSheet sheet;
		CHECK(sheet.GetColumns() == 1);
		CHECK(sheet.GetRows() == 1);
		CHECK(sheet.GetFrameTexCoords(0, 0) == glm::vec4(0.f, 0.f, 1.f, 1.f));
	}

	// ---------------------------------------------------------------------------
	// The top-left cell of an 8x8 sheet occupies the first eighth of each axis.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::TopLeftCellOfGrid")
	{
		pg::SpriteSheet sheet(8, 8);
		CHECK(sheet.GetFrameTexCoords(0, 0) == glm::vec4(0.f, 0.f, 1.f / 8.f, 1.f / 8.f));
	}

	// ---------------------------------------------------------------------------
	// An interior cell maps to its column/row fractions; row 0 is the top, so a
	// larger row index moves the V range downward.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::InteriorCellTexCoords")
	{
		pg::SpriteSheet sheet(8, 8);
		CHECK(sheet.GetFrameTexCoords(3, 2) == glm::vec4(3.f / 8.f, 2.f / 8.f, 4.f / 8.f, 3.f / 8.f));
	}

	// ---------------------------------------------------------------------------
	// A non-square grid divides each axis by its own count independently.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::NonSquareGrid")
	{
		pg::SpriteSheet sheet(9, 5);
		CHECK(sheet.GetFrameTexCoords(8, 4) == glm::vec4(8.f / 9.f, 4.f / 5.f, 1.f, 1.f));
	}

	// ---------------------------------------------------------------------------
	// Out-of-range cells clamp to the nearest edge cell rather than sampling
	// outside the texture.
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::OutOfRangeClampsToEdge")
	{
		pg::SpriteSheet sheet(4, 4);
		CHECK(sheet.GetFrameTexCoords(99, 99) == sheet.GetFrameTexCoords(3, 3));
		CHECK(sheet.GetFrameTexCoords(-5, -5) == sheet.GetFrameTexCoords(0, 0));
	}

	// ---------------------------------------------------------------------------
	// Degenerate dimensions are clamped to at least 1x1 (no divide-by-zero).
	// ---------------------------------------------------------------------------
	TEST_CASE("Renderer.SpriteSheet::DegenerateDimensionsClampToOne")
	{
		pg::SpriteSheet sheet(0, -3);
		CHECK(sheet.GetColumns() == 1);
		CHECK(sheet.GetRows() == 1);
		CHECK(sheet.GetFrameTexCoords(0, 0) == glm::vec4(0.f, 0.f, 1.f, 1.f));
	}
} // namespace CatchTestsetFail
