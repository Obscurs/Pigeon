#pragma once
#include "Pigeon/Core/Core.h"
#include "Pigeon/Renderer/Buffer.h"
#include "Pigeon/Renderer/Shader.h"

namespace pg
{
	// Upper bounds for a single model's GPU buffers; the dynamic buffers are sized to these once and the
	// 3D pass re-uploads each model's geometry per draw.
	static const unsigned int MODEL_MAX_VERTICES = 65536;
	static const unsigned int MODEL_MAX_INDICES = 196608;

	struct Renderer3DDataSingletonComponent
	{
		Renderer3DDataSingletonComponent() {};
		Renderer3DDataSingletonComponent(const Renderer3DDataSingletonComponent&) = default;

		pg::S_Ptr<pg::VertexBuffer> m_VertexBuffer = nullptr;
		pg::S_Ptr<pg::IndexBuffer> m_IndexBuffer = nullptr;

		pg::S_Ptr<pg::Shader> m_ModelShader = nullptr;
	};
}
