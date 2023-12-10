#pragma once

#include "pigeon/Renderer/Shader.h"

#include <string>
#include <d3d11.h>

namespace pigeon
{
	class Dx11Shader : public Shader
	{
	public:
		Dx11Shader(const char* vertexSrc, const char* fragmentSrc, const BufferLayout& buffLayout);
		~Dx11Shader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		ID3D11InputLayout* m_InputLayout = nullptr;
		ID3D11VertexShader* m_VertexShader = nullptr;
		ID3D11PixelShader* m_PixelShader = nullptr;
	};
}