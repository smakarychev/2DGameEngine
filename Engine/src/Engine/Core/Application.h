#pragma once

#include <memory>
#include "Window.h"
#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"

namespace Engine {
	class Application
	{
	public:
		Application();
		~Application();
		void Run();
		void OnEvent(Event& event);

		static const Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }

	private:
		void OnCreate();
		void OnUpdate();
		bool OnApplicationClose(WindowCloseEvent& event);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_IsRunning;

		static Application* s_Instance;
	};

	// Client shall define this function.
	std::unique_ptr<Application> CreateApplication();
}

