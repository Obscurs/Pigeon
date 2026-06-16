#pragma once

#include <string>

#include "Pigeon/TextGen/TextGenBackend.h"

namespace pg
{
	// Real LLM inference backend wrapping llama.cpp (CUDA). Active only when PigeonLib is built with
	// -DPG_ENABLE_TEXTGEN=ON (which fetches llama.cpp and defines PG_LLAMA_ENABLED); otherwise it is an
	// inert no-op that records the resident model path but produces no text, so a plain build links and
	// runs without the heavy runtime and the Testing build never needs it.
	class LlamaCppBackend : public TextGenBackend
	{
	public:
		LlamaCppBackend() = default;
		~LlamaCppBackend() override;

		bool LoadModel(const std::string& modelPath, int gpuLayers) override;
		bool IsLoaded() const override;
		std::string Generate(const TextGenParams& params) override;

	private:
		std::string m_ModelPath;
		// Opaque llama.cpp handles (the concrete types are named only inside the .cpp); null until a model
		// is loaded.
		void* m_Model = nullptr;
		void* m_Context = nullptr;
	};
}
