#include "pch.h"
#include "Platform/Onnx/OnnxMattingBackend.h"

// The onnxruntime runtime (CUDA execution provider) is bound only when PigeonLib is built with
// -DPG_ENABLE_MATTING=ON (which fetches the onnxruntime GPU package and defines PG_MATTING_ENABLED).
// Without it this backend is inert: it records the model path but produces no matte, so a plain build
// links without the heavy runtime and the figure composite falls back to the skeleton silhouette
// (ADR 0012). The Testing build never selects this backend (it uses TestingMattingBackend).
#ifdef PG_MATTING_ENABLED

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <onnxruntime_cxx_api.h>

namespace
{
	// Bilinearly samples one RGB channel of an 8-bit image at fractional (sx, sy), clamping to the edge.
	float SampleChannel(const pg::Image& image, float sx, float sy, int channel)
	{
		const int width = static_cast<int>(image.m_Width);
		const int height = static_cast<int>(image.m_Height);
		int x0 = static_cast<int>(std::floor(sx));
		int y0 = static_cast<int>(std::floor(sy));
		const float fx = sx - x0;
		const float fy = sy - y0;
		int x1 = std::clamp(x0 + 1, 0, width - 1);
		int y1 = std::clamp(y0 + 1, 0, height - 1);
		x0 = std::clamp(x0, 0, width - 1);
		y0 = std::clamp(y0, 0, height - 1);
		const float p00 = image.m_Pixels[(static_cast<size_t>(y0) * width + x0) * 3 + channel];
		const float p10 = image.m_Pixels[(static_cast<size_t>(y0) * width + x1) * 3 + channel];
		const float p01 = image.m_Pixels[(static_cast<size_t>(y1) * width + x0) * 3 + channel];
		const float p11 = image.m_Pixels[(static_cast<size_t>(y1) * width + x1) * 3 + channel];
		const float top = p00 + (p10 - p00) * fx;
		const float bottom = p01 + (p11 - p01) * fx;
		return top + (bottom - top) * fy;
	}

	// Bilinearly samples a single-channel float map (mapWidth x mapHeight) at fractional (sx, sy).
	float SampleMap(const std::vector<float>& map, int mapWidth, int mapHeight, float sx, float sy)
	{
		int x0 = static_cast<int>(std::floor(sx));
		int y0 = static_cast<int>(std::floor(sy));
		const float fx = sx - x0;
		const float fy = sy - y0;
		int x1 = std::clamp(x0 + 1, 0, mapWidth - 1);
		int y1 = std::clamp(y0 + 1, 0, mapHeight - 1);
		x0 = std::clamp(x0, 0, mapWidth - 1);
		y0 = std::clamp(y0, 0, mapHeight - 1);
		const float p00 = map[static_cast<size_t>(y0) * mapWidth + x0];
		const float p10 = map[static_cast<size_t>(y0) * mapWidth + x1];
		const float p01 = map[static_cast<size_t>(y1) * mapWidth + x0];
		const float p11 = map[static_cast<size_t>(y1) * mapWidth + x1];
		const float top = p00 + (p10 - p00) * fx;
		const float bottom = p01 + (p11 - p01) * fx;
		return top + (bottom - top) * fy;
	}
}

pg::OnnxMattingBackend::~OnnxMattingBackend()
{
	delete static_cast<Ort::Session*>(m_Session);
	delete static_cast<Ort::Env*>(m_Env);
	m_Session = nullptr;
	m_Env = nullptr;
}

