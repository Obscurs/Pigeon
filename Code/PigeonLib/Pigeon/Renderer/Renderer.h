#pragma once

namespace pigeon 
{

	enum class RendererAPI
	{
		None = 0, DirectX11 = 1
	};

	class Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return s_RendererAPI; }
	private:
		static RendererAPI s_RendererAPI;
	};


}