#pragma once

#include "Pigeon/Renderer/Buffer.h"

#include <d3d11.h>
namespace pig 
{
	class Dx11VertexBuffer : public VertexBuffer
	{
	public:
		struct Data
		{
			ID3D11Buffer* m_Buffer = nullptr;
		};

		Dx11VertexBuffer(float* vertices, uint32_t size);
		virtual ~Dx11VertexBuffer();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		Data m_Data;
	};

	class Dx11IndexBuffer : public IndexBuffer
	{
	public:
		struct Data
		{
			ID3D11Buffer* m_Buffer = nullptr;
			uint32_t m_Count;
		};

		Dx11IndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~Dx11IndexBuffer();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const { return m_Data.m_Count; }
	private:
		Data m_Data;
	};

	template<typename T>
	class Dx11ConstantBuffer
	{
	public:
		struct Data
		{
			ID3D11Buffer* m_Buffer = nullptr;
		};

		Dx11ConstantBuffer(T* data)
		{
			D3D11_BUFFER_DESC bd = {};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(T);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;
			
			D3D11_SUBRESOURCE_DATA initData = { 0 };
			initData.pSysMem = data;

			auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
			context->GetPd3dDevice()->CreateBuffer(&bd, &initData, &m_Data.m_Buffer);
		}
		~Dx11ConstantBuffer()
		{
			if (m_Data.m_Buffer)
				m_Data.m_Buffer->Release();
		}

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void Bind(UINT slot)
		{
			auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
			context->GetPd3dDeviceContext()->VSSetConstantBuffers(slot, 1, &m_Data.m_Buffer);
		}

	private:
		Data m_Data;
	};
}