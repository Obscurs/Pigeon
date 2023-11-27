#pragma once

#ifdef PG_PLATFORM_WINDOWS

extern pigeon::Application* pigeon::CreateApplication();

#ifndef TESTS_ENABLED
int main(int argc, char** argv)
{
	pigeon::Log::Init();
	PG_CORE_WARN("Initialized Log!");

	auto app = pigeon::CreateApplication();
	app->Run();
	delete app;
}
#endif

#endif