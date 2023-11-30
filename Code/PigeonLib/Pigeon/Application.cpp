#include "pch.h"

#include "Application.h"
#include "Input.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/Shader.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Log.h"

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


	float s_OurVertices[3 * 7] = {
		 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 0.45f, -0.5, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 -0.45f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
	};

	uint32_t s_Indices[3] = { 0, 1, 2 };
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

		m_VertexBuffer.reset(VertexBuffer::Create(s_OurVertices, sizeof(s_OurVertices)));
		m_IndexBuffer.reset(IndexBuffer::Create(s_Indices, sizeof(s_Indices) / sizeof(uint32_t)));

		BufferLayout buffLayout = {
			{ ShaderDataType::Float3, "POSITION" },
			{ ShaderDataType::Float4, "COLOR" }
		};

		m_Shader.reset(new Shader(s_VsCode, s_PsCode, buffLayout));

		m_Initialized = true;
	}

	Application::~Application()
	{
		m_ImGuiLayer->OnDetach();
		m_LayerStack.PopOverlay(m_ImGuiLayer);
		m_Shader.reset();
		m_Window.reset();

		m_VertexBuffer.reset();
		m_IndexBuffer.reset();

		s_Instance = nullptr;
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
		if (m_Initialized)
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
	}

#ifndef TESTS_ENABLED
	void Application::Run()
	{
		while (m_Running)
		{
			Update();
		}
	}
#endif

	void Application::Update()
	{
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });

		Renderer::BeginScene();
		
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
		m_Shader->Bind();

		Renderer::Submit();
		Renderer::EndScene();

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

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PG_CORE_ASSERT(m_Window, "Window not initialized");
		m_Window->SetSize(e.GetWidth(), e.GetHeight());
		return false;
	}
}