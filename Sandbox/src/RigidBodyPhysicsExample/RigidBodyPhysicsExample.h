#pragma once

#include <Engine.h>

using namespace Engine;
using namespace Engine::Types;

class CustomContactListener : public Physics::ContactListener
{
public:
	CustomContactListener() = default;
	void OnContactBegin(const Physics::ContactInfo2D& contact) override
	{
		//ENGINE_WARN("Collision!");
	}

	void OnContactEnd(const Physics::ContactInfo2D& contact) override
	{
		//ENGINE_WARN("No more!");
	}
};

class RigidBodyPhysicsExample : public Layer
{
public:
	RigidBodyPhysicsExample();
	void OnAttach() override;
	void OnUpdate() override;
	void OnImguiUpdate() override;
private:
	void ValidateViewport();
	void Render();
	CRect GetCameraBounds();

private:
	Ref<CameraController> m_CameraController;

	Ref<Font> m_Font;

	Ref<FrameBuffer> m_FrameBuffer;
	glm::vec2 m_ViewportSize = glm::vec2{ 0.0f };
	Scope<CustomContactListener> m_ContactListener;
	Physics::RigidBodyWorld2D m_World;
	Physics::RigidBody2D* m_Mover = nullptr;
	std::vector<Ref<Component::LocalToWorldTransform2D>> m_Transforms;
};