bool pg::OnnxMattingBackend::LoadModel(const std::string& modelPath)
{
	m_ModelPath = modelPath;
	if (modelPath.empty())
	{
		return false;
	}

	try
	{
		Ort::Env* env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "pg_matting");

#ifdef _WIN32
		const std::wstring modelPathNative(modelPath.begin(), modelPath.end());
		const wchar_t* modelPathArg = modelPathNative.c_str();
#else
		const char* modelPathArg = modelPath.c_str();
#endif

		// Try the CUDA execution provider first (shares the GPU with the resident diffusion checkpoint,
		// ADR 0012), but FALL BACK to CPU when it cannot initialise. onnxruntime's CUDA EP additionally
		// needs cuDNN, which the GGML CUDA stack (cuBLAS/cudart only) does not ship — so a GPU that runs
		// diffusion fine may still reject this EP, and without the fallback the matte silently fails and the
		// composite drops to the skeleton silhouette. CPU isnet is ~1-2s, acceptable next to the diffusion job.
		Ort::Session* session = nullptr;
		try
		{
			Ort::SessionOptions cudaSessionOptions;
			cudaSessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			OrtCUDAProviderOptions cudaOptions{};
			cudaOptions.device_id = 0;
			cudaSessionOptions.AppendExecutionProvider_CUDA(cudaOptions);
			session = new Ort::Session(*env, modelPathArg, cudaSessionOptions);
			PG_CORE_INFO("OnnxMattingBackend: matting on the CUDA execution provider");
		}
		catch (const std::exception& cudaError)
		{
			PG_CORE_WARN("OnnxMattingBackend: CUDA execution provider unavailable ({0}); falling back to CPU", cudaError.what());
			Ort::SessionOptions cpuSessionOptions;
			cpuSessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
			session = new Ort::Session(*env, modelPathArg, cpuSessionOptions);
			PG_CORE_INFO("OnnxMattingBackend: matting on the CPU execution provider");
		}

		Ort::AllocatorWithDefaultOptions allocator;
		Ort::AllocatedStringPtr inputName = session->GetInputNameAllocated(0, allocator);
		Ort::AllocatedStringPtr outputName = session->GetOutputNameAllocated(0, allocator);
		m_InputName = inputName.get();
		m_OutputName = outputName.get();

		// NCHW input; a dynamic spatial axis (<= 0) falls back to the isnet default already in the members.
		const std::vector<int64_t> inputShape = session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
		if (inputShape.size() == 4)
		{
			if (inputShape[2] > 0) m_InputHeight = inputShape[2];
			if (inputShape[3] > 0) m_InputWidth = inputShape[3];
		}

		m_Env = env;
		m_Session = session;
		PG_CORE_INFO("OnnxMattingBackend: loaded matting model '{0}' (input {1}x{2}, in='{3}', out='{4}')", modelPath, m_InputWidth, m_InputHeight, m_InputName, m_OutputName);
		return true;
	}
	catch (const std::exception& e)
	{
		PG_CORE_ERROR("OnnxMattingBackend: failed to load '{0}': {1}", modelPath, e.what());
		delete static_cast<Ort::Session*>(m_Session);
		delete static_cast<Ort::Env*>(m_Env);
		m_Session = nullptr;
		m_Env = nullptr;
		return false;
	}
}

bool pg::OnnxMattingBackend::IsLoaded() const
{
	return m_Session != nullptr;
}

