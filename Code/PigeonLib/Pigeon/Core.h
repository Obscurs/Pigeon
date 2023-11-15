#pragma once

#ifdef PG_PLATFORM_WINDOWS
	#ifdef PG_BUILD_DLL
		#define PIGEON_API __declspec(dllexport)
	#else
		#define PIGEON_API __declspec(dllimport)
	#endif
#else
	#error Pigeon only supports Windows!
#endif

#ifdef PG_ENABLE_ASSERTS
	#define PG_ASSERT(x, ...) { if(!(x)) { PG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define PG_CORE_ASSERT(x, ...) { if(!(x)) { PG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define PG_ASSERT(x, ...)
	#define PG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define PG_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)