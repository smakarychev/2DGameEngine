#pragma once

#include <memory>

namespace Engine {
	class Application
	{
	public:
		Application();
		~Application();
		void Run();
	};

	// Client shall define this function.
	std::unique_ptr<Application> CreateApplication();
}

