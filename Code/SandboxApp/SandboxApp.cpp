#include <Pigeon.h>

#include "imgui/imgui.h"

class ExampleLayer : public pigeon::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		if (pigeon::Input::IsKeyPressed(PG_KEY_TAB))
			PG_TRACE("Tab key is pressed (poll)!");
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}

	void OnEvent(pigeon::Event& event) override
	{
		if (event.GetEventType() == pigeon::EventType::KeyPressed)
		{
			pigeon::KeyPressedEvent& e = (pigeon::KeyPressedEvent&)event;
			if (e.GetKeyCode() == PG_KEY_TAB)
				PG_TRACE("Tab key is pressed (event)!");
			PG_TRACE("{0}", (char)e.GetKeyCode());
		}
	}

};
class Sandbox : public pigeon::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

pigeon::Application* pigeon::CreateApplication()
{
	return new Sandbox();
}