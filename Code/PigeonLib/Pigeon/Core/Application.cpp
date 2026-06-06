#include "pch.h"

#include "Pigeon/Core/Application.h"

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
#include "Pigeon/Core/ResourceManagerSystem.h"
#include "Pigeon/Core/WindowCloseEventComponent.h"
#include "Pigeon/Core/WindowResizeEventComponent.h"
#include "Pigeon/ECS/World.h"
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/ImGui/ImGuiLayer.h"
#include "Pigeon/Renderer/CameraSystem.h"
#include "Pigeon/Renderer/Renderer.h"
#include "Pigeon/Renderer/Renderer2DSystem.h"
#include "Pigeon/UI/UIControlSystem.h"
#include "Pigeon/UI/UIEventSystem.h"
#include "Pigeon/UI/UIRenderSystem.h"
#include <chrono>

pg::U_Ptr<pg::Application> pg::Application::s_Instance = nullptr;

pg::Application::~Application()
{
	Shutdown();
}

void pg::Application::PushLayer(pg::S_Ptr<pg::Layer> layer)
{
	m_Data.m_LayerStack.PushLayer(layer);
	layer->OnAttach();
}

void pg::Application::PushOverlay(pg::S_Ptr<pg::Layer> layer)
{
	m_Data.m_LayerStack.PushOverlay(layer);
	layer->OnAttach();
}

void pg::Application::OnEvent(const pg::Event& e)
{
	if (m_Data.m_Initialized)
	{
		if (e.GetEventType() == pg::WindowResizeEvent::GetStaticType())
		{
			OnWindowResize(dynamic_cast<const pg::WindowResizeEvent&>(e));
		}
		else if (e.GetEventType() == pg::WindowCloseEvent::GetStaticType())
		{
			OnWindowClose(dynamic_cast<const pg::WindowCloseEvent&>(e));
		}

		if (!m_Data.m_Minimized)
		{
			if (e.GetEventType() == pg::WindowResizeEvent::GetStaticType())
			{
				const pg::WindowResizeEvent& resolvedEvent = dynamic_cast<const pg::WindowResizeEvent&>(e);
				pg::WindowResizeEventComponent comp;
				comp.m_Height = resolvedEvent.GetHeight();
				comp.m_Width = resolvedEvent.GetWidth();
				pg::World::Get().EmplaceExternalEvent<pg::WindowResizeEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::WindowCloseEvent::GetStaticType())
			{
				const pg::WindowCloseEvent& resolvedEvent = dynamic_cast<const pg::WindowCloseEvent&>(e);
				pg::WindowCloseEventComponent comp;
				pg::World::Get().EmplaceExternalEvent<pg::WindowCloseEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::MouseButtonPressedEvent::GetStaticType())
			{
				const pg::MouseButtonPressedEvent& resolvedEvent = dynamic_cast<const pg::MouseButtonPressedEvent&>(e);
				pg::MouseButtonPressedEventComponent comp;
				comp.m_Button = resolvedEvent.GetMouseButton();
				pg::World::Get().EmplaceExternalEvent<pg::MouseButtonPressedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::MouseButtonReleasedEvent::GetStaticType())
			{
				const pg::MouseButtonReleasedEvent& resolvedEvent = dynamic_cast<const pg::MouseButtonReleasedEvent&>(e);
				pg::MouseButtonReleasedEventComponent comp;
				comp.m_Button = resolvedEvent.GetMouseButton();
				pg::World::Get().EmplaceExternalEvent<pg::MouseButtonReleasedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::MouseMovedEvent::GetStaticType())
			{
				const pg::MouseMovedEvent& resolvedEvent = dynamic_cast<const pg::MouseMovedEvent&>(e);
				pg::MouseMovedEventComponent comp;
				comp.m_MouseX = resolvedEvent.GetX();
				comp.m_MouseY = resolvedEvent.GetY();
				pg::World::Get().EmplaceExternalEvent<pg::MouseMovedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::MouseScrolledEvent::GetStaticType())
			{
				const pg::MouseScrolledEvent& resolvedEvent = dynamic_cast<const pg::MouseScrolledEvent&>(e);
				pg::MouseScrolledEventComponent comp;
				comp.m_XOffset = resolvedEvent.GetXOffset();
				comp.m_YOffset = resolvedEvent.GetYOffset();
				pg::World::Get().EmplaceExternalEvent<pg::MouseScrolledEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::KeyPressedEvent::GetStaticType())
			{
				const pg::KeyPressedEvent& resolvedEvent = dynamic_cast<const pg::KeyPressedEvent&>(e);
				pg::KeyPressedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pg::World::Get().EmplaceExternalEvent<pg::KeyPressedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::KeyReleasedEvent::GetStaticType())
			{
				const pg::KeyReleasedEvent& resolvedEvent = dynamic_cast<const pg::KeyReleasedEvent&>(e);
				pg::KeyReleasedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pg::World::Get().EmplaceExternalEvent<pg::KeyReleasedEventComponent>(std::move(comp));
			}
			else if (e.GetEventType() == pg::KeyTypedEvent::GetStaticType())
			{
				const pg::KeyTypedEvent& resolvedEvent = dynamic_cast<const pg::KeyTypedEvent&>(e);
				pg::KeyTypedEventComponent comp;
				comp.m_KeyCode = resolvedEvent.GetKeyCode();
				pg::World::Get().EmplaceExternalEvent<pg::KeyTypedEventComponent>(std::move(comp));
			}
			else
			{
				PG_CORE_ASSERT(false, "Event not implemented");
			}
		}
	}
}

#ifndef TESTS_ENABLED
void pg::Application::Run()
{
	while (m_Data.m_Running)
	{
		UpdateApp();
	}
}
#endif

void pg::Application::Shutdown()
{	
	m_Data.m_LayerStack.Shutdown();
}

void pg::Application::Init()
{
	m_Data.m_Window = std::move(Window::Create());
	m_Data.m_Window->SetEventCallback(pg::BindEventFn<&Application::OnEvent, Application>(this));
	pg::World& world = pg::World::Create();

	world.RegisterSystem(std::move(std::make_unique<pg::CameraSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::ConfigLoaderSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::InputSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::Renderer2DSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::ResourceManagerSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::ui::UIControlSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::ui::UIEventSystem>()));
	world.RegisterSystem(std::move(std::make_unique<pg::ui::UIRenderSystem>()));

	pg::Renderer::Init();

#ifndef TESTS_ENABLED
	m_Data.m_ImGuiLayer = std::make_shared<ImGuiLayer>();
	PushOverlay(m_Data.m_ImGuiLayer);
#endif
	m_Data.m_Initialized = true;
}

void pg::Application::UpdateApp()
{
	Update(m_Data.m_ClockFrameTime.Restart());
}

void pg::Application::Update(const Timestep& ts)
{
	m_Data.m_LastFrameTime = ts;
	for (pg::S_Ptr<pg::Layer> layer : m_Data.m_LayerStack)
		layer->Begin();

	for (pg::S_Ptr<pg::Layer> layer : m_Data.m_LayerStack)
		layer->OnUpdate(ts);

	pg::World::Get().Update(ts);

	for (pg::S_Ptr<pg::Layer> layer : m_Data.m_LayerStack)
		layer->End();

	m_Data.m_Window->OnUpdate();
}

bool pg::Application::OnWindowClose(const pg::WindowCloseEvent& e)
{
	m_Data.m_Running = false;
	return false;
}

bool pg::Application::OnWindowResize(const pg::WindowResizeEvent& e)
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