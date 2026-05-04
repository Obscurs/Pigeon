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
			pig::U_Ptr<ID3D11Buffer, pig::ReleaseDeleter> m_Buffer = nullptr;
			UINT m_Stride = 0;
		};

		Dx11VertexBuffer(const float* vertices, uint32_t size, uint32_t stride);
		virtual ~Dx11VertexBuffer() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetVertices(const float* vertices, unsigned int count, unsigned int countOffset) override;
		virtual void AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset) override;

		Data m_Data;
	};

	class Dx11IndexBuffer : public IndexBuffer
	{
	public:
		struct Data
		{
			pig::U_Ptr<ID3D11Buffer, pig::ReleaseDeleter> m_Buffer = nullptr;
			uint32_t m_Count;
		};

		Dx11IndexBuffer(const uint32_t* indices, uint32_t count);
		virtual ~Dx11IndexBuffer() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) override;
		virtual void AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset) override;

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
			pig::U_Ptr<ID3D11Buffer, pig::ReleaseDeleter> m_Buffer = nullptr;
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
			ID3D11Buffer* buffer = nullptr;
			context->GetPd3dDevice()->CreateBuffer(&bd, &initData, &buffer);
			m_Data.m_Buffer.reset(buffer);
		}
		~Dx11ConstantBuffer() = default;

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
#endif

		void Bind(UINT slot)
		{
			auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
			ID3D11Buffer* buffer = m_Data.m_Buffer.get();
			context->GetPd3dDeviceContext()->VSSetConstantBuffers(slot, 1, &buffer);
		}

	private:
		Data m_Data;
	};
}