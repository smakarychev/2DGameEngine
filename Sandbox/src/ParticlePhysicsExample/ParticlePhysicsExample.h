#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

class ParticlePhysicsExample : public Layer
{
public:
	ParticlePhysicsExample() : Layer("ParticlePhysics layer")
	{}
	void OnAttach() override;
	void OnUpdate() override;
	void OnImguiUpdate() override;
private:
	void ValidateViewport();
	void Render();
	Rect GetCameraBounds();
	
private:
	Ref<CameraController> m_CameraController;

	Ref<Font> m_Font;

	Ref<FrameBuffer> m_FrameBuffer;
	glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };

	ParticleWorld m_World;
};