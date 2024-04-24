#include "enginepch.h"
#include "Application.h"

#include <ranges>

#include "Engine/Core/Input.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Rendering/Renderer.h"

namespace Engine {

	Application* Application::s_Instance = nullptr;

	Application::Application() : m_IsRunning(true)
	{
		ENGINE_CORE_ASSERT(s_Instance == nullptr, "Application already exists.")
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

		m_ImguiLayer = CreateRef<ImguiLayer>();
		PushOverlay(m_ImguiLayer);

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

		m_ImguiLayer->BeginFrame();
		for (auto& layer : m_LayerStack)
		{
			layer->OnUpdate();
		}

		for (auto& layer : m_LayerStack)
		{
			layer->OnImguiUpdate();
		}
		m_ImguiLayer->EndFrame();

		// Should be last for correct work of ...Down()/...Up() versions of input.
		Input::OnUpdate();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_FN(Application::OnApplicationClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_FN(Application::OnWindowResize));
		Input::OnEvent(event);

		for (auto& layer : std::ranges::reverse_view(m_LayerStack))
		{
			layer->OnEvent(event);
			if (event.Handled) break;
		}
	}

	void Application::PushLayer(Ref<Layer> layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Ref<Layer> overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnApplicationClose(WindowCloseEvent& event)
	{
		m_IsRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		return false;
	}
}


