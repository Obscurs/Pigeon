#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace pig 
{
	class PIGEON_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define PG_CORE_TRACE(...)    ::pig::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PG_CORE_INFO(...)     ::pig::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PG_CORE_WARN(...)     ::pig::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PG_CORE_ERROR(...)    ::pig::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PG_CORE_FATAL(...)    ::pig::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define PG_TRACE(...)	      ::pig::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PG_INFO(...)	      ::pig::Log::GetClientLogger()->info(__VA_ARGS__)
#define PG_WARN(...)	      ::pig::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PG_ERROR(...)	      ::pig::Log::GetClientLogger()->error(__VA_ARGS__)
#define PG_FATAL(...)	      ::pig::Log::GetClientLogger()->critical(__VA_ARGS__)