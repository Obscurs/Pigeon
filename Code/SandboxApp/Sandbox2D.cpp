#include <Pigeon.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/UI/UIComponents.h"
#include "Pigeon/Core/Clock.h"
#include "Sandbox/ConfigLoaderSystem.h"
#include "Sandbox/SampleUISystem.h"

class Sandbox : public pig::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;
};

pig::Application& pig::CreateApplication()
{
	pig::Application& sandbox = Sandbox::Create();
	pig::World::Get().RegisterSystem(std::move(std::make_unique<sbx::ConfigLoaderSystem>()));
	pig::World::Get().RegisterSystem(std::move(std::make_unique<sbx::SampleUISystem>()));
	
	return sandbox;
}