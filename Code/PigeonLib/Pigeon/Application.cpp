#include "pch.h"

#include "Application.h"
#include "Input.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Log.h"

#include <chrono>

#define BIND_EVENT_FN(x) std::bind(&pig::Application::x, this, std::placeholders::_1)

pig::S_Ptr<pig::Application> pig::Application::s_Instance = nullptr;

pig::Application::~Application()
{
	m_Data.m_ImGuiLayer->OnDetach();
	m_Data.m_LayerStack.PopOverlay(m_Data.m_ImGuiLayer);
	m_Data.m_LayerStack.Shutdown();

	m_Data.m_Window.reset();
	s_Instance = nullptr;
}

void pig::Application::PushLayer(pig::Layer* layer)
{
	m_Data.m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void pig::Application::PushOverlay(pig::Layer* layer)
{
	m_Data.m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void pig::Application::OnEvent(pig::Event& e)
{
	if (m_Data.m_Initialized)
	{
		pig::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<pig::WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<pig::WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_Data.m_LayerStack.end(); it != m_Data.m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
}

#ifndef TESTS_ENABLED
void pig::Application::Run()
{
	while (m_Data.m_Running)
	{
		Update();
	}
}
#endif

void pig::Application::Init()
{
	m_Data.m_Window = std::unique_ptr<Window>(Window::Create());
	m_Data.m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

	m_Data.m_ImGuiLayer = new ImGuiLayer();
	PushOverlay(m_Data.m_ImGuiLayer);

	m_Data.m_Initialized = true;
}

void pig::Application::Update()
{
	auto currentTime = std::chrono::steady_clock::now();
	pig::Timestep timestep = std::chrono::duration<float>(currentTime - m_Data.m_LastFrameTime).count();
	m_Data.m_LastFrameTime = currentTime;

	for (pig::Layer* layer : m_Data.m_LayerStack)
		layer->Begin();

	for (pig::Layer* layer : m_Data.m_LayerStack)
		layer->OnUpdate(timestep);

	if (m_Data.m_ImGuiLayer->IsAttached())
	{
		for (pig::Layer* layer : m_Data.m_LayerStack)
			layer->OnImGuiRender();
	}

	for (pig::Layer* layer : m_Data.m_LayerStack)
		layer->End();

	m_Data.m_Window->OnUpdate();
}

bool pig::Application::OnWindowClose(pig::WindowCloseEvent& e)
{
	m_Data.m_Running = false;
	return false;
}

bool pig::Application::OnWindowResize(pig::WindowResizeEvent& e)
{
	PG_CORE_ASSERT(m_Data.m_Window, "Window not initialized");
	m_Data.m_Window->SetSize(e.GetWidth(), e.GetHeight());
	return false;
}