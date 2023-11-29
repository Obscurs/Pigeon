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
	class PIGEON_API Application
	{
	public:
		Application();
		virtual ~Application();

#ifdef TESTS_ENABLED
		void TestUpdate() { Update(); }
#else
		void Run();
#endif
		void OnEvent(Event& e);
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }

	private:
		void Update();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ImGuiLayer* m_ImGuiLayer;
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		bool m_Initialized = false;
		LayerStack m_LayerStack;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;

		std::unique_ptr<Shader> m_Shader;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}