#pragma once

#include "Core.h"

#include "Window.h"
#include "Pigeon/Events/Event.h"
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/LayerStack.h"
#include "Pigeon/Renderer/Buffer.h"

namespace pigeon
{
	class ImGuiLayer;
	class Shader;
}

namespace pigeon 
{
	class Application
	{
	public:
		struct Data
		{
			ImGuiLayer* m_ImGuiLayer;
			std::unique_ptr<Window> m_Window;
			bool m_Running = true;
			bool m_Initialized = false;
			LayerStack m_LayerStack;
		};

		Application();
		virtual ~Application();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
		void TestUpdate() { Update(); }
#else
		void Run();
#endif
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Data.m_Window; }

		inline static Application& Get() { return *s_Instance; }

	private:
		void Update();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		Data m_Data;
	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}