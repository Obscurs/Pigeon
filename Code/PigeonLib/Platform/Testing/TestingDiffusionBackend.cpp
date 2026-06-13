#include "pch.h"
#include "Platform/Testing/TestingDiffusionBackend.h"

bool pg::TestingDiffusionBackend::LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath)
{
	m_CheckpointPath = checkpointPath;
	m_ControlNetPath = controlNetPath;
	m_VaePath = vaePath;
	m_Loaded = !checkpointPath.empty();
	return m_Loaded;
}

pg::Image pg::TestingDiffusionBackend::Generate(const pg::DiffusionJobParams& params)
{
	m_LastParams = params;
	++m_GenerateCount;

	if (!m_Loaded)
	{
		return pg::Image{};
	}

	pg::Image image;
	image.m_Width = params.m_Width;
	image.m_Height = params.m_Height;
	image.m_Pixels.assign(static_cast<size_t>(params.m_Width) * params.m_Height * 3, 128);
	return image;
}
