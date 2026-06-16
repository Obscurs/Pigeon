#pragma once

#include <cstdint>
#include <string>

#include "Pigeon/Core/Core.h"

namespace pg
{
	// The fully-resolved inputs of one text generation, assembled by TextGenSystem from a Generate Text
	// Request and the engine defaults. Everything the backend needs to run one prompt->completion loop,
	// with no ECS or resource-map knowledge.
	struct TextGenParams
	{
		std::string m_Prompt;
		std::string m_SystemPrompt;
		int m_MaxTokens = 256;
		float m_Temperature = 0.8f;
		float m_TopP = 0.95f;
		int64_t m_Seed = -1;
	};

	// Platform-abstracted LLM inference runtime. The concrete backend (llama.cpp for real builds, a
	// deterministic no-op for tests) is selected by Create() using the same renderer-API switch as the
	// other platform resources (cf. AudioDevice / DiffusionBackend). Held in
	// TextGenBackendSingletonComponent.
	class TextGenBackend
	{
	public:
		virtual ~TextGenBackend() = default;

		// Loads the resident GGUF model once at startup; returns true on success. The loaded model stays
		// resident for the session. gpuLayers is how many transformer layers to offload to the GPU
		// (ADR 0010): 0 = CPU-only, a large value (e.g. 999) = all layers.
		virtual bool LoadModel(const std::string& modelPath, int gpuLayers) = 0;
		virtual bool IsLoaded() const = 0;

		// Runs one prompt->completion synchronously (invoked on a background worker thread). Returns an
		// empty string on failure or when no model is loaded.
		virtual std::string Generate(const TextGenParams& params) = 0;

		static pg::S_Ptr<TextGenBackend> Create();
	};
}
