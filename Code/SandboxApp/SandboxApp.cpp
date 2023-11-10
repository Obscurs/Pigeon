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
		PG_INFO("ExampleLayer::Update");
	}

	void OnEvent(pigeon::Event& event) override
	{
		PG_TRACE("{0}", event);
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