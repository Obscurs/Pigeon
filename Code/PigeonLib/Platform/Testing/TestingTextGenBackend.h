#pragma once

#include <string>

#include "Pigeon/TextGen/TextGenBackend.h"

namespace pg
{
	// Deterministic no-op LLM backend for tests: records the model path + GPU-layer count it was asked to
	// load with and the parameters of the most recent Generate call, and returns a fixed completion that
	// echoes the prompt (empty when no model is loaded). Lets tests verify the request -> generate ->
	// result path (and the config -> LoadModel GPU-offload plumbing) without any real inference, GGUF
	// weights, or llama.cpp.
	class TestingTextGenBackend : public TextGenBackend
	{
	public:
		bool LoadModel(const std::string& modelPath, int gpuLayers) override;
		bool IsLoaded() const override { return m_Loaded; }
		std::string Generate(const TextGenParams& params) override;

		const std::string& GetModelPath() const { return m_ModelPath; }
		int GetGpuLayers() const { return m_GpuLayers; }
		const TextGenParams& GetLastParams() const { return m_LastParams; }
		int GetGenerateCount() const { return m_GenerateCount; }

	private:
		bool m_Loaded = false;
		std::string m_ModelPath;
		int m_GpuLayers = -1; // sentinel: no load yet (a real load records 0 = CPU or > 0 = GPU layers)
		TextGenParams m_LastParams;
		int m_GenerateCount = 0;
	};
}
