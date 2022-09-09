#pragma once

#include "Engine/Rendering/Buffer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#include <string>


namespace Engine
{
	inline glm::vec2 ImguiMainViewport(const FrameBuffer& frameBuffer, const std::string& windowName = "Viewport")
	{
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
        ImGui::Begin(windowName.c_str());

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        U64 textureID = frameBuffer.GetColorBufferId();
        ImGui::Image(reinterpret_cast<void*>(textureID), viewportSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
        ImGui::PopStyleVar(1);
        ImGui::End();
        return glm::vec2{ viewportSize.x, viewportSize.y };
	}
}
