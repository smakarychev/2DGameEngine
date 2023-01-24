#pragma once

#include "RigidBody.h"
#include "Collision/BroadPhase.h"
#include "Collision/NarrowPhase.h"

#include "Engine/Core/Core.h"

#include <vector>


namespace Engine::WIP::Physics
{
	using namespace Types;

	class RigidBodyWorld2D
	{
	public:
		RigidBodyWorld2D(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f });
		~RigidBodyWorld2D();
		void Clear();
		// All rigid bodies shall be created by this method.
		RigidBody2D* CreateBody(const RigidBodyDef2D& rbDef);
		void RemoveBody(RigidBody2D* body);
		// Adds new collider to given rigidbody.
		Collider2D* AddCollider(RigidBody2D* body, const ColliderDef2D& colliderDef);
		void RemoveCollider(RigidBody2D* body, Collider2D* collider);
		
		void Update(F32 deltaTime, U32 velocityIters = 8, U32 positionIters = 10);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		
		const RigidBodyList2D& GetBodyList() const { return m_BodyList; }
		RigidBodyList2D& GetBodyList() { return m_BodyList; }

		const BroadPhase2D<>& GetBroadPhase() const { return m_BroadPhase; }

		void EnableWarmStart(bool enable) { m_WarmStartEnabled = enable; }
		bool IsWarmStartEnabled() const { return m_WarmStartEnabled; }
		void SetContactListener(ContactListener* contactListener) { m_NarrowPhase.SetContactListener(contactListener); }
	private:
		void SynchronizeBroadPhase(F32 deltaTime);
		void UpdateDeletedColliders();
	private:
		// Stores all the bodies.
		RigidBodyList2D m_BodyList;

		std::vector<Collider2D*> m_CollidersToDelete;
		
		BroadPhase2D<> m_BroadPhase;

		NarrowPhase2D m_NarrowPhase;
		bool m_WarmStartEnabled;
			
		glm::vec2 m_Gravity;

	};
}