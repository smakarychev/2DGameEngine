#pragma once

#include "Engine/Rendering/Buffer.h"

#include <imgui/imgui.h>
#include <glm/glm.hpp>

#include <string>


namespace Engine
{
	namespace ImguiState
	{
		inline bool BlockEventPropagation = false;
		inline glm::vec2 MainViewportSize = {1600.0f, 900.0f};
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
		ImguiState::BlockEventPropagation = !ImGui::IsWindowHovered(); 

		ImGui::End();
        return glm::vec2{ viewportSize.x, viewportSize.y };
	}

	static constexpr auto IMGUI_LIMITLESS = FLT_MAX / INT_MAX;

	namespace ImGuiCommon
	{
		template <typename Fn>
		decltype(auto) Draw2Columns(const std::string& label, Fn fn, bool delayEnd = false)
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;
			if (ImGui::BeginTable(label.c_str(), 2, flags))
			{
				ImGui::TableNextColumn();
				ImGui::TextWrapped(label.c_str());
				ImGui::TableNextColumn();
				auto retVal = fn();
				if (!delayEnd || !retVal) ImGui::EndTable();
				return retVal;
			}
		}
		
		
		inline void DrawFloat(const std::string& label, float& val, float step,  float min, float max)
		{
			Draw2Columns(label, [&](){ return ImGui::DragFloat( std::string("##" + label).c_str(), &val, step, min, max); });
		}
	
		inline void DrawFloat2(const std::string& label, glm::vec2& float2, float step,  float min, float max)
		{
			Draw2Columns(label, [&](){ return ImGui::DragFloat2( std::string("##" + label).c_str(), &float2[0], step, min, max); });
		}
	
		inline void DrawFloat3(const std::string& label, glm::vec3& float3, float step,  float min, float max)
		{
			Draw2Columns(label, [&](){ return ImGui::DragFloat3( std::string("##" + label).c_str(), &float3[0], step, min, max); });
		}

		inline void DrawTextField(const std::string& label, std::string& text)
		{
			ENGINE_CORE_ASSERT(text.size() < 256, "Text too big.")
			char buf[256] = {};
			text.copy(buf, text.size() + 1);
			Draw2Columns(label, [&](){ return ImGui::InputText( std::string("##" + label).c_str(), buf, 256); });
			text = std::string(buf);
		}

		inline void DrawColor(const std::string& label, glm::vec4& color)
		{
			Draw2Columns(label, [&](){ return ImGui::ColorPicker4(std::string("##" + label).c_str(), &color[0]); });
		}

		inline bool BeginCombo(const std::string& label, const std::string& val)
		{
			return Draw2Columns(label, [&](){ return ImGui::BeginCombo(std::string("##" + label).c_str(), val.c_str()); }, true);
		}

		inline void EndCombo()
		{
			ImGui::EndCombo();
			ImGui::EndTable();
		}
	
	}
}
