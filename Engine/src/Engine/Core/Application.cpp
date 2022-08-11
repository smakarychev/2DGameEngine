#include "enginepch.h"
#include "Application.h"

#include "Engine/Core/Input.h"
#include "Engine/Rendering/Renderer.h"

namespace Engine {

	Application* Application::s_Instance = nullptr;

	Application::Application() : m_IsRunning(true)
	{
		ENGINE_CORE_ASSERT(s_Instance == nullptr, "Application already exists.");
		s_Instance = this;
		OnCreate();
	}

	Application::~Application()	
	{

	}

	void Application::OnCreate()
	{
		m_Window = Window::Create();
		m_Window->SetEventCallbackFunction(BIND_FN(Application::OnEvent));

		Renderer::Init();
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			OnUpdate();
		}
	}

	void Application::OnUpdate()
	{

		m_Window->OnUpdate();
		for (auto it = m_LayerStack.begin(); it != m_LayerStack.end(); it++)
		{
			(*it)->OnUpdate();
		}

		// Should be last for correct work of ...Down()/...Up() versions of input.
		Input::OnUpdate();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_FN(Application::OnApplicationClose));
		Input::OnEvent(event);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); it++)
		{
			(*it)->OnEvent(event);
			if (event.Handled) break;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushLayer(overlay);
		overlay->OnAttach();
	}

	bool Application::OnApplicationClose(WindowCloseEvent& event)
	{
		m_IsRunning = false;
		return true;
	}
}


