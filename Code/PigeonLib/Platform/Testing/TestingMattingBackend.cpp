#include "pch.h"
#include "Platform/Testing/TestingMattingBackend.h"

bool pg::TestingMattingBackend::LoadModel(const std::string& modelPath)
{
	m_ModelPath = modelPath;
	m_Loaded = !modelPath.empty();
	return m_Loaded;
}

pg::Image pg::TestingMattingBackend::Matte(const pg::Image& input)
{
	++m_MatteCount;

	if (!m_Loaded || input.m_Pixels.empty())
	{
		return pg::Image{};
	}

	// A deterministic split matte: left half foreground (white), right half background (black). Replicated
	// across RGB so it feeds the red-channel mask-composite alpha directly.
	pg::Image matte;
	matte.m_Width = input.m_Width;
	matte.m_Height = input.m_Height;
	matte.m_Pixels.resize(static_cast<size_t>(input.m_Width) * input.m_Height * 3);
	const uint32_t half = input.m_Width / 2;
	for (uint32_t y = 0; y < input.m_Height; ++y)
	{
		for (uint32_t x = 0; x < input.m_Width; ++x)
		{
			const uint8_t value = (x < half) ? 255 : 0;
			const size_t index = (static_cast<size_t>(y) * input.m_Width + x) * 3;
			matte.m_Pixels[index] = value;
			matte.m_Pixels[index + 1] = value;
			matte.m_Pixels[index + 2] = value;
		}
	}
	return matte;
}
