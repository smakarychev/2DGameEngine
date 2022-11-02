#pragma once

#include "RigidBody.h"
#include "RigidBodyForceGenerator.h"
#include "Collision/BroadPhase.h"
#include "Collision/NarrowPhase.h"

#include "Engine/Core/Core.h"

#include <vector>


namespace Engine::Physics
{
	using namespace Types;

	//! The main problem now is the lack of ability to delete bodies.
	class RigidBodyWorld2D
	{
		using RigidBodyList  = RigidBodyListEntry2D;
	public:
		RigidBodyWorld2D(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f });
		~RigidBodyWorld2D();
		// All rigid bodies shall be created by this method.
		RigidBody2D* CreateBody(const RigidBodyDef2D& rbDef);
		void RemoveBody(RigidBody2D* body);
		Collider2D* AddCollider(RigidBody2D* body, const ColliderDef2D& colliderDef);
		
		void Update(F32 deltaTime, U32 velocityIters = 8, U32 positionIters = 10);

		// Add the global force, acting on all bodies.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator);

		// Add force to some body.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		
		const RigidBodyList* GetBodyList() const { return m_BodyList; }

		const BroadPhase2D<>& GetBroadPhase() const { return m_BroadPhase; }

		void EnableWarmStart(bool enable) { m_WarmStartEnabled = enable; }
		bool IsWarmStartEnabled() const { return m_WarmStartEnabled; }
		void SetContactListener(ContactListener* contactListener) { m_NarrowPhase.SetContactListener(contactListener); }
	private:
		RigidBody2D* AddBodyToList(const RigidBodyDef2D& rbDef);
		void ApplyGlobalForces();
		void SynchronizeBroadPhase(F32 deltaTime);
	private:
		// Stores all the bodies (smart pointers to them).
		RigidBodyList* m_BodyList = nullptr;

		// Stores all global forces.
		std::vector<Ref<RigidBody2DForceGenerator>> m_GlobalForces;

		// Stores all force generators and the bodies they apply force to.
		RigidBody2DForceRegistry m_Registry;

		BroadPhase2D<> m_BroadPhase;
		// Stores the nodeIds of broad phase.
		std::vector<I32> m_BroadPhaseNodes;

		NarrowPhase2D m_NarrowPhase;
		bool m_WarmStartEnabled;
			
		glm::vec2 m_Gravity;
		

	};
}