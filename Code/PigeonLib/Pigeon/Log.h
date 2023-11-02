#pragma once

#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"

namespace pigeon 
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
#define PG_CORE_TRACE(...)    ::pigeon::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PG_CORE_INFO(...)     ::pigeon::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PG_CORE_WARN(...)     ::pigeon::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PG_CORE_ERROR(...)    ::pigeon::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PG_CORE_FATAL(...)    ::pigeon::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define PG_TRACE(...)	      ::pigeon::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PG_INFO(...)	      ::pigeon::Log::GetClientLogger()->info(__VA_ARGS__)
#define PG_WARN(...)	      ::pigeon::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PG_ERROR(...)	      ::pigeon::Log::GetClientLogger()->error(__VA_ARGS__)
#define PG_FATAL(...)	      ::pigeon::Log::GetClientLogger()->fatal(__VA_ARGS__)