#pragma once

#include "Engine/Rendering/Buffer.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include <string>


namespace Engine
{
	namespace ImguiState
	{
		static bool BlockEventPropagation = false;
	}
	
	inline glm::vec2 ImguiMainViewport(const FrameBuffer& frameBuffer, const std::string& windowName = "Viewport")
	{
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(windowName.c_str());

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        U64 textureID = frameBuffer.GetColorBufferId(0);
        ImGui::Image(reinterpret_cast<void*>(textureID), viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::PopStyleVar(1);

		// Check if we need to block events.
		ImguiState::BlockEventPropagation = !(ImGui::IsWindowFocused() || ImGui::IsWindowHovered()); 

		ImGui::End();
        return glm::vec2{ viewportSize.x, viewportSize.y };
	}
	
}
