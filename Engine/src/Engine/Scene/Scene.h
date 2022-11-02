#pragma once

#include <utility>

#include  "Action.h"

#include "Engine/Core/Camera.h"
#include "Engine/ECS/Components.h"
#include "Engine/ECS/Entity.h"
#include "Engine/Events/Event.h"

#include "Engine/Physics/RigidBodyEngine/RigidBodyWorld.h"

namespace Engine
{
	class Scene
	{
		// KeyCode and MouseCode are of the same type, and are not intersecting.
		using InputCode = KeyCode;	
	public:
		virtual ~Scene() = default;
		virtual void OnInit() = 0;
		virtual void OnUpdate(F32 dt) = 0;
		virtual void OnEvent(Event& event) = 0;
		virtual void OnRender() = 0;
		virtual void PerformAction(Action& action) = 0;
		bool HasAction(InputCode input) const { return m_RegisteredActions[input] != nullptr; }
		Action& GetAction(InputCode input) const { return *m_RegisteredActions[input]; }
		void RegisterAction(InputCode input, Ref<Action> action) { m_RegisteredActions[input] = std::move(action); }
		void SetCamera(Camera* camera) { m_Camera = camera; }
	protected:
		std::vector<Ref<Action>> m_RegisteredActions = std::vector<Ref<Action>>(Key::KEY_COUNT, nullptr);
		// TODO: does scene w/o a camera make sense?
		Camera* m_Camera = nullptr;
	};
}
