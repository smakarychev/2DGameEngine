#pragma once

#include <Engine.h>

#include "Engine/Physics/NewRBE/RigidBodyWorld.h"

using namespace Engine;
using namespace Engine::Types;

class WCustomContactListener : public WIP::Physics::ContactListener
{
public:
	WCustomContactListener() = default;
	void OnContactBegin(const WIP::Physics::ContactInfo2D& contact) override
	{
		//ENGINE_WARN("Collision!");
	}

	void OnContactEnd(const WIP::Physics::ContactInfo2D& contact) override
	{
		//ENGINE_WARN("No more!");
	}
};

class RigidBodyNewPhysicsExample : public Layer
{
public:
	RigidBodyNewPhysicsExample();
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
	Scope<WCustomContactListener> m_ContactListener;
	WIP::Physics::RigidBodyWorld2D m_World;
	WIP::Physics::RigidBody2D* m_Mover = nullptr;
	std::vector<Ref<Component::LocalToWorldTransform2D>> m_Transforms;
};