#pragma once

#include <cstdint>
#include <string>

#include "Pigeon/Diffusion/MattingBackend.h"

namespace pg
{
	// Real image-matting backend backed by onnxruntime with the CUDA execution provider (ADR 0012).
	// Selected for DirectX11 builds. The heavy ONNX runtime is linked only when PigeonLib is built with
	// -DPG_ENABLE_MATTING=ON (which defines PG_MATTING_ENABLED); otherwise this class is an inert no-op
	// (IsLoaded() == false, Matte() returns empty) so a plain build links without onnxruntime and the
	// figure composite falls back to the skeleton silhouette. The Testing build never selects this backend
	// (it uses TestingMattingBackend), so this class carries no test-only code.
	class OnnxMattingBackend : public MattingBackend
	{
	public:
		OnnxMattingBackend() = default;
		~OnnxMattingBackend() override;

		bool LoadModel(const std::string& modelPath) override;
		bool IsLoaded() const override;
		pg::Image Matte(const pg::Image& input) override;

	private:
		// Opaque onnxruntime objects (Ort::Env*, Ort::Session*), owned for the session. Held as void* so
		// this header stays free of the third-party include; the .cpp casts them.
		void* m_Env = nullptr;
		void* m_Session = nullptr;
		std::string m_ModelPath;
		std::string m_InputName;
		std::string m_OutputName;
		// The model's expected input resolution (NCHW). isnet-general-use is 1024x1024; read from the model
		// at load and used as the fallback for a dynamic axis.
		int64_t m_InputWidth = 1024;
		int64_t m_InputHeight = 1024;
	};
}
