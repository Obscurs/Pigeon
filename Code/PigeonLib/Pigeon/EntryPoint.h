#pragma once

#ifdef PG_PLATFORM_WINDOWS

extern pigeon::Application* pigeon::CreateApplication();

int main(int argc, char** argv)
{
	auto app = pigeon::CreateApplication();
	app->Run();
	delete app;
}

#endif