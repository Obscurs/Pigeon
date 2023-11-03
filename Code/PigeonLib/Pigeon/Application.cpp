#include "pch.h"

#include "Application.h"

#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Log.h"

namespace pigeon 
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			PG_TRACE(e);
		}
		if (e.IsInCategory(EventCategoryInput))
		{
			PG_TRACE(e);
		}

		while (true);
	}
}