#include <Pigeon.h>

class Sandbox : public pigeon::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

pigeon::Application* pigeon::CreateApplication()
{
	return new Sandbox();
}