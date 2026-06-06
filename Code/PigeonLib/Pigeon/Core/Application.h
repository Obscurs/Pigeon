#pragma once

#include "Pigeon/Core/Clock.h"
#include "Pigeon/Core/Core.h"
#include "Pigeon/Core/LayerStack.h"
#include "Pigeon/Core/Window.h"
#include "Pigeon/Events/ApplicationEvent.h"
#include "Pigeon/Events/Event.h"
#include "Pigeon/Renderer/Buffer.h"

namespace pg
{
	class ImGuiLayer;
	class InputLayer;
	class Shader;
}

namespace pg
{
	class Application
	{
	public:
		struct Data
		{
#ifndef TESTS_ENABLED
			pg::S_Ptr<ImGuiLayer> m_ImGuiLayer;
#endif
			pg::S_Ptr<InputLayer> m_InputLayer;
			pg::S_Ptr<Window> m_Window;
			bool m_Running = true;
			bool m_Initialized = false;
			Clock m_ClockUptime;
			Clock m_ClockFrameTime;
			Timestep m_LastFrameTime;
			LayerStack m_LayerStack;
			bool m_Minimized = false;
		};

		static Application& Create()
		{
			s_Instance = std::make_unique<Application>();
			s_Instance->Init();
			return s_Instance->Get();
		}

		Application() = default;
		virtual ~Application();

#ifdef TESTS_ENABLED
		const Data& GetData() const { return m_Data; }
		void TestUpdate(const Timestep& ts) { Update(ts); }
#else
		void Run();
#endif
		void OnEvent(const Event& e);
		void PushLayer(pg::S_Ptr<Layer> layer);
		void PushOverlay(pg::S_Ptr<Layer> layer);

		inline Window& GetWindow() { return *m_Data.m_Window; }
		inline InputLayer& GetInputLayer() { return *m_Data.m_InputLayer; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Shutdown();
		void Init();

		void UpdateApp();
		void Update(const Timestep& ts);

		bool OnWindowClose(const WindowCloseEvent& e);
		bool OnWindowResize(const WindowResizeEvent& e);

		Data m_Data;
	private:
		static pg::U_Ptr<Application> s_Instance;
	};

	// To be defined in CLIENT
	Application& CreateApplication();
}