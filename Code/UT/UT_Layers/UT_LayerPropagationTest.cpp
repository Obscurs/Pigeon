#pragma once
#include <catch.hpp>
#include <cstdlib>
#include <variant>

#include "Utils/TestApp.h"

#include <Pigeon/Events/KeyEvent.h>
#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>

namespace
{
	class TestLayerEventNoPropagate : public pigeon::Layer
	{
	public:
		TestLayerEventNoPropagate()
			: Layer("UTLayerEventNoPropagate")
		{
		}

		void OnUpdate() override
		{
			m_ReceivedEvent = false;
		}

		void OnEvent(pigeon::Event& event) override
		{

			//THIS IS TO TEST LAYER PROPAGATION
			if (auto e = dynamic_cast<pigeon::KeyPressedEvent*>(&event)) {
				m_ReceivedEvent = true;
				event.Handled = true;
			}
		}
		bool m_ReceivedEvent = false;
	};

	class TestLayerEventPropagate : public pigeon::Layer
	{
	public:
		TestLayerEventPropagate()
			: Layer("UTLayerEventPropagate")
		{
		}
		void OnUpdate() override
		{
			m_ReceivedEvent = false;
		}

		void OnEvent(pigeon::Event& event) override
		{
			if (auto e = dynamic_cast<pigeon::KeyPressedEvent*>(&event)) {
				m_ReceivedEvent = true;
			}
		}
		bool m_ReceivedEvent = false;
	};
}

namespace CatchTestsetFail
{
	TEST_CASE("app.Layers::Propagation")
	{
		TestApp* app = static_cast<TestApp*>(pigeon::CreateApplication());
		
		int wParam = 123;
		int lParam = 456;

		const pigeon::LayerStack::Data& layerStackData = app->GetData().m_LayerStack.GetData();
		CHECK(layerStackData.m_LayerInsertIndex == 0);
		CHECK(layerStackData.m_Layers.size() == 1);
		TestLayerEventPropagate* testLayerPropagate1 = new TestLayerEventPropagate();
		TestLayerEventPropagate* testLayerPropagate2 = new TestLayerEventPropagate();
		TestLayerEventNoPropagate* testLayerNoPropagate1 = new TestLayerEventNoPropagate();
			
		CHECK(!testLayerPropagate1->m_ReceivedEvent);
		CHECK(!testLayerPropagate2->m_ReceivedEvent);
		CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

		app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);

		CHECK(!testLayerPropagate1->m_ReceivedEvent);
		CHECK(!testLayerPropagate2->m_ReceivedEvent);
		CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

		SECTION("Test layer event reception")
		{
			app->PushLayer(testLayerPropagate1);

			CHECK(layerStackData.m_LayerInsertIndex == 1);
			CHECK(layerStackData.m_Layers.size() == 2);

			CHECK(!testLayerPropagate1->m_ReceivedEvent);
			CHECK(!testLayerPropagate2->m_ReceivedEvent);
			CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

			app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
			CHECK(testLayerPropagate1->m_ReceivedEvent);
			CHECK(!testLayerPropagate2->m_ReceivedEvent);
			CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			app->TestUpdate();
			CHECK(!testLayerPropagate1->m_ReceivedEvent);

			SECTION("Two propagatorLayers")
			{
				app->PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 2);
				CHECK(layerStackData.m_Layers.size() == 3);

				app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
				CHECK(testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

				app->TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
			}
		}
		SECTION("Test layer no propagation")
		{
			SECTION("Two propagatorLayers and one no propagator at the beggining")
			{
				app->PushLayer(testLayerPropagate1);
				app->PushLayer(testLayerPropagate2);
				app->PushLayer(testLayerNoPropagate1);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 4);

				app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app->TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
			SECTION("Two propagatorLayers and one no propagator at the end")
			{
				app->PushLayer(testLayerNoPropagate1);
				app->PushLayer(testLayerPropagate1);
				app->PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 4);

				app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
				CHECK(testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app->TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
			SECTION("Two propagatorLayers and one no propagator at the middle")
			{
				app->PushLayer(testLayerPropagate1);
				app->PushLayer(testLayerNoPropagate1);
				app->PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 4);

				app->SendFakeEvent(pigeon::WindowsWindow::EventType::KEYDOWN, wParam, lParam);
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app->TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
		}
		delete app;
	}
} // End namespace: CatchTestsetFail

