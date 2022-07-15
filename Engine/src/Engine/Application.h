#pragma once

#include <memory>
#include "Window.h"

namespace Engine {
	class Application
	{
	public:
		Application();
		~Application();
		void Run();
	private:
		std::unique_ptr<Window> m_Window;
	};

	// Client shall define this function.
	std::unique_ptr<Application> CreateApplication();
}

