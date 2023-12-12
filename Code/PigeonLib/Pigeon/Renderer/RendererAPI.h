#pragma once

#include <glm/glm.hpp>

namespace pig 
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, DirectX11 = 1
		};
	public:
		virtual void SetClearColor(const glm::vec4& color) = 0;

		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(unsigned int count) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}