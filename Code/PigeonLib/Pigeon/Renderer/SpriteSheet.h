#pragma once

#include <glm/glm.hpp>

namespace pg
{
	// A uniform grid that subdivides one texture into columns x rows equal cells. Maps a cell to its
	// normalised UV rectangle (x,y = min corner, z,w = max corner, each in [0,1]); the top-left cell is
	// column 0 / row 0, matching the renderer's UV convention (V increases downward). Pixel-agnostic:
	// cells are fractions of the texture, so the same sheet works at any texture resolution.
	class SpriteSheet
	{
	public:
		SpriteSheet() = default;
		SpriteSheet(int columns, int rows);
		~SpriteSheet() = default;

		// UV rectangle of the cell at (column, row). Inputs are clamped into the valid grid range so a
		// frame index past the edge never samples outside the texture.
		glm::vec4 GetFrameTexCoords(int column, int row) const;

		int GetColumns() const { return m_Columns; }
		int GetRows() const { return m_Rows; }

	private:
		int m_Columns = 1;
		int m_Rows = 1;
	};
}
