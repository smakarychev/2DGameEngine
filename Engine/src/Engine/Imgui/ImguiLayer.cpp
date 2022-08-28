#include "enginepch.h"

#include "ImguiLayer.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Input.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	ImguiLayer::ImguiLayer() : Layer("Imgui layer"), 
		m_DockspaceOpen(true), m_DockspaceFlags(ImGuiDockNodeFlags_None), m_MainDockNodeFlags(ImGuiWindowFlags_None)
	{
		m_DockspaceID = 0;
	}

	void ImguiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::SetAllocatorFunctions([](U64 size, void*) { return MemoryManager::Alloc(size); }, [](void* address, void*) { return MemoryManager::Dealloc(address); });
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 450");

		m_MainDockNodeFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	
	void ImguiLayer::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Init dockspace.
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &m_DockspaceOpen, m_MainDockNodeFlags);
		ImGui::PopStyleVar(3);
		m_DockspaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(m_DockspaceID, ImVec2(0.0f, 0.0f), m_DockspaceFlags);
		ImGui::End();
	}
	
	void ImguiLayer::EndFrame()
	{
		ImGui::Begin("Viewport");
		F32 offsetY = ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y;
		F32 offsetX = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x;
		Input::SetMainViewportOffset({ -offsetX, -offsetY });
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImguiLayer::OnEvent(Event& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		e.Handled |= e.IsInCategory(EventCategory::Mouse) & io.WantCaptureMouse;
		e.Handled |= e.IsInCategory(EventCategory::Keyboard) & io.WantCaptureKeyboard;
	}
	
	void ImguiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	
}