#include "pch.h"
#include "Pigeon/Diffusion/Image.h"

#include "vendor/stb_image/stb_image.h"

pg::Image pg::LoadImageFromFile(const std::string& path)
{
	pg::Image image;
	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 3); // force RGB
	if (data == nullptr)
	{
		PG_CORE_WARN("Could not load input image: {0}", path);
		return image;
	}

	image.m_Width = static_cast<uint32_t>(width);
	image.m_Height = static_cast<uint32_t>(height);
	image.m_Pixels.assign(data, data + (static_cast<size_t>(width) * height * 3));
	stbi_image_free(data);
	return image;
}
