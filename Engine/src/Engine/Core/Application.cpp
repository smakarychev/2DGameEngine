#include "enginepch.h"
#include "Application.h"

#include "Engine/Core/Input.h"

namespace Engine {

	Application* Application::s_Instance = nullptr;

	Application::Application() : m_IsRunning(true)
	{
		ENGINE_ASSERT(s_Instance == nullptr, "Application already exists.");
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

		

		// Should be last for correct work of ...Down()/...Up() versions of input.
		Input::OnUpdate();
	}

	void Application::OnEvent(Event& event)
	{
		ENGINE_CORE_INFO("{}", event.ToString());
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_FN(Application::OnApplicationClose));
		Input::OnEvent(event);
	}

	bool Application::OnApplicationClose(WindowCloseEvent& event)
	{
		m_IsRunning = false;
		return true;
	}
}


