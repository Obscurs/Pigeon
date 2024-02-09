#pragma once

#include "Core.h"

#include "Window.h"
#include "Pigeon/Events/Event.h"
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/LayerStack.h"
#include "Pigeon/Renderer/Buffer.h"

namespace pig
{
	class ImGuiLayer;
	class Shader;
}

namespace pig
{
	class Application
	{
	public:
		struct Data
		{
			pig::S_Ptr<ImGuiLayer> m_ImGuiLayer;
			pig::U_Ptr<Window> m_Window;
			bool m_Running = true;
			bool m_Initialized = false;
			std::chrono::steady_clock::time_point m_LastFrameTime;
			LayerStack m_LayerStack;
			bool m_Minimized = false;
		};

		static pig::S_Ptr<Application> Create()
		{
			if (s_Instance)
			{
				s_Instance.reset();
			}
			if (!s_Instance)
			{
				s_Instance = std::make_shared<Application>();
				s_Instance->Init();
			}
			return s_Instance;
		}

		Application() = default;
		virtual ~Application();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
		void TestUpdate() { Update(); }
#else
		void Run();
#endif
		void OnEvent(Event& e);
		void PushLayer(pig::S_Ptr<Layer> layer);
		void PushOverlay(pig::S_Ptr<Layer> layer);

		inline Window& GetWindow() { return *m_Data.m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Init();

		void Update();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		Data m_Data;
	private:
		static pig::S_Ptr<Application> s_Instance;
	};

	// To be defined in CLIENT
	pig::S_Ptr<Application> CreateApplication();
}