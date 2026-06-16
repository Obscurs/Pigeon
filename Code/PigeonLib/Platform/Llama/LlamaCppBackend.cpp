#include "pch.h"
#include "Platform/Llama/LlamaCppBackend.h"

// The llama.cpp runtime is bound only when PigeonLib is built with -DPG_ENABLE_TEXTGEN=ON (which fetches
// the library and defines PG_LLAMA_ENABLED). Without it this backend is inert: it records the resident
// model path but produces no text, so a plain build links and runs without the heavy runtime, and the
// Testing build never needs it.

#ifdef PG_LLAMA_ENABLED

#include <cstdint>
#include <vector>

#include <llama.h>

namespace
{
	// Forwards llama.cpp's internal logging to the engine logger so load/inference failures (bad GGUF,
	// OOM, unsupported arch) surface in the console instead of being swallowed.
	void LlamaLogCallback(enum ggml_log_level level, const char* text, void* /*data*/)
	{
		if (text == nullptr)
		{
			return;
		}
		switch (level)
		{
		case GGML_LOG_LEVEL_ERROR: PG_CORE_ERROR("llama: {0}", text); break;
		case GGML_LOG_LEVEL_WARN:  PG_CORE_WARN("llama: {0}", text); break;
		default:                   PG_CORE_INFO("llama: {0}", text); break;
		}
	}

	// Builds the model's chat-formatted prompt from an optional system message + the user prompt, using
	// the GGUF's own chat template so the special tokens match the model (here, Llama-3). Falls back to
	// the raw prompt when the model carries no template.
	std::string ApplyChatTemplate(llama_model* model, const pg::TextGenParams& params)
	{
		const char* tmpl = llama_model_chat_template(model, nullptr);
		if (tmpl == nullptr)
		{
			return params.m_Prompt;
		}

		std::vector<llama_chat_message> messages;
		if (!params.m_SystemPrompt.empty())
		{
			messages.push_back(llama_chat_message{ "system", params.m_SystemPrompt.c_str() });
		}
		messages.push_back(llama_chat_message{ "user", params.m_Prompt.c_str() });

		std::vector<char> buffer(params.m_Prompt.size() + params.m_SystemPrompt.size() + 512);
		int written = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, buffer.data(), static_cast<int32_t>(buffer.size()));
		if (written > static_cast<int>(buffer.size()))
		{
			buffer.resize(written);
			written = llama_chat_apply_template(tmpl, messages.data(), messages.size(), true, buffer.data(), static_cast<int32_t>(buffer.size()));
		}
		if (written < 0)
		{
			return params.m_Prompt;
		}
		return std::string(buffer.data(), written);
	}
}

pg::LlamaCppBackend::~LlamaCppBackend()
{
	if (m_Context != nullptr)
	{
		llama_free(static_cast<llama_context*>(m_Context));
		m_Context = nullptr;
	}
	if (m_Model != nullptr)
	{
		llama_model_free(static_cast<llama_model*>(m_Model));
		m_Model = nullptr;
	}
}

bool pg::LlamaCppBackend::LoadModel(const std::string& modelPath, int gpuLayers)
{
	m_ModelPath = modelPath;
	if (modelPath.empty())
	{
		return false;
	}

	llama_log_set(LlamaLogCallback, nullptr);
	llama_backend_init();

	// Offload gpuLayers transformer layers to the GPU (ADR 0010): the default (999) offloads all layers
	// for full-GPU inference, 0 keeps it CPU-only. The app sizes the model so the offloaded weights
	// co-reside in VRAM with the renderer and the resident diffusion checkpoint.
	llama_model_params modelParams = llama_model_default_params();
	modelParams.n_gpu_layers = gpuLayers;

	llama_model* model = llama_model_load_from_file(m_ModelPath.c_str(), modelParams);
	if (model == nullptr)
	{
		PG_CORE_ERROR("Llama: failed to load model '{0}'", modelPath);
		return false;
	}

	llama_context_params contextParams = llama_context_default_params();
	contextParams.n_ctx = 4096;
	contextParams.n_batch = 512;

	llama_context* context = llama_init_from_model(model, contextParams);
	if (context == nullptr)
	{
		PG_CORE_ERROR("Llama: failed to create context for '{0}'", modelPath);
		llama_model_free(model);
		return false;
	}

	m_Model = model;
	m_Context = context;
	PG_CORE_INFO("Llama: loaded model '{0}' ({1} GPU layers)", modelPath, gpuLayers);
	return true;
}

bool pg::LlamaCppBackend::IsLoaded() const
{
	return m_Context != nullptr && m_Model != nullptr;
}

std::string pg::LlamaCppBackend::Generate(const pg::TextGenParams& params)
{
	if (!IsLoaded())
	{
		return std::string();
	}

	llama_model* model = static_cast<llama_model*>(m_Model);
	llama_context* context = static_cast<llama_context*>(m_Context);
	const llama_vocab* vocab = llama_model_get_vocab(model);

	// Clear any state from a previous generation so each request starts fresh.
	llama_memory_clear(llama_get_memory(context), true);

	const std::string prompt = ApplyChatTemplate(model, params);

	// Tokenize the prompt (two-pass: query the count, then fill).
	const int32_t needed = -llama_tokenize(vocab, prompt.c_str(), static_cast<int32_t>(prompt.size()), nullptr, 0, true, true);
	std::vector<llama_token> tokens(needed);
	if (llama_tokenize(vocab, prompt.c_str(), static_cast<int32_t>(prompt.size()), tokens.data(), static_cast<int32_t>(tokens.size()), true, true) < 0)
	{
		PG_CORE_ERROR("Llama: tokenization failed");
		return std::string();
	}

	// Sampler chain: top-p then temperature then a seeded distribution sample.
	const uint32_t seed = params.m_Seed < 0 ? LLAMA_DEFAULT_SEED : static_cast<uint32_t>(params.m_Seed);
	llama_sampler* sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
	llama_sampler_chain_add(sampler, llama_sampler_init_top_p(params.m_TopP, 1));
	llama_sampler_chain_add(sampler, llama_sampler_init_temp(params.m_Temperature));
	llama_sampler_chain_add(sampler, llama_sampler_init_dist(seed));

	std::string output;
	llama_batch batch = llama_batch_get_one(tokens.data(), static_cast<int32_t>(tokens.size()));

	char pieceBuffer[256];
	for (int generated = 0; generated < params.m_MaxTokens; ++generated)
	{
		if (llama_decode(context, batch) != 0)
		{
			PG_CORE_ERROR("Llama: llama_decode failed");
			break;
		}

		llama_token token = llama_sampler_sample(sampler, context, -1);
		if (llama_vocab_is_eog(vocab, token))
		{
			break;
		}

		const int pieceLength = llama_token_to_piece(vocab, token, pieceBuffer, sizeof(pieceBuffer), 0, true);
		if (pieceLength > 0)
		{
			output.append(pieceBuffer, pieceLength);
		}

		batch = llama_batch_get_one(&token, 1);
	}

	llama_sampler_free(sampler);
	return output;
}

#else // PG_LLAMA_ENABLED

pg::LlamaCppBackend::~LlamaCppBackend() = default;

bool pg::LlamaCppBackend::LoadModel(const std::string& modelPath, int gpuLayers)
{
	(void)gpuLayers;
	m_ModelPath = modelPath;
	return IsLoaded();
}

bool pg::LlamaCppBackend::IsLoaded() const
{
	return m_Context != nullptr;
}

std::string pg::LlamaCppBackend::Generate(const pg::TextGenParams& params)
{
	(void)params;
	return std::string();
}

#endif // PG_LLAMA_ENABLED
