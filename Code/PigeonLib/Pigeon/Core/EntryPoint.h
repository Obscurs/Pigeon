#pragma once

#ifdef PG_PLATFORM_WINDOWS

extern pig::Application& pig::CreateApplication();

#ifndef TESTS_ENABLED
int main(int argc, char** argv)
{
	pig::Log::Init();
	PG_CORE_WARN("Initialized Log!");

	auto& app = pig::CreateApplication();
	app.Run();
}
#endif

#endif