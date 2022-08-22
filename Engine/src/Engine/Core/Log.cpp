#include "enginepch.h"
#include "Log.h"

#pragma warning (push, 0)
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning (pop)

constexpr auto CORE_LOGGER_NAME = "CoreLogger";
constexpr auto CLIENT_LOGGER_NAME = "ClientLogger";

namespace Engine {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		// _mt version might be slightly slower than _st.
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("%^[%X] %n: %v%$");

		std::vector<spdlog::sink_ptr> sinks = { consoleSink, };
		
		s_CoreLogger = std::make_shared<spdlog::logger>(CORE_LOGGER_NAME, sinks.begin(), sinks.end());
		s_ClientLogger = std::make_shared<spdlog::logger>(CLIENT_LOGGER_NAME, sinks.begin(), sinks.end());

		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger->set_level(spdlog::level::trace);

		s_CoreLogger->flush_on(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);
	}

}
