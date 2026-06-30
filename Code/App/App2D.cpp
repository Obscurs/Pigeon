#include "Pigeon.h"

// The minimal Pigeon application. Like SandboxApp it defines pg::CreateApplication (the single client
// entry point the engine's EntryPoint calls), but registers no app-specific systems: it relies entirely
// on the engine systems registered by Application::Init (window, input, resources, renderer, UI). Its
// assets live under Data/Assets/App, which is the engine default app-assets folder, so it does not call
// SetAppAssetsFolder. Add app systems here as the application grows.
class App : public pg::Application
{
public:
	App() = default;
	~App() = default;
};

pg::Application& pg::CreateApplication()
{
	return App::Create();
}

#ifdef TESTS_ENABLED
// App is not a Testing artifact (UT.exe is). Pigeon's EntryPoint omits the real main under TESTS_ENABLED,
// so provide a stub here purely so the Testing config links cleanly.
int main()
{
	return 0;
}
#endif
