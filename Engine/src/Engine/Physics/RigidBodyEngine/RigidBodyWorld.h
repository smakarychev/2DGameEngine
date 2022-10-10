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

	//! The main problem now is the lack of ability to delete bodies.
	class RigidBody2DWorld
	{
	public:
		RigidBody2DWorld(const glm::vec2& gravity = glm::vec2{ 0.0f, -10.0f });

		// All rigid bodies shall be created by this method.
		RigidBody2D& CreateBody(const RigidBodyDef2D& rbDef);
		
		void Update(F32 deltaTime);

		// Add the global force, acting on all bodies.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator);

		// Add force to some body.
		void AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body);

		void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }
		const std::vector<Ref<RigidBody2D>>& GetBodies() const { return m_Bodies; }

		const BroadPhase2D<>& GetBroadPhase() const { return m_BroadPhase; }
	private:
		void VelocityVerletIntegration(F32 deltaTime);
		void ApplyGlobalForces();
		void SynchronizeBroadPhase(F32 deltaTime);
	private:
		// Stores all the bodies (smart pointers to them).
		std::vector<Ref<RigidBody2D>> m_Bodies;

		// Stores all global forces.
		std::vector<Ref<RigidBody2DForceGenerator>> m_GlobalForces;

		// Stores all force generators and the bodies they apply force to.
		RigidBody2DForceRegistry m_Registry;

		BroadPhase2D<> m_BroadPhase;
		// Stores the nodeIds of broad phase.
		std::vector<I32> m_BroadPhaseNodes;

		NarrowPhase2D m_NarrowPhase;
			
		glm::vec2 m_Gravity;

	};
}