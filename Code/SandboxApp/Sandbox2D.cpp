#include "Pigeon.h"
#include "Pigeon/Core/Clock.h"
#include "Pigeon/Renderer/Font.h"
#include "Pigeon/Renderer/Sprite.h"
#include "Pigeon/UI/UIComponents.h"
#include "Sandbox/ConfigLoaderSystem.h"
#include "Sandbox/SampleUISystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

class Sandbox : public pg::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;
};

pg::Application& pg::CreateApplication()
{
	pg::Application& sandbox = Sandbox::Create();
	pg::World::Get().RegisterSystem(std::move(std::make_unique<sbx::ConfigLoaderSystem>()));
	pg::World::Get().RegisterSystem(std::move(std::make_unique<sbx::SampleUISystem>()));

	return sandbox;
}

#ifdef TESTS_ENABLED
// SandboxApp is not a Testing artifact (UT.exe is). Pigeon's EntryPoint omits the real main
// under TESTS_ENABLED, so provide a stub here purely so the Testing config links cleanly.
int main()
{
	return 0;
}
#endif