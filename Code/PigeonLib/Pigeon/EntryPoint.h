#pragma once

#ifdef PG_PLATFORM_WINDOWS

extern pigeon::Application* pigeon::CreateApplication();

int main(int argc, char** argv)
{
	pigeon::Log::Init();
	PG_CORE_WARN("Initialized Log!");
	int a = 5;
	PG_INFO("Hello! Var={0}", a);

	auto app = pigeon::CreateApplication();
	app->Run();
	delete app;
}

#endif