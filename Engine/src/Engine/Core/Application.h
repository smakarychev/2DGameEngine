#pragma once

#include "Engine/Core/LayerStack.h"
#include "Engine/Core/Window.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"

#include <memory>

namespace Engine {
	class Application
	{
	public:
		Application();
		~Application();
		void Run();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

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

		LayerStack m_LayerStack;
	};

	// Client shall define this function.
	std::unique_ptr<Application> CreateApplication();
}

