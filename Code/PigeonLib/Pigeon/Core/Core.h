#pragma once
#define PG_ENABLE_ASSERTS

#ifdef PG_PLATFORM_WINDOWS
	#define NOMINMAX
	#if PG_DYNAMIC_LINK
		#ifdef PG_BUILD_DLL
			#define PIGEON_API __declspec(dllexport)
		#else
			#define PIGEON_API __declspec(dllimport)
		#endif
	#else
		#define PIGEON_API
	#endif
#else
	#error Pigeon only supports Windows!
#endif

#ifndef TESTS_ENABLED
#define PG_DEBUGBREAK __debugbreak();
#else
#define PG_DEBUGBREAK
#endif
#ifdef PG_ENABLE_ASSERTS
	#define PG_ASSERT(x, ...) { if(!(x)) { PG_ERROR("Assertion Failed: {0}", __VA_ARGS__); PG_DEBUGBREAK } }
	#define PG_CORE_ASSERT(x, ...) { if(!(x)) { PG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PG_DEBUGBREAK } }
	#define PG_CORE_EXCEPT(x, ...) { if(!(x)) { PG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PG_DEBUGBREAK throw std::invalid_argument(__VA_ARGS__);} }
#else
	#define PG_ASSERT(x, ...)
	#define PG_CORE_ASSERT(x, ...)
#endif

namespace pig
{
	constexpr int BIT(int x) noexcept
	{
		return 1 << x;
	}

	template<typename T, typename Deleter = std::default_delete<T>>
	using U_Ptr = std::unique_ptr<T, Deleter>;

	template<typename T>
	using S_Ptr = std::shared_ptr<T>;

	//ARNAU TODO Move this funcs elsewhere?
	struct ReleaseDeleter {
		template<typename T>
		void operator()(T* ptr) const {
			if (ptr) ptr->Release();
		}
	};

	template<typename T, typename Deleter = std::default_delete<T>>
	std::function<T** (std::unique_ptr<T, Deleter>&)> U_PtrToPtr = [](std::unique_ptr<T, Deleter>& uniquePtr) {
		static T* rawPtr = nullptr;
		rawPtr = uniquePtr.get();
		return &rawPtr;
		};

	template <typename T>
	class RAII_Ptr {
	public:
		RAII_Ptr() : value(nullptr) {}
		RAII_Ptr(T* val) : value(val) {}
		~RAII_Ptr() { if(value) delete value;}
		T* value;
	};

	template <typename T>
	class RAII_PtrRelease {
	public:
		RAII_PtrRelease() : value(nullptr) {}
		RAII_PtrRelease(T* val) : value(val) {}
		~RAII_PtrRelease() { if (value) value->Release(); }
		T* value;
	};

	template <auto EventFn, typename EventHandler>
	auto BindEventFn(EventHandler* handler) 
	{
		return [handler](auto&&... args) -> decltype(auto)
		{
			return std::invoke(EventFn, handler, std::forward<decltype(args)>(args)...);
		};
	}
}