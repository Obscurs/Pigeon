#include "pch.h"
#include "Dx11Buffer.h"

#include "Pigeon/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::Dx11VertexBuffer::Dx11VertexBuffer(const float* vertices, uint32_t size, uint32_t stride)
{
	D3D11_BUFFER_DESC bd = { 0 };
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = context->GetPd3dDevice()->CreateBuffer(&bd, &initData, &buffer);

	if (SUCCEEDED(hr))
	{
		m_Data.m_Buffer.reset(buffer);
		m_Data.m_Stride = stride;
	}
	else
	{
		PG_CORE_ASSERT(false, "Failed to create vertex buffer");
	}
}

pig::Dx11VertexBuffer::~Dx11VertexBuffer()
{
	Unbind();
}

void pig::Dx11VertexBuffer::Bind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	const UINT offset = 0;
	const UINT stride = m_Data.m_Stride;
	ID3D11Buffer* buffer = m_Data.m_Buffer.get();
	context->GetPd3dDeviceContext()->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
}

void pig::Dx11VertexBuffer::Unbind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	const UINT offset = 0;
	const UINT stride = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	context->GetPd3dDeviceContext()->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
}

void pig::Dx11VertexBuffer::AppendVertices(const float* vertices, unsigned int count, unsigned int countOffset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	HRESULT hr = context->GetPd3dDeviceContext()->Map(m_Data.m_Buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(hr))
	{
		BYTE* destData = reinterpret_cast<BYTE*>(mappedResource.pData) + countOffset * m_Data.m_Stride;
		memcpy(destData, vertices, count * m_Data.m_Stride);
		context->GetPd3dDeviceContext()->Unmap(m_Data.m_Buffer.get(), 0);
	}
	else
	{
		PG_CORE_ASSERT(false, "Could not append vertexs");
	}
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::Dx11IndexBuffer::Dx11IndexBuffer(const uint32_t* indices, uint32_t count)
{
	m_Data.m_Count = count;
	D3D11_BUFFER_DESC ibd = { 0 };
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.ByteWidth = sizeof(DWORD) * count; // Number of indices
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA iinitData = { 0 };
	iinitData.pSysMem = indices;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	ID3D11Buffer* buffer = nullptr;
	HRESULT hr = context->GetPd3dDevice()->CreateBuffer(&ibd, &iinitData, &buffer);

	if (SUCCEEDED(hr))
	{
		m_Data.m_Buffer.reset(buffer);
	}
	else
	{
		PG_CORE_ASSERT(false, "Failed to create index buffer");
	}
}

pig::Dx11IndexBuffer::~Dx11IndexBuffer()
{
	Unbind();
}

void pig::Dx11IndexBuffer::Bind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetIndexBuffer(m_Data.m_Buffer.get(), DXGI_FORMAT_R32_UINT, 0);
}

void pig::Dx11IndexBuffer::Unbind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}

void pig::Dx11IndexBuffer::AppendIndices(const uint32_t* indices, unsigned int count, unsigned int countOffset)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	HRESULT hr = context->GetPd3dDeviceContext()->Map(m_Data.m_Buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(hr))
	{
		BYTE* destData = reinterpret_cast<BYTE*>(mappedResource.pData) + countOffset * sizeof(int);
		memcpy(destData, indices, count * sizeof(int));
		context->GetPd3dDeviceContext()->Unmap(m_Data.m_Buffer.get(), 0);
	}
	else
	{
		PG_CORE_ASSERT(false, "Could not append vertexs");
	}
}