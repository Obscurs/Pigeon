#include "pch.h"

#include "Application.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Log.h"

namespace pigeon 
{
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_Running)
		{
			m_Window->OnUpdate();
		}
	}
}