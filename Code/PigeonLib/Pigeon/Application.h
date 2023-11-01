#pragma once

#include "Core.h"

namespace pigeon 
{
	class PIGEON_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}