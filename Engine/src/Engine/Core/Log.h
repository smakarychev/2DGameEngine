#pragma once

#include <memory>

#pragma warning (push, 0)
#include <spdlog/spdlog.h>
#pragma warning (pop)

namespace Engine {
	
	class Log
	{
		template<typename... Args>
		using Fmt = spdlog::format_string_t<Args...>;
	public:
		static void Init();

		template<typename ...Ts>
		static void CoreTrace(Fmt<Ts...> fmt, Ts&&... args)	  { s_CoreLogger->trace(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>
		static void CoreInfo(Fmt<Ts...> fmt, Ts&&... args)    { s_CoreLogger->info(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>								  
		static void CoreWarn(Fmt<Ts...> fmt, Ts&&... args)    { s_CoreLogger->warn(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>								  
		static void CoreError(Fmt<Ts...> fmt, Ts&&... args)   { s_CoreLogger->error(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>								  
		static void CoreFatal(Fmt<Ts...> fmt, Ts&&... args)	  { s_CoreLogger->critical(fmt, std::forward<Ts>(args)...); }

		template<typename ...Ts>
		static void ClientTrace(Fmt<Ts...> fmt, Ts&&... args) {	s_ClientLogger->trace(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>
		static void ClientInfo(Fmt<Ts...> fmt, Ts&&... args)  { s_ClientLogger->info(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>
		static void ClientWarn(Fmt<Ts...> fmt, Ts&&... args)  { s_ClientLogger->warn(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>
		static void ClientError(Fmt<Ts...> fmt, Ts&&... args) {	s_ClientLogger->error(fmt, std::forward<Ts>(args)...); }
		template<typename ...Ts>
		static void ClientFatal(Fmt<Ts...> fmt, Ts&&... args) { s_ClientLogger->critical(fmt, std::forward<Ts>(args)...); }
		
		// Repeat all that is above but for the case when no format string was passed.
		template<typename T>
		static void CoreTrace(const T& message) { s_CoreLogger->trace(message); }
		template<typename T>
		static void CoreInfo(const T& message) { s_CoreLogger->info(message); }
		template<typename T>
		static void CoreWarn(const T& message) { s_CoreLogger->warn(message); }
		template<typename T>
		static void CoreError(const T& message) { s_CoreLogger->error(message); }
		template<typename T>
		static void CoreFatal(const T& message) { s_CoreLogger->critical(message); }

		template<typename T>
		static void ClientTrace(const T& message) { s_ClientLogger->trace(message); }
		template<typename T>
		static void ClientInfo(const T& message) { s_ClientLogger->info(message); }
		template<typename T>
		static void ClientWarn(const T& message) { s_ClientLogger->warn(message); }
		template<typename T>
		static void ClientError(const T& message) { s_ClientLogger->error(message); }
		template<typename T>
		static void ClientFatal(const T& message) { s_ClientLogger->critical(message); }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Macros for engine messages
#define ENGINE_CORE_TRACE(...)  ::Engine::Log::CoreTrace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)   ::Engine::Log::CoreInfo(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)   ::Engine::Log::CoreWarn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)  ::Engine::Log::CoreError(__VA_ARGS__)
#define ENGINE_CORE_FATAL(...)  ::Engine::Log::CoreFatal(__VA_ARGS__)

// Macros for client messages
#define ENGINE_TRACE(...)		::Engine::Log::ClientTrace(__VA_ARGS__)
#define ENGINE_INFO(...)		::Engine::Log::ClientInfo(__VA_ARGS__)
#define ENGINE_WARN(...)		::Engine::Log::ClientWarn(__VA_ARGS__)
#define ENGINE_ERROR(...)		::Engine::Log::ClientError(__VA_ARGS__)
#define ENGINE_FATAL(...)		::Engine::Log::ClientFatal(__VA_ARGS__)