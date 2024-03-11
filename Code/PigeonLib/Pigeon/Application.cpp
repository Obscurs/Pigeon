#include "pch.h"

#include "Application.h"
#include "Input.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Log.h"
#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/Renderer2D.h"

#include <chrono>

pig::U_Ptr<pig::Application> pig::Application::s_Instance = nullptr;

pig::Application::~Application()
{
	Shutdown();
}

void pig::Application::PushLayer(pig::S_Ptr<pig::Layer> layer)
{
	m_Data.m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void pig::Application::PushOverlay(pig::S_Ptr<pig::Layer> layer)
{
	m_Data.m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void pig::Application::OnEvent(const pig::Event& e)
{
	if (m_Data.m_Initialized)
	{
		pig::EventDispatcher::Dispatch<pig::WindowCloseEvent>(e, pig::BindEventFn<&Application::OnWindowClose, Application>(this));
		pig::EventDispatcher::Dispatch<pig::WindowResizeEvent>(e, pig::BindEventFn<&Application::OnWindowResize, Application>(this));

		if (!m_Data.m_Minimized)
		{
			for (auto it = m_Data.m_LayerStack.rbegin(); it != m_Data.m_LayerStack.rend(); it++)
			{
				bool handled = (*it)->OnEvent(e);
				if (handled)
				{
					break;
				}
			}
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

void pig::Application::Shutdown()
{	
	m_Data.m_LayerStack.Shutdown();
	pig::Renderer2D::Destroy();
}

void pig::Application::Init()
{
	m_Data.m_Window = std::move(Window::Create());
	m_Data.m_Window->SetEventCallback(pig::BindEventFn<&Application::OnEvent, Application>(this));
	pig::Renderer::Init();
	pig::Renderer2D::Init();
#ifndef TESTS_ENABLED
	m_Data.m_ImGuiLayer = std::make_shared<ImGuiLayer>();
	PushOverlay(m_Data.m_ImGuiLayer);
#endif
	m_Data.m_Initialized = true;
}

void pig::Application::Update()
{
	const Timestep deltaTime = m_Data.m_ClockFrameTime.Restart();
	m_Data.m_LastFrameTime = deltaTime;

	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->Begin();

	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->OnUpdate(deltaTime);
#ifndef TESTS_ENABLED
	if (m_Data.m_ImGuiLayer->IsAttached())
	{
		for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
			layer->OnImGuiRender();
	}
#endif
	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->End();

	m_Data.m_Window->OnUpdate();
}

bool pig::Application::OnWindowClose(const pig::WindowCloseEvent& e)
{
	pig::Renderer2D::Destroy();
	m_Data.m_Running = false;
	return false;
}

bool pig::Application::OnWindowResize(const pig::WindowResizeEvent& e)
{
	if (e.GetWidth() == 0 || e.GetHeight() == 0)
	{
		m_Data.m_Minimized = true;
		return false;
	}

	m_Data.m_Minimized = false;

	PG_CORE_ASSERT(m_Data.m_Window, "Window not initialized");
	Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
	return false;
}