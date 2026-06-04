#pragma once

#ifdef PG_PLATFORM_WINDOWS

extern pg::Application& pg::CreateApplication();

#ifndef TESTS_ENABLED
int main(int argc, char** argv)
{
	pg::Log::Init();
	PG_CORE_WARN("Initialized Log!");

	auto& app = pg::CreateApplication();
	app.Run();
}
#endif

#endif