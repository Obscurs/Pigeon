#include "Pigeon.h"
#include "Pigeon/ECS/World.h"
#include "Sandbox/AudioDebugPanelSystem.h"
#include "Sandbox/AudioDemoSystem.h"
#include "Sandbox/CameraControlSystem.h"
#include "Sandbox/ConfigLoaderSystem.h"
#include "Sandbox/DebugPanelSystem.h"
#include "Sandbox/InputReadoutSystem.h"
#include "Sandbox/LifetimeSystem.h"
#include "Sandbox/QuadAnimationSystem.h"
#include "Sandbox/QuadRenderSystem.h"
#include "Sandbox/QuadSpawnSystem.h"
#include "Sandbox/SaveDataDebugPanelSystem.h"
#include "Sandbox/SceneSetupSystem.h"
#include "Sandbox/SpriteRenderSystem.h"
#include "Sandbox/TextRenderSystem.h"
#include "Sandbox/TransformResolveSystem.h"
#include "Sandbox/UIButtonSystem.h"
#include "Sandbox/UIButtonVisualSystem.h"
#include "Sandbox/UICloseSystem.h"
#include "Sandbox/UIScrollSystem.h"
#include "Sandbox/UIStatusSystem.h"
#include "Sandbox/WindowDebugPanelSystem.h"

class Sandbox : public pg::Application
{
public:
	Sandbox() = default;
	~Sandbox() = default;
};

pg::Application& pg::CreateApplication()
{
	pg::Application& sandbox = Sandbox::Create();
	pg::World& world = pg::World::Get();

	// Engine systems (camera, input, resources, renderer, UI) are registered by Application::Init.
	// The sandbox registers only its own systems; execution order is derived automatically from
	// each system's DeclareAccess(), so registration order here does not matter.
	world.RegisterSystem(std::make_unique<sbx::AudioDemoSystem>());
	world.RegisterSystem(std::make_unique<sbx::AudioDebugPanelSystem>());
	world.RegisterSystem(std::make_unique<sbx::CameraControlSystem>());
	world.RegisterSystem(std::make_unique<sbx::ConfigLoaderSystem>());
	world.RegisterSystem(std::make_unique<sbx::TransformResolveSystem>());
	world.RegisterSystem(std::make_unique<sbx::SceneSetupSystem>());
	world.RegisterSystem(std::make_unique<sbx::QuadSpawnSystem>());
	world.RegisterSystem(std::make_unique<sbx::QuadAnimationSystem>());
	world.RegisterSystem(std::make_unique<sbx::LifetimeSystem>());
	world.RegisterSystem(std::make_unique<sbx::QuadRenderSystem>());
	world.RegisterSystem(std::make_unique<sbx::SpriteRenderSystem>());
	world.RegisterSystem(std::make_unique<sbx::TextRenderSystem>());
	world.RegisterSystem(std::make_unique<sbx::InputReadoutSystem>());
	world.RegisterSystem(std::make_unique<sbx::UIButtonVisualSystem>());
	world.RegisterSystem(std::make_unique<sbx::UIButtonSystem>());
	world.RegisterSystem(std::make_unique<sbx::UICloseSystem>());
	world.RegisterSystem(std::make_unique<sbx::UIScrollSystem>());
	world.RegisterSystem(std::make_unique<sbx::UIStatusSystem>());
	world.RegisterSystem(std::make_unique<sbx::DebugPanelSystem>());
	world.RegisterSystem(std::make_unique<sbx::WindowDebugPanelSystem>());
	world.RegisterSystem(std::make_unique<sbx::SaveDataDebugPanelSystem>());

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
