#pragma once

#include <utility>

#include  "Action.h"
#include "SceneGraph.h"

#include "Engine/Core/Camera.h"
#include "Engine/Core/Input.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Events/Event.h"
#include "Engine/Scene/ScenePanels.h"
#include "Engine/Scene/Serialization/SceneSerializer.h"
#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace Engine
{
	class Scene
	{
		// KeyCode and MouseCode are of the same type, and are not intersecting.
		using InputCode = KeyCode;	
	public:
		Scene() : m_SceneGraph(m_Registry), m_ScenePanels(*this), m_SceneSerializer(*this) {}
		virtual ~Scene() = default;
		virtual void Open(const std::string& filename) {}
		virtual void Save(const std::string& filename) {}
		virtual void Clear() {}
		virtual void OnInit() = 0;
		virtual void OnUpdate(F32 dt) = 0;
		virtual void OnEvent(Event& event) = 0;
		virtual void OnRender() = 0;
		virtual void PerformAction(Action& action) = 0;
		virtual Component::Camera* GetMainCamera() = 0;
		virtual FrameBuffer* GetMainFrameBuffer() = 0;
		
		// This is not pure virtual, since it is unnecessary.
		virtual void OnImguiUpdate() {}
		// TODO: very questionable name, basically the purpose of this function is
		// to update internal structures upon introducing new prefabs, etc.
		virtual void OnSceneGlobalUpdate() {}
		
		bool HasAction(InputCode input) const { return m_RegisteredActions[input] != nullptr; }
		Action& GetAction(InputCode input) const { return *m_RegisteredActions[input]; }
		void RegisterAction(InputCode input, Ref<Action> action) { m_RegisteredActions[input] = std::move(action); }
		
		Registry& GetRegistry() { return m_Registry; }
		SceneGraph& GetSceneGraph() { return m_SceneGraph; }
		ScenePanels& GetScenePanels() { return m_ScenePanels; }
		Physics::RigidBodyWorld2D& GetRigidBodyWorld2D() { return m_RigidBodyWorld2D; }
		SceneSerializer& GetSerializer() { return m_SceneSerializer; }
	protected:
		std::vector<Ref<Action>> m_RegisteredActions = std::vector<Ref<Action>>(Key::KEY_COUNT, nullptr);

		Physics::RigidBodyWorld2D m_RigidBodyWorld2D{};
		Registry m_Registry{};
		SceneGraph m_SceneGraph;
		ScenePanels m_ScenePanels;
		SceneSerializer m_SceneSerializer;
		glm::vec2 m_MainViewportSize{};

		bool m_IsSceneReady{false};
	};
}
