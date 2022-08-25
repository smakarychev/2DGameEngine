#include "enginepch.h"

#include "ImguiLayer.h"

#include "Engine/Core/Application.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	ImguiLayer::ImguiLayer() : Layer("Imgui layer")
	{
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
	}
	
	void ImguiLayer::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	
	void ImguiLayer::EndFrame()
	{
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