#pragma once

#include <string>
#include <d3d11.h>

namespace pigeon {

	class Shader
	{
	public:
		Shader(const char* vertexSrc, const char* fragmentSrc);
		~Shader();

		void Bind() const;
		void Unbind() const;
	private:
		ID3D11InputLayout* m_InputLayout = nullptr;
		ID3D11VertexShader* m_VertexShader = nullptr;
		ID3D11PixelShader* m_PixelShader = nullptr;
	};

}