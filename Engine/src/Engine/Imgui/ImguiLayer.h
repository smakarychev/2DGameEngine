#pragma once

#include "Engine/Core/Layer.h"

#include <imgui/imgui.h>

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
		void SetUpStyle();
	private:
		bool m_DockSpaceOpen;
		ImGuiDockNodeFlags m_DockSpaceFlags;
		ImGuiWindowFlags m_MainDockNodeFlags;
		ImGuiID m_DockSpaceID;
	};
}