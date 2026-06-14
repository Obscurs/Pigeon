#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace pg
{
	// A CPU RGB8 image: row-major, 3 bytes per pixel (size = width * height * 3). The common pixel
	// container across the diffusion subsystem — ControlNet hints, img2img init images, and generated
	// results all use it.
	struct Image
	{
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		std::vector<uint8_t> m_Pixels;
	};

	// Decodes an image file (PNG/JPG/…) into an RGB8 Image. Returns an empty Image (zero size) if the
	// file cannot be opened or decoded.
	Image LoadImageFromFile(const std::string& path);
}
