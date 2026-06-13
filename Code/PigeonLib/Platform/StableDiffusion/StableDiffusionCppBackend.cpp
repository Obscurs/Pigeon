#include "pch.h"
#include "Platform/StableDiffusion/StableDiffusionCppBackend.h"

// The stable-diffusion.cpp runtime (CUDA) is bound only when PigeonLib is built with
// -DPG_ENABLE_DIFFUSION=ON (which fetches the library and defines PG_STABLE_DIFFUSION_ENABLED).
// Without it this backend is inert: it records the resident model paths but produces no image, so a
// plain build links and runs without the heavy runtime, and the Testing build never needs CUDA.
#ifdef PG_STABLE_DIFFUSION_ENABLED

#include <vector>

#include <stable-diffusion.h>

namespace
{
	// Forwards stable-diffusion.cpp's internal logging to the engine logger, so load/generation failures
	// (unsupported tensors, OOM, missing files) surface in the console instead of being swallowed.
	void SdLogCallback(enum sd_log_level_t level, const char* text, void* /*data*/)
	{
		if (text == nullptr)
		{
			return;
		}
		switch (level)
		{
		case SD_LOG_ERROR: PG_CORE_ERROR("sd.cpp: {0}", text); break;
		case SD_LOG_WARN:  PG_CORE_WARN("sd.cpp: {0}", text); break;
		default:           PG_CORE_INFO("sd.cpp: {0}", text); break;
		}
	}

	sample_method_t SamplerFromName(const std::string& name)
	{
		if (name == "euler") return EULER_SAMPLE_METHOD;
		if (name == "heun") return HEUN_SAMPLE_METHOD;
		if (name == "dpm2") return DPM2_SAMPLE_METHOD;
		if (name == "dpm++2s_a" || name == "dpmpp2s_a") return DPMPP2S_A_SAMPLE_METHOD;
		if (name == "dpm++2m" || name == "dpmpp2m") return DPMPP2M_SAMPLE_METHOD;
		if (name == "lcm") return LCM_SAMPLE_METHOD;
		return EULER_A_SAMPLE_METHOD; // default / "euler_a"
	}

	sd_image_t MakeImageView(const pg::Image& image)
	{
		sd_image_t view{};
		view.width = image.m_Width;
		view.height = image.m_Height;
		view.channel = 3;
		view.data = const_cast<uint8_t*>(image.m_Pixels.data());
		return view;
	}

	pg::Image ToImage(const sd_image_t& result)
	{
		pg::Image image;
		if (result.data == nullptr || result.channel < 3)
		{
			return image;
		}
		image.m_Width = result.width;
		image.m_Height = result.height;
		const size_t pixelCount = static_cast<size_t>(result.width) * result.height;
		image.m_Pixels.resize(pixelCount * 3);
		for (size_t i = 0; i < pixelCount; ++i)
		{
			image.m_Pixels[i * 3 + 0] = result.data[i * result.channel + 0];
			image.m_Pixels[i * 3 + 1] = result.data[i * result.channel + 1];
			image.m_Pixels[i * 3 + 2] = result.data[i * result.channel + 2];
		}
		return image;
	}
}

pg::StableDiffusionCppBackend::~StableDiffusionCppBackend()
{
	if (m_Context != nullptr)
	{
		free_sd_ctx(static_cast<sd_ctx_t*>(m_Context));
		m_Context = nullptr;
	}
}

