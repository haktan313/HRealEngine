
//Logger.h
#pragma once
#include "Core.h"
#include "spdlog/spdlog.h"
#include "memory"

namespace HRealEngine
{
	class HREALENGINE_API Logger
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return clientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> coreLogger;
		static std::shared_ptr<spdlog::logger> clientLogger;
	};
}

#define LOG_CORE_TRACE(...) HRealEngine::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) HRealEngine::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARN(...) HRealEngine::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) HRealEngine::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) HRealEngine::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define LOG_CLIENT_TRACE(...) HRealEngine::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define LOG_CLIENT_INFO(...) HRealEngine::Logger::GetClientLogger()->info(__VA_ARGS__)
#define LOG_CLIENT_WARN(...) HRealEngine::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_CLIENT_ERROR(...) HRealEngine::Logger::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CLIENT_FATAL(...) HRealEngine::Logger::GetClientLogger()->critical(__VA_ARGS__)
