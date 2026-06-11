#include "pch.h"
#include "Pigeon/Renderer/SpriteSheet.h"

#include <algorithm>

pg::SpriteSheet::SpriteSheet(int columns, int rows)
	: m_Columns(std::max(1, columns))
	, m_Rows(std::max(1, rows))
{
}

glm::vec4 pg::SpriteSheet::GetFrameTexCoords(int column, int row) const
{
	const int clampedColumn = std::min(std::max(column, 0), m_Columns - 1);
	const int clampedRow = std::min(std::max(row, 0), m_Rows - 1);

	const float u0 = static_cast<float>(clampedColumn) / static_cast<float>(m_Columns);
	const float v0 = static_cast<float>(clampedRow) / static_cast<float>(m_Rows);
	const float u1 = static_cast<float>(clampedColumn + 1) / static_cast<float>(m_Columns);
	const float v1 = static_cast<float>(clampedRow + 1) / static_cast<float>(m_Rows);

	return glm::vec4(u0, v0, u1, v1);
}
