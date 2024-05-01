#include "pch.h"

#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

std::shared_ptr<spdlog::logger> pig::Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> pig::Log::s_ClientLogger;

void pig::Log::Init()
{
	if (!s_CoreLogger)
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("PGN");
		s_CoreLogger->set_level(spdlog::level::trace);
	}

	if (!s_ClientLogger)
	{
		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}
