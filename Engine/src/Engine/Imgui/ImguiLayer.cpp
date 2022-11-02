#include "enginepch.h"

#include "ImguiLayer.h"

#include "ImguiCommon.h"

#include "Engine/Core/Application.h"
#include "Engine/Core/Input.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	ImguiLayer::ImguiLayer() : Layer("Imgui layer"), 
		m_DockSpaceOpen(true), m_DockSpaceFlags(ImGuiDockNodeFlags_None), m_MainDockNodeFlags(ImGuiWindowFlags_None)
	{
		m_DockSpaceID = 0;
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
		ImGui::Begin("DockSpace", &m_DockSpaceOpen, m_MainDockNodeFlags);
		ImGui::PopStyleVar(3);
		m_DockSpaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(m_DockSpaceID, ImVec2(0.0f, 0.0f), m_DockSpaceFlags);
		ImGui::End();
	}
	
	void ImguiLayer::EndFrame()
	{
		ImGui::Begin("Viewport");
		ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
		ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
		F32 offsetY = contentMin.y + ImGui::GetWindowPos().y;
		F32 offsetX = contentMin.x + ImGui::GetWindowPos().x;
		
		Input::SetMainViewportOffset({ -offsetX, -offsetY });
		Input::SetMainViewportSize({contentMax.x - contentMin.x, contentMax.y - contentMin.y });
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImguiLayer::OnEvent(Event& e)
	{
		const ImGuiIO& io = ImGui::GetIO();
		e.Handled |= e.IsInCategory(EventCategory::Mouse) & io.WantCaptureMouse;
		e.Handled |= e.IsInCategory(EventCategory::Keyboard) & io.WantCaptureKeyboard;
		e.Handled &= ImguiState::BlockEventPropagation;
	}
	
	void ImguiLayer::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	
}