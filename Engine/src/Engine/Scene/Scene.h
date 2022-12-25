#pragma once

#include <utility>

#include  "Action.h"
#include "SceneGraph.h"

#include "Engine/Core/Camera.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Events/Event.h"
#include "Engine/Scene/ScenePanels.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace Engine
{
	class Scene
	{
		// KeyCode and MouseCode are of the same type, and are not intersecting.
		using InputCode = KeyCode;	
	public:
		Scene() : m_ScenePanels(*this), m_SceneGraph(m_Registry) {}
		virtual ~Scene() = default;
		virtual void OnInit() = 0;
		virtual void OnUpdate(F32 dt) = 0;
		virtual void OnEvent(Event& event) = 0;
		virtual void OnRender() = 0;
		virtual void PerformAction(Action& action) = 0;
		// This is not pure virtual, since it is unnecessary.
		virtual void OnImguiUpdate() {}
		bool HasAction(InputCode input) const { return m_RegisteredActions[input] != nullptr; }
		Action& GetAction(InputCode input) const { return *m_RegisteredActions[input]; }
		void RegisterAction(InputCode input, Ref<Action> action) { m_RegisteredActions[input] = std::move(action); }
		void SetCamera(Camera* camera) { m_Camera = camera; }
		void SetFramebuffer(FrameBuffer* frameBuffer) { m_FrameBuffer = frameBuffer; }
		FrameBuffer* GetFrameBuffer() { return m_FrameBuffer; }
		
		Registry& GetRegistry() { return m_Registry; }
		Physics::RigidBodyWorld2D& GetRigidBodyWorld2D() { return m_RigidBodyWorld2D; }
		
	protected:
		std::vector<Ref<Action>> m_RegisteredActions = std::vector<Ref<Action>>(Key::KEY_COUNT, nullptr);
		// TODO: does scene w/o a camera / framebuffer make sense?
		Camera* m_Camera = nullptr;
		FrameBuffer* m_FrameBuffer = nullptr;

		Physics::RigidBodyWorld2D m_RigidBodyWorld2D{};
		Registry m_Registry{};
		SceneGraph m_SceneGraph;
		ScenePanels m_ScenePanels;
	};
}
