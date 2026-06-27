#pragma once

#include <string>

#include "Pigeon/Diffusion/MattingBackend.h"

namespace pg
{
	// Deterministic no-op matting backend for tests: records the model path it was asked to load and the
	// number of Matte calls, and returns a fixed Alpha Matte — the LEFT half white (foreground) and the
	// RIGHT half black (background) — of the input size (empty when no model is loaded). The fixed split
	// lets a test prove the matte alpha (not the centred skeleton silhouette or a chroma key) drove the
	// composite, and exercises the skeleton fallback when the model is absent. No real inference, CUDA, or
	// model files.
	class TestingMattingBackend : public MattingBackend
	{
	public:
		bool LoadModel(const std::string& modelPath) override;
		bool IsLoaded() const override { return m_Loaded; }
		pg::Image Matte(const pg::Image& input) override;

		const std::string& GetModelPath() const { return m_ModelPath; }
		int GetMatteCount() const { return m_MatteCount; }

	private:
		bool m_Loaded = false;
		std::string m_ModelPath;
		int m_MatteCount = 0;
	};
}
