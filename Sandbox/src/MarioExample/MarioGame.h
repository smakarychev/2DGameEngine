#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

class MarioGame : public Layer
{
public:
	MarioGame() : Layer("MarioGame layer")
	{}
	void OnAttach() override;
	void OnUpdate() override;
	void OnImguiUpdate() override;
	void OnEvent(Event& e) override;

private:
	bool OnKeyboardPressed(KeyPressedEvent& event);
	bool OnKeyboardReleased(KeyReleasedEvent& event);
	
	void Render();
private:
	std::vector<Ref<Scene>> m_Scenes;
	Ref<Scene> m_CurrentScene{nullptr};
	glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };
};