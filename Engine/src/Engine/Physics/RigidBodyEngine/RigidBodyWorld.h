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

	class RigidBodyWorld2D
	{
		using RigidBodyList = RigidBodyListEntry2D;
		using ColliderList = ColliderListEntry2D;
	public:
		RigidBodyWorld2D(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f });
		~RigidBodyWorld2D();
		void Clear();
		// All rigid bodies shall be created by this method.
		RigidBody2D* CreateBody(const RigidBodyDef2D& rbDef);
		void RemoveBody(RigidBody2D* body);
		// Adds new collider to given rigidbody.
		Collider2D* SetCollider(RigidBody2D* body, const ColliderDef2D& colliderDef);
		Collider2D* AddCollider(const ColliderDef2D& colliderDef);
		// Removes collider from rigidbody. NOTE: this does not delete collider,
		// to delete collider completely, use `DeleteCollider`.
		void RemoveCollider(RigidBody2D* body, Collider2D* collider);
		void DeleteCollider(Collider2D* collider);
		
		void Update(F32 deltaTime, U32 velocityIters = 8, U32 positionIters = 10);

		// Add the global force, acting on all bodies.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator);

		// Add force to some body.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		
		const RigidBodyList* GetBodyList() const { return m_BodyList; }
		const ColliderList* GetColliderList() const { return m_ColliderList; }

		const BroadPhase2D<>& GetBroadPhase() const { return m_BroadPhase; }

		void EnableWarmStart(bool enable) { m_WarmStartEnabled = enable; }
		bool IsWarmStartEnabled() const { return m_WarmStartEnabled; }
		void SetContactListener(ContactListener* contactListener) { m_NarrowPhase.SetContactListener(contactListener); }
	private:
		RigidBody2D* AddBodyToList(const RigidBodyDef2D& rbDef);
		void RemoveBodyFromList(RigidBody2D* body);
		Collider2D* AddColliderToList(const ColliderDef2D& colliderDef);
		void RemoveColliderFromList(Collider2D* collider);
		void ApplyGlobalForces();
		void SynchronizeBroadPhase(F32 deltaTime);
	private:
		// Stores all the bodies.
		RigidBodyList* m_BodyList{nullptr};

		// Stores all the colliders.
		ColliderList* m_ColliderList{nullptr};

		// Stores all global forces.
		std::vector<Ref<RigidBody2DForceGenerator>> m_GlobalForces;

		// Stores all force generators and the bodies they apply force to.
		RigidBody2DForceRegistry m_Registry;

		BroadPhase2D<> m_BroadPhase;
		// Stores the nodeIds of broad phase.
		std::unordered_map<Collider2D*, I32> m_BroadPhaseNodesMap;
		std::vector<Collider2D*> m_BroadPhaseNodesToDelete;

		NarrowPhase2D m_NarrowPhase;
		bool m_WarmStartEnabled;
			
		glm::vec2 m_Gravity;

	};
}