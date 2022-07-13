#pragma once

#include <spdlog/spdlog.h>

namespace Engine {
	
	class Log
	{
	public:
		static void Init();
		inline static auto& GetCoreLogger() { return *s_CoreLogger; }
		inline static auto& GetClientLogger() { return *s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Macros for engine messages
#define ENGINE_CORE_TRACE(...)  ::Engine::Log::GetCoreLogger().trace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)   ::Engine::Log::GetCoreLogger().info(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)   ::Engine::Log::GetCoreLogger().warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)  ::Engine::Log::GetCoreLogger().error(__VA_ARGS__)
#define ENGINE_CORE_FATAL(...)  ::Engine::Log::GetCoreLogger().critical(__VA_ARGS__)

// Macros for client messages
#define ENGINE_TRACE(...)		::Engine::Log::GetClientLogger().trace(__VA_ARGS__)
#define ENGINE_INFO(...)		::Engine::Log::GetClientLogger().info(__VA_ARGS__)
#define ENGINE_WARN(...)		::Engine::Log::GetClientLogger().warn(__VA_ARGS__)
#define ENGINE_ERROR(...)		::Engine::Log::GetClientLogger().error(__VA_ARGS__)
#define ENGINE_FATAL(...)		::Engine::Log::GetClientLogger().critical(__VA_ARGS__)