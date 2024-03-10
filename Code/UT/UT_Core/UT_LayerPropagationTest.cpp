#pragma once
#include <catch.hpp>
#include <cstdlib>

#include "Utils/TestApp.h"

#include <Pigeon/Events/KeyEvent.h>
#include <Pigeon/Events/ApplicationEvent.h>
#include <Pigeon/Events/MouseEvent.h>

#include <Platform/Testing/TestingWindow.h>

namespace
{
	class TestLayerEventNoPropagate : public pig::Layer
	{
	public:
		TestLayerEventNoPropagate()
			: Layer("UTLayerEventNoPropagate")
		{
		}

		void OnUpdate(const pig::Timestep& ts) override
		{
			m_ReceivedEvent = false;
		}

		bool OnEvent(const pig::Event& event) override
		{
			//THIS IS TO TEST LAYER PROPAGATION
			if (auto e = dynamic_cast<const pig::KeyPressedEvent*>(&event)) {
				m_ReceivedEvent = true;
				return true;
			}
			return false;
		}
		bool m_ReceivedEvent = false;
	};

	class TestLayerEventPropagate : public pig::Layer
	{
	public:
		TestLayerEventPropagate()
			: Layer("UTLayerEventPropagate")
		{
		}
		void OnUpdate(const pig::Timestep& ts) override
		{
			m_ReceivedEvent = false;
		}

		bool OnEvent(const pig::Event& event) override
		{
			if (auto e = dynamic_cast<const pig::KeyPressedEvent*>(&event)) {
				m_ReceivedEvent = true;
			}
			return false;
		}
		bool m_ReceivedEvent = false;
	};
}

namespace CatchTestsetFail
{
	TEST_CASE("Core.Layers::Propagation")
	{
		pig::Application& app = pig::CreateApplication();
		pig::TestingWindow& appWindow = static_cast<pig::TestingWindow&>(app.GetWindow());
		int eventKeyCode = 123;
		int eventRepeat = 0;
		pig::KeyPressedEvent eventSent1(eventKeyCode, eventRepeat);
		const pig::LayerStack::Data& layerStackData = app.GetData().m_LayerStack.GetData();
		CHECK(layerStackData.m_LayerInsertIndex == 0);
		CHECK(layerStackData.m_Layers.size() == 0);
		pig::S_Ptr<TestLayerEventPropagate> testLayerPropagate1 = std::make_shared<TestLayerEventPropagate>();
		pig::S_Ptr<TestLayerEventPropagate> testLayerPropagate2 = std::make_shared < TestLayerEventPropagate>();
		pig::S_Ptr<TestLayerEventNoPropagate> testLayerNoPropagate1 = std::make_shared < TestLayerEventNoPropagate>();

		CHECK(!testLayerPropagate1->m_ReceivedEvent);
		CHECK(!testLayerPropagate2->m_ReceivedEvent);
		CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

		appWindow.TESTING_TriggerEvent(&eventSent1);

		CHECK(!testLayerPropagate1->m_ReceivedEvent);
		CHECK(!testLayerPropagate2->m_ReceivedEvent);
		CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

		SECTION("Test layer event reception")
		{
			app.PushLayer(testLayerPropagate1);

			CHECK(layerStackData.m_LayerInsertIndex == 1);
			CHECK(layerStackData.m_Layers.size() == 1);

			CHECK(!testLayerPropagate1->m_ReceivedEvent);
			CHECK(!testLayerPropagate2->m_ReceivedEvent);
			CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

			appWindow.TESTING_TriggerEvent(&eventSent1);
			CHECK(testLayerPropagate1->m_ReceivedEvent);
			CHECK(!testLayerPropagate2->m_ReceivedEvent);
			CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			app.TestUpdate();
			CHECK(!testLayerPropagate1->m_ReceivedEvent);

			SECTION("Two propagatorLayers")
			{
				app.PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 2);
				CHECK(layerStackData.m_Layers.size() == 2);

				appWindow.TESTING_TriggerEvent(&eventSent1);
				CHECK(testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);

				app.TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
			}
		}
		SECTION("Test layer no propagation")
		{
			SECTION("Two propagatorLayers and one no propagator at the beggining")
			{
				app.PushLayer(testLayerPropagate1);
				app.PushLayer(testLayerPropagate2);
				app.PushLayer(testLayerNoPropagate1);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 3);

				appWindow.TESTING_TriggerEvent(&eventSent1);
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app.TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
			SECTION("Two propagatorLayers and one no propagator at the end")
			{
				app.PushLayer(testLayerNoPropagate1);
				app.PushLayer(testLayerPropagate1);
				app.PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 3);

				appWindow.TESTING_TriggerEvent(&eventSent1);
				CHECK(testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app.TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
			SECTION("Two propagatorLayers and one no propagator at the middle")
			{
				app.PushLayer(testLayerPropagate1);
				app.PushLayer(testLayerNoPropagate1);
				app.PushLayer(testLayerPropagate2);
				CHECK(layerStackData.m_LayerInsertIndex == 3);
				CHECK(layerStackData.m_Layers.size() == 3);

				appWindow.TESTING_TriggerEvent(&eventSent1);
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(testLayerPropagate2->m_ReceivedEvent);
				CHECK(testLayerNoPropagate1->m_ReceivedEvent);

				app.TestUpdate();
				CHECK(!testLayerPropagate1->m_ReceivedEvent);
				CHECK(!testLayerPropagate2->m_ReceivedEvent);
				CHECK(!testLayerNoPropagate1->m_ReceivedEvent);
			}
		}
	}
} // End namespace: CatchTestsetFail

