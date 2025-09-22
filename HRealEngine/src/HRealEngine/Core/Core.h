
#pragma once
#include <memory>

#ifdef HREALENGINE_PLATFORM_WINDOWS
#if HREALENGINE_DYNAMIC_LINK
	#ifdef HREALENGINE_BUILD_DLL
		#define HREALENGINE_API __declspec(dllexport)
	#else
		#define HREALENGINE_API __declspec(dllimport)
	#endif
#else
	#define HREALENGINE_API
#endif
#else
	#error HRealEngine only supports Windows!
#endif

#ifdef HREALENGINE_ENABLE_DEBUGBREAKS
	#define HREALENGINE_CLIENT_DEBUGBREAK(x, ...) { if(!(x)) { LOG_CLIENT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak()} }
	#define HREALENGINE_CORE_CLIENT_DEBUGBREAK(x, ...) { if(!(x)) { LOG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak() } }
#else
	#define HREALENGINE_CLIENT_DEBUGBREAK(x, ...)
	#define HREALENGINE_CORE_DEBUGBREAK(x, ...)
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace HRealEngine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}