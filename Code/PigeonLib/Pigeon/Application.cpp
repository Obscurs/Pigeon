#include "pch.h"

#include "Application.h"
#include "Input.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Log.h"

namespace pigeon 
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		PG_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Data.m_Window = std::unique_ptr<Window>(Window::Create());
		m_Data.m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_Data.m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_Data.m_ImGuiLayer);

		m_Data.m_Initialized = true;
	}

	Application::~Application()
	{
		m_Data.m_ImGuiLayer->OnDetach();
		m_Data.m_LayerStack.PopOverlay(m_Data.m_ImGuiLayer);
		m_Data.m_LayerStack.Shutdown();

		m_Data.m_Window.reset();
		s_Instance = nullptr;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_Data.m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_Data.m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		if (m_Data.m_Initialized)
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
			dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

			for (auto it = m_Data.m_LayerStack.end(); it != m_Data.m_LayerStack.begin(); )
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
		while (m_Data.m_Running)
		{
			Update();
		}
	}
#endif

	void Application::Update()
	{
		for (Layer* layer : m_Data.m_LayerStack)
			layer->Begin();

		for (Layer* layer : m_Data.m_LayerStack)
			layer->OnUpdate();

		if (m_Data.m_ImGuiLayer->IsAttached())
		{
			for (Layer* layer : m_Data.m_LayerStack)
				layer->OnImGuiRender();
		}

		for (Layer* layer : m_Data.m_LayerStack)
			layer->End();

		m_Data.m_Window->OnUpdate();
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Data.m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		PG_CORE_ASSERT(m_Data.m_Window, "Window not initialized");
		m_Data.m_Window->SetSize(e.GetWidth(), e.GetHeight());
		return false;
	}
}