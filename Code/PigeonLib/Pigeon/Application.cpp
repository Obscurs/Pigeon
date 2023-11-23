#include "pch.h"

#include "Application.h"
#include "Input.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Log.h"

#include "Platform/DirectX11/Dx11Context.h"

namespace
{
	// Simple vertex shader
	char* s_VsCode =
		"struct VS_INPUT\n"
		"{\n"
		"	float4 Pos : POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION; \n"
		"	float4 Col : COLOR; \n"
		"};\n"
		"PS_INPUT main(VS_INPUT input)\n"
		"{\n"
		"	PS_INPUT output;\n"
		"	output.Pos = input.Pos; // Pass position to rasterizer\n"
		"	output.Col = input.Col; // Pass color to pixel shader\n"
		"	return output;\n"
		"};";

	// Simple pixel shader
	char* s_PsCode =
		"struct PS_INPUT\n"
		"{\n"
		"	float4 Pos : SV_POSITION;\n"
		"	float4 Col : COLOR;\n"
		"};\n"
		"float4 main(PS_INPUT input) : SV_TARGET\n"
		"{\n"
		"    return input.Col; // Output the interpolated color\n"
		"};";

	struct VERTEX {
		FLOAT X, Y, Z;
		FLOAT Color[4];
	};

	VERTEX s_OurVertices[] = {
		{ 0.0f, 0.5f, 0.0f, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ 0.45f, -0.5, 0.0f, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ -0.45f, -0.5f, 0.0f, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	DWORD s_Indices[] = { 0, 1, 2 };
}

namespace pigeon 
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		PG_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		D3D11_BUFFER_DESC bd = { 0 };
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = s_OurVertices;

		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		context->GetPd3dDevice()->CreateBuffer(&bd, &initData, &m_VertexBuffer);

		D3D11_BUFFER_DESC ibd = { 0 };
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = sizeof(DWORD) * 3; // Number of indices
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData = { 0 };
		iinitData.pSysMem = s_Indices;

		context->GetPd3dDevice()->CreateBuffer(&ibd, &iinitData, &m_IndexBuffer);

		m_Shader.reset(new Shader(s_VsCode, s_PsCode));

	}

	Application::~Application()
	{
		if (m_IndexBuffer) {
			m_IndexBuffer->Release();
			m_IndexBuffer = nullptr;
		}
		if (m_VertexBuffer) {
			m_VertexBuffer->Release();
			m_VertexBuffer = nullptr;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->OnBegin();

			//TODO move this wherever
			UINT stride = sizeof(VERTEX);
			UINT offset = 0;
			auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
			context->GetPd3dDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
			context->GetPd3dDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			context->GetPd3dDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_Shader->Bind();
			context->GetPd3dDeviceContext()->DrawIndexed(3, 0, 0);

			for (Layer* layer : m_LayerStack)
				layer->Begin();

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			if (m_ImGuiLayer->IsAttached())
			{
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}

			for (Layer* layer : m_LayerStack)
				layer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		//TODO do directly in the context layer???
		auto context = static_cast<Dx11Context*>(Application::Get().GetWindow().GetGraphicsContext());
		context->SetSize(e.GetWidth(), e.GetHeight());
		return true;
	}
}