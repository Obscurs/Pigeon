#pragma once

#include <string>

#include "Pigeon/Diffusion/DiffusionBackend.h"

namespace pg
{
	// Deterministic no-op diffusion backend for tests: records the checkpoint/ControlNet it was asked
	// to load and the parameters of the most recent Generate call, and returns a solid mid-grey image
	// of the requested size (empty when no checkpoint is loaded). Lets tests verify the request →
	// generate → result-handling path without any real inference, CUDA, or model files.
	class TestingDiffusionBackend : public DiffusionBackend
	{
	public:
		bool LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath) override;
		bool IsLoaded() const override { return m_Loaded; }
		pg::Image Generate(const DiffusionJobParams& params) override;

		const std::string& GetCheckpointPath() const { return m_CheckpointPath; }
		const std::string& GetControlNetPath() const { return m_ControlNetPath; }
		const std::string& GetVaePath() const { return m_VaePath; }
		const DiffusionJobParams& GetLastParams() const { return m_LastParams; }
		int GetGenerateCount() const { return m_GenerateCount; }

	private:
		bool m_Loaded = false;
		std::string m_CheckpointPath;
		std::string m_ControlNetPath;
		std::string m_VaePath;
		DiffusionJobParams m_LastParams;
		int m_GenerateCount = 0;
	};
}