pg::Image pg::OnnxMattingBackend::Matte(const pg::Image& input)
{
	if (!IsLoaded() || input.m_Pixels.empty())
	{
		return pg::Image{};
	}

	const int inputWidth = static_cast<int>(input.m_Width);
	const int inputHeight = static_cast<int>(input.m_Height);
	const int modelWidth = static_cast<int>(m_InputWidth);
	const int modelHeight = static_cast<int>(m_InputHeight);

	try
	{
		// Preprocess: resize the figure to the model's input size and lay it out as a planar NCHW float
		// tensor, normalised to roughly [-0.5, 0.5] (the isnet/u2net convention: pixels/255 then minus 0.5).
		std::vector<float> tensorData(static_cast<size_t>(3) * modelWidth * modelHeight);
		const float scaleX = static_cast<float>(inputWidth) / modelWidth;
		const float scaleY = static_cast<float>(inputHeight) / modelHeight;
		const size_t plane = static_cast<size_t>(modelWidth) * modelHeight;
		for (int y = 0; y < modelHeight; ++y)
		{
			const float sy = (y + 0.5f) * scaleY - 0.5f;
			for (int x = 0; x < modelWidth; ++x)
			{
				const float sx = (x + 0.5f) * scaleX - 0.5f;
				const size_t offset = static_cast<size_t>(y) * modelWidth + x;
				tensorData[0 * plane + offset] = SampleChannel(input, sx, sy, 0) / 255.f - 0.5f;
				tensorData[1 * plane + offset] = SampleChannel(input, sx, sy, 1) / 255.f - 0.5f;
				tensorData[2 * plane + offset] = SampleChannel(input, sx, sy, 2) / 255.f - 0.5f;
			}
		}

		Ort::Session& session = *static_cast<Ort::Session*>(m_Session);
		Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
		const std::array<int64_t, 4> inputShape{ 1, 3, m_InputHeight, m_InputWidth };
		Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, tensorData.data(), tensorData.size(), inputShape.data(), inputShape.size());

		const char* inputNames[] = { m_InputName.c_str() };
		const char* outputNames[] = { m_OutputName.c_str() };
		std::vector<Ort::Value> outputs = session.Run(Ort::RunOptions{ nullptr }, inputNames, &inputTensor, 1, outputNames, 1);
		if (outputs.empty())
		{
			return pg::Image{};
		}

		const float* output = outputs[0].GetTensorData<float>();
		// The saliency map is the trailing modelHeight*modelWidth of the output ([1,1,H,W] or [1,H,W]).
		float minValue = output[0];
		float maxValue = output[0];
		for (size_t i = 1; i < plane; ++i)
		{
			minValue = std::min(minValue, output[i]);
			maxValue = std::max(maxValue, output[i]);
		}
		const float range = std::max(maxValue - minValue, 1e-6f);

		// Min-max normalise to [0,1], resize back to the figure's size, and replicate across RGB so the
		// matte feeds the mask-composite alpha (which reads the red channel).
		std::vector<float> mask(plane);
		for (size_t i = 0; i < plane; ++i)
		{
			mask[i] = (output[i] - minValue) / range;
		}

		pg::Image matte;
		matte.m_Width = input.m_Width;
		matte.m_Height = input.m_Height;
		matte.m_Pixels.resize(static_cast<size_t>(inputWidth) * inputHeight * 3);
		const float invScaleX = static_cast<float>(modelWidth) / inputWidth;
		const float invScaleY = static_cast<float>(modelHeight) / inputHeight;
		for (int y = 0; y < inputHeight; ++y)
		{
			const float sy = (y + 0.5f) * invScaleY - 0.5f;
			for (int x = 0; x < inputWidth; ++x)
			{
				const float sx = (x + 0.5f) * invScaleX - 0.5f;
				const float alpha = std::clamp(SampleMap(mask, modelWidth, modelHeight, sx, sy), 0.f, 1.f);
				const uint8_t value = static_cast<uint8_t>(alpha * 255.f + 0.5f);
				const size_t index = (static_cast<size_t>(y) * inputWidth + x) * 3;
				matte.m_Pixels[index] = value;
				matte.m_Pixels[index + 1] = value;
				matte.m_Pixels[index + 2] = value;
			}
		}
		return matte;
	}
	catch (const std::exception& e)
	{
		PG_CORE_ERROR("OnnxMattingBackend: matte inference threw '{0}'; returning empty matte", e.what());
		return pg::Image{};
	}
}

#else // PG_MATTING_ENABLED — inert no-op: links without onnxruntime; composite falls back to the skeleton.

pg::OnnxMattingBackend::~OnnxMattingBackend() = default;

bool pg::OnnxMattingBackend::LoadModel(const std::string& modelPath)
{
	m_ModelPath = modelPath;
	return false;
}

bool pg::OnnxMattingBackend::IsLoaded() const
{
	return false;
}

pg::Image pg::OnnxMattingBackend::Matte(const pg::Image& /*input*/)
{
	return pg::Image{};
}

#endif // PG_MATTING_ENABLED
