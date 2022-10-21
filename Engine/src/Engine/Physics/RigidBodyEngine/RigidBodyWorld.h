#pragma once

#include "RigidBody.h"
#include "RigidBodyForceGenerator.h"
#include "Collision/BroadPhase.h"
#include "Collision/NarrowPhase.h"

#include "Engine/Core/Core.h"

#include <vector>


namespace Engine
{
	using namespace Types;

	struct RigidBodyNode2D
	{
		RigidBody2D* Body = nullptr;
		RigidBodyNode2D* Next = nullptr;
		RigidBodyNode2D* Prev = nullptr;
	};

	//! The main problem now is the lack of ability to delete bodies.
	class RigidBody2DWorld
	{
	public:
		RigidBody2DWorld(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f }, U32 iterations = 4);
		~RigidBody2DWorld();
		// All rigid bodies shall be created by this method.
		RigidBody2D* CreateBody(const RigidBodyDef2D& rbDef);
		
		void Update(F32 deltaTime);

		// Add the global force, acting on all bodies.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator);

		// Add force to some body.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		void SetIterations(U32 iterations) { m_NarrowPhaseIterations = iterations; }
		
		const RigidBodyNode2D* GetBodyList() const { return m_BodyList; }
		void RemoveBody(RigidBodyNode2D* bodyNode);


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
		RigidBodyNode2D* m_BodyList = nullptr;

		// Stores all global forces.
		std::vector<Ref<RigidBody2DForceGenerator>> m_GlobalForces;

		// Stores all force generators and the bodies they apply force to.
		RigidBody2DForceRegistry m_Registry;

		BroadPhase2D<> m_BroadPhase;
		// Stores the nodeIds of broad phase.
		std::vector<I32> m_BroadPhaseNodes;

		NarrowPhase2D m_NarrowPhase;
		U32 m_NarrowPhaseIterations;
		bool m_WarmStartEnabled;
			
		glm::vec2 m_Gravity;
		

	};
}