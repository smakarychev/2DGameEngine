#pragma once

#include "Engine/Core/Layer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Engine
{
	class ImguiLayer : public Layer
	{
	public:
		ImguiLayer();
		void OnAttach() override;
		void OnDetach() override;
		void BeginFrame();
		void EndFrame();
		void OnEvent(Event& e) override;
	private:
		bool m_DockSpaceOpen;
		ImGuiDockNodeFlags m_DockSpaceFlags;
		ImGuiWindowFlags m_MainDockNodeFlags;
		ImGuiID m_DockSpaceID;
	};
}