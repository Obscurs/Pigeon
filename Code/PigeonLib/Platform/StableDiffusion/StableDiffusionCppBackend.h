#pragma once

#include <string>

#include "Pigeon/Diffusion/DiffusionBackend.h"

namespace pg
{
	// Real text-to-image backend backed by stable-diffusion.cpp (CUDA). Selected for DirectX11 builds.
	// The heavy inference runtime is linked only in non-test builds; the Testing build never selects
	// this backend (it uses TestingDiffusionBackend), so this class carries no test-only code.
	class StableDiffusionCppBackend : public DiffusionBackend
	{
	public:
		StableDiffusionCppBackend() = default;
		~StableDiffusionCppBackend() override;

		bool LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath) override;
		bool IsLoaded() const override;
		pg::Image Generate(const DiffusionJobParams& params) override;

	private:
		// Opaque stable-diffusion.cpp context (sd_ctx_t*), owned for the session. Held as void* so this
		// header stays free of the third-party include; the .cpp casts it.
		void* m_Context = nullptr;
		std::string m_CheckpointPath;
		std::string m_ControlNetPath;
		std::string m_VaePath;
	};
}
