#include "pch.h"
#include "Dx11Buffer.h"

#include "Pigeon/Application.h"
#include "Platform/DirectX11/Dx11Context.h"

/////////////////////////////////////////////////////////////////////////////
// VertexBuffer /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::Dx11VertexBuffer::Dx11VertexBuffer(float* vertices, uint32_t size)
{
	D3D11_BUFFER_DESC bd = { 0 };
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDevice()->CreateBuffer(&bd, &initData, &m_Data.m_Buffer);
}

pig::Dx11VertexBuffer::~Dx11VertexBuffer()
{
	Unbind();
	if (m_Data.m_Buffer)
		m_Data.m_Buffer->Release();
}

void pig::Dx11VertexBuffer::Bind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	const UINT offset = 0;
	const UINT stride = sizeof(float) * 7;
	context->GetPd3dDeviceContext()->IASetVertexBuffers(0, 1, &m_Data.m_Buffer, &stride, &offset);
}

void pig::Dx11VertexBuffer::Unbind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	const UINT offset = 0;
	const UINT stride = 0;
	ID3D11Buffer* nullBuffer = nullptr;
	context->GetPd3dDeviceContext()->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
}

/////////////////////////////////////////////////////////////////////////////
// IndexBuffer //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

pig::Dx11IndexBuffer::Dx11IndexBuffer(uint32_t* indices, uint32_t count)
{
	m_Data.m_Count = count;
	D3D11_BUFFER_DESC ibd = { 0 };
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(DWORD) * 3; // Number of indices
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData = { 0 };
	iinitData.pSysMem = indices;

	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDevice()->CreateBuffer(&ibd, &iinitData, &m_Data.m_Buffer);
}

pig::Dx11IndexBuffer::~Dx11IndexBuffer()
{
	Unbind();
	if (m_Data.m_Buffer)
		m_Data.m_Buffer->Release();
}

void pig::Dx11IndexBuffer::Bind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetIndexBuffer(m_Data.m_Buffer, DXGI_FORMAT_R32_UINT, 0);
}

void pig::Dx11IndexBuffer::Unbind() const
{
	auto context = static_cast<pig::Dx11Context*>(pig::Application::Get().GetWindow().GetGraphicsContext());
	context->GetPd3dDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
}