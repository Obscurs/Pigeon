#include "pch.h"

#include "Application.h"

#include "Pigeon/Core/CameraSystem.h"
#include "Pigeon/Core/ConfigLoaderSystem.h"
#include "Pigeon/Core/InputSystem.h"
#include "Pigeon/Core/KeyPressedEventComponent.h"
#include "Pigeon/Core/KeyReleasedEventComponent.h"
#include "Pigeon/Core/KeyTypedEventComponent.h"
#include "Pigeon/Core/Log.h"
#include "Pigeon/Core/MouseButtonPressedEventComponent.h"
#include "Pigeon/Core/MouseButtonReleasedEventComponent.h"
#include "Pigeon/Core/MouseMovedEventComponent.h"
#include "Pigeon/Core/MouseScrolledEventComponent.h"
#include "Pigeon/Core/OrthographicCameraComponent.h"
#include "Pigeon/Core/ResourceManagerSystem.h"
#include "Pigeon/Core/WindowCloseEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/Renderer2DSystem.h"
#include "Pigeon/UI/UIControlSystem.h"
#include "Pigeon/UI/UIEventSystem.h"
#include "Pigeon/UI/UIRenderSystem.h"
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
		if (e.GetEventType() == pig::WindowResizeEvent::GetStaticType())
		{
			OnWindowResize(dynamic_cast<const pig::WindowResizeEvent&>(e));
		}
		else if (e.GetEventType() == pig::WindowCloseEvent::GetStaticType())
		{
			OnWindowClose(dynamic_cast<const pig::WindowCloseEvent&>(e));
		}

		if (!m_Data.m_Minimized)
		{
			if (e.GetEventType() == pig::WindowResizeEvent::GetStaticType())
			{
				const pig::WindowResizeEvent& resolvedEvent = dynamic_cast<const pig::WindowResizeEvent&>(e);
				pig::WindowResizeEventComponent comp;
				comp.m_Height = resolvedEvent.GetHeight();
				comp.m_Width = resolvedEvent.GetWidth();
				pig::World::Get().EmplaceExternalEvent<pig::WindowResizeEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::WindowCloseEvent::GetStaticType())
			{
				const pig::WindowCloseEvent& resolvedEvent = dynamic_cast<const pig::WindowCloseEvent&>(e);
				pig::WindowCloseEventComponent comp;
				pig::World::Get().EmplaceExternalEvent<pig::WindowCloseEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::MouseButtonPressedEvent::GetStaticType())
			{
				const pig::MouseButtonPressedEvent& resolvedEvent = dynamic_cast<const pig::MouseButtonPressedEvent&>(e);
				pig::MouseButtonPressedEventComponent comp;
				comp.m_Button = resolvedEvent.GetMouseButton();
				pig::World::Get().EmplaceExternalEvent<pig::MouseButtonPressedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::MouseButtonReleasedEvent::GetStaticType())
			{
				const pig::MouseButtonReleasedEvent& resolvedEvent = dynamic_cast<const pig::MouseButtonReleasedEvent&>(e);
				pig::MouseButtonReleasedEventComponent comp;
				comp.m_Button = resolvedEvent.GetMouseButton();
				pig::World::Get().EmplaceExternalEvent<pig::MouseButtonReleasedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::MouseMovedEvent::GetStaticType())
			{
				const pig::MouseMovedEvent& resolvedEvent = dynamic_cast<const pig::MouseMovedEvent&>(e);
				pig::MouseMovedEventComponent comp;
				comp.m_MouseX = resolvedEvent.GetX();
				comp.m_MouseY = resolvedEvent.GetY();
				pig::World::Get().EmplaceExternalEvent<pig::MouseMovedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::MouseScrolledEvent::GetStaticType())
			{
				const pig::MouseScrolledEvent& resolvedEvent = dynamic_cast<const pig::MouseScrolledEvent&>(e);
				pig::MouseScrolledEventComponent comp;
				comp.m_XOffset = resolvedEvent.GetXOffset();
				comp.m_YOffset = resolvedEvent.GetYOffset();
				pig::World::Get().EmplaceExternalEvent<pig::MouseScrolledEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::KeyPressedEvent::GetStaticType())
			{
				const pig::KeyPressedEvent& resolvedEvent = dynamic_cast<const pig::KeyPressedEvent&>(e);
				pig::KeyPressedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pig::World::Get().EmplaceExternalEvent<pig::KeyPressedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::KeyReleasedEvent::GetStaticType())
			{
				const pig::KeyReleasedEvent& resolvedEvent = dynamic_cast<const pig::KeyReleasedEvent&>(e);
				pig::KeyReleasedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pig::World::Get().EmplaceExternalEvent<pig::KeyReleasedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pig::KeyTypedEvent::GetStaticType())
			{
				const pig::KeyTypedEvent& resolvedEvent = dynamic_cast<const pig::KeyTypedEvent&>(e);
				pig::KeyTypedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pig::World::Get().EmplaceExternalEvent<pig::KeyTypedEventComponent>(std::move(comp));
			}
			else
			{
				PG_CORE_ASSERT(false, "Event not implemented");
			}
		}
	}
}

#ifndef TESTS_ENABLED
void pig::Application::Run()
{
	while (m_Data.m_Running)
	{
		UpdateApp();
	}
}
#endif

void pig::Application::Shutdown()
{	
	m_Data.m_LayerStack.Shutdown();
}

void pig::Application::Init()
{
	m_Data.m_Window = std::move(Window::Create());
	m_Data.m_Window->SetEventCallback(pig::BindEventFn<&Application::OnEvent, Application>(this));
	pig::World& world = pig::World::Create();

	world.RegisterSystem(std::move(std::make_unique<pig::CameraSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::ConfigLoaderSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::InputSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::Renderer2DSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::ResourceManagerSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::ui::UIControlSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::ui::UIEventSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pig::ui::UIRenderSystem>(std::make_shared<pig::ui::UIRenderSystemHelper>())));

	pig::Renderer::Init();

#ifndef TESTS_ENABLED
	m_Data.m_ImGuiLayer = std::make_shared<ImGuiLayer>();
	PushOverlay(m_Data.m_ImGuiLayer);
#endif
	m_Data.m_Initialized = true;
}

void pig::Application::UpdateApp()
{
	Update(m_Data.m_ClockFrameTime.Restart());
}

void pig::Application::Update(const Timestep& ts)
{
	m_Data.m_LastFrameTime = ts;
	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->Begin();

	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->OnUpdate(ts);

	pig::World::Get().Update(ts);

	for (pig::S_Ptr<pig::Layer> layer : m_Data.m_LayerStack)
		layer->End();

	m_Data.m_Window->OnUpdate();
}

bool pig::Application::OnWindowClose(const pig::WindowCloseEvent& e)
{
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