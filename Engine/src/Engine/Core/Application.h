#pragma once

#include  "Engine/Core/Core.h"
#include "Engine/Core/LayerStack.h"
#include "Engine/Core/Window.h"

#include "Engine/Events/Event.h"
#include "Engine/Events/WindowEvents.h"
#include "Engine/Events/KeyboardEvents.h"
#include "Engine/Events/MouseEvents.h"
#include "Engine/Imgui/ImguiLayer.h"

#include <memory>

namespace Engine {
	class Application
	{
	public:
		Application();
		Application(const Application&) = delete;
		Application& operator=(Application&) = delete;
		~Application();
		void Run();

		void PushLayer(Ref<Layer> layer);
		void PushOverlay(Ref<Layer> overlay);

		static Application& Get() { return *s_Instance; }
		Window& GetWindow() const { return *m_Window; }

		void Exit() { m_IsRunning = false; }
		
	private:
		void OnCreate();
		void OnUpdate();
		void OnEvent(Event& event);
		bool OnApplicationClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_IsRunning;

		static Application* s_Instance;

		LayerStack m_LayerStack;
		Ref<ImguiLayer> m_ImguiLayer;
	};

	// Client shall define this function.
	std::unique_ptr<Application> createApplication();
}

