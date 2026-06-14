#pragma once

#include "Pigeon/ECS/System.h"

namespace pg
{
	// Drives in-engine text generation (ADR 0009). Returns early until the engine config and resource map
	// exist. Lazily creates the LLM backend + job + result singletons (deferred), then loads the resident
	// GGUF model once from the resource map's language-model paths. Each frame it reaps a finished Text
	// Gen Job — writing the completion into the result store under the caller-assigned target UUID — and,
	// while idle and the backend is loaded, assembles one GenerateTextRequest into backend job parameters
	// (resolving Text Generation Config defaults) and launches a single generation on a background worker
	// thread so the frame loop never blocks. Only one job runs at a time.
	class TextGenSystem : public pg::System
	{
	public:
		TextGenSystem() = default;
		~TextGenSystem() = default;
		void Update(const pg::Timestep& ts) override;
		pg::SystemAccessDecl DeclareAccess() const override;
	};
}
