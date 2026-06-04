#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace pg 
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
#define PG_CORE_TRACE(...)    ::pg::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PG_CORE_INFO(...)     ::pg::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PG_CORE_WARN(...)     ::pg::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PG_CORE_ERROR(...)    ::pg::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PG_CORE_FATAL(...)    ::pg::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define PG_TRACE(...)	      ::pg::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PG_INFO(...)	      ::pg::Log::GetClientLogger()->info(__VA_ARGS__)
#define PG_WARN(...)	      ::pg::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PG_ERROR(...)	      ::pg::Log::GetClientLogger()->error(__VA_ARGS__)
#define PG_FATAL(...)	      ::pg::Log::GetClientLogger()->critical(__VA_ARGS__)