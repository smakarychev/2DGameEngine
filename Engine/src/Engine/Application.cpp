#include "enginepch.h"
#include "Application.h"

namespace Engine {
	Application::Application() 
	{
		m_Window = Window::Create();
	}
	Application::~Application()	
	{

	}

	void Application::Run()
	{
		m_Window->OnUpdate();
	}
}


