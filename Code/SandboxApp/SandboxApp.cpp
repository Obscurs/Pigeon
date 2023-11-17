#include <Pigeon.h>

class ExampleLayer : public pigeon::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		//PG_INFO("ExampleLayer::Update");
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
		PushOverlay(new pigeon::ImGuiLayer());
	}

	~Sandbox()
	{

	}
};

pigeon::Application* pigeon::CreateApplication()
{
	return new Sandbox();
}