#include "pch.h"
#include "Platform/Testing/TestingTextGenBackend.h"

bool pg::TestingTextGenBackend::LoadModel(const std::string& modelPath, int gpuLayers)
{
	m_ModelPath = modelPath;
	m_GpuLayers = gpuLayers;
	m_Loaded = !modelPath.empty();
	return m_Loaded;
}

std::string pg::TestingTextGenBackend::Generate(const pg::TextGenParams& params)
{
	m_LastParams = params;
	++m_GenerateCount;

	if (!m_Loaded)
	{
		return std::string();
	}

	// Echo the prompt so tests can assert the request flowed through to the backend and back.
	return "[mock] " + params.m_Prompt;
}