bool pg::StableDiffusionCppBackend::LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath)
{
	m_CheckpointPath = checkpointPath;
	m_ControlNetPath = controlNetPath;
	m_VaePath = vaePath;
	if (checkpointPath.empty())
	{
		return false;
	}

	sd_set_log_callback(SdLogCallback, nullptr);

	// Start from the library's defaults, then point it at the resident checkpoint (+ ControlNet + VAE).
	sd_ctx_params_t params;
	sd_ctx_params_init(&params);
	params.model_path = m_CheckpointPath.c_str();
	if (!m_ControlNetPath.empty())
	{
		params.control_net_path = m_ControlNetPath.c_str();
	}
	if (!m_VaePath.empty())
	{
		params.vae_path = m_VaePath.c_str();
		// The stock SDXL fp16 VAE overflows to NaN on GPU (decodes to a blank/white image); decoding it
		// on the CPU runs in fp32 and avoids that. Slower per generation, but correct.
		params.keep_vae_on_cpu = true;
	}

	m_Context = new_sd_ctx(&params);
	if (m_Context == nullptr)
	{
		PG_CORE_ERROR("StableDiffusion: new_sd_ctx failed to load checkpoint '{0}'", checkpointPath);
	}
	else
	{
		PG_CORE_INFO("StableDiffusion: loaded checkpoint '{0}' (controlnet '{1}')", checkpointPath, controlNetPath);
	}
	return m_Context != nullptr;
}

bool pg::StableDiffusionCppBackend::IsLoaded() const
{
	return m_Context != nullptr;
}

pg::Image pg::StableDiffusionCppBackend::Generate(const pg::DiffusionJobParams& params)
{
	if (m_Context == nullptr)
	{
		return pg::Image{};
	}

	sd_img_gen_params_t gen;
	sd_img_gen_params_init(&gen);
	gen.prompt = params.m_Prompt.c_str();
	gen.negative_prompt = params.m_NegativePrompt.c_str();
	gen.clip_skip = params.m_ClipSkip;
	gen.width = static_cast<int>(params.m_Width);
	gen.height = static_cast<int>(params.m_Height);
	gen.seed = params.m_Seed;
	gen.batch_count = 1;
	gen.sample_params.sample_method = SamplerFromName(params.m_Sampler);
	gen.sample_params.sample_steps = params.m_Steps;
	gen.sample_params.guidance.txt_cfg = params.m_CfgScale;

	// LoRAs are applied natively per generation (path + multiplier) on top of the resident checkpoint.
	std::vector<sd_lora_t> loras;
	loras.reserve(params.m_Loras.size());
	for (const pg::DiffusionLora& lora : params.m_Loras)
	{
		sd_lora_t entry{};
		entry.is_high_noise = false;
		entry.multiplier = lora.m_Weight;
		entry.path = lora.m_Path.c_str();
		loras.push_back(entry);
	}
	gen.loras = loras.empty() ? nullptr : loras.data();
	gen.lora_count = static_cast<uint32_t>(loras.size());

	if (params.m_HasControlHint && !params.m_ControlHint.m_Pixels.empty())
	{
		gen.control_image = MakeImageView(params.m_ControlHint);
		gen.control_strength = params.m_ControlStrength;
	}
	if (params.m_HasInitImage && !params.m_InitImage.m_Pixels.empty())
	{
		gen.init_image = MakeImageView(params.m_InitImage);
		gen.strength = params.m_DenoiseStrength;
	}

	sd_image_t* results = generate_image(static_cast<sd_ctx_t*>(m_Context), &gen);
	pg::Image image;
	if (results != nullptr)
	{
		image = ToImage(results[0]);
		free_sd_images(results, 1);
	}
	if (image.m_Pixels.empty())
	{
		PG_CORE_WARN("StableDiffusion: generate_image returned no image (check VAE/params/VRAM)");
	}
	return image;
}

#else // PG_STABLE_DIFFUSION_ENABLED

pg::StableDiffusionCppBackend::~StableDiffusionCppBackend() = default;

bool pg::StableDiffusionCppBackend::LoadCheckpoint(const std::string& checkpointPath, const std::string& controlNetPath, const std::string& vaePath)
{
	m_CheckpointPath = checkpointPath;
	m_ControlNetPath = controlNetPath;
	m_VaePath = vaePath;
	return IsLoaded();
}

bool pg::StableDiffusionCppBackend::IsLoaded() const
{
	return m_Context != nullptr;
}

pg::Image pg::StableDiffusionCppBackend::Generate(const pg::DiffusionJobParams& params)
{
	(void)params;
	return pg::Image{};
}

#endif // PG_STABLE_DIFFUSION_ENABLED
