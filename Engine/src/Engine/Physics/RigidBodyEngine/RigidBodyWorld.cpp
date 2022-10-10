#include "enginepch.h"

#include "RigidBodyWorld.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	RigidBody2DWorld::RigidBody2DWorld(const glm::vec2& gravity)
		: m_Gravity(gravity), m_NarrowPhase(m_BroadPhase)
	{
	}

	RigidBody2D& RigidBody2DWorld::CreateBody(const RigidBodyDef2D& rbDef)
	{
		m_Bodies.emplace_back(CreateRef<RigidBody2D>(rbDef.Position, rbDef.Mass, rbDef.Inertia));
		RigidBody2D& newBody = *m_Bodies.back();
		newBody.SetPhysicsMaterial(rbDef.PhysicsMaterial);
		if (rbDef.ColliderDef.Collider != nullptr)
		{
			newBody.SetCollider(rbDef.ColliderDef.Collider->Clone());
			newBody.GetCollider()->SetAttachedRigidBody(&newBody);
			// Add body to broad phase and safe returned nodeId for later modification of bvh tree.
			// TODO: store only bodies that are dynamic / kinematic.
			I32 nodeId = m_BroadPhase.InsertRigidBody(&newBody, newBody.GetCollider()->GenerateBounds(newBody.GetTransform()));
			m_BroadPhaseNodes.push_back(nodeId);
		}
		return newBody;
	}

	void RigidBody2DWorld::Update(F32 deltaTime)
	{
		VelocityVerletIntegration(deltaTime);
		SynchronizeBroadPhase(deltaTime);
		//TODO: not the final version.
		m_BroadPhase.FindContacts();
		m_NarrowPhase.Collide(m_BroadPhase.GetContacts());
	}

	void RigidBody2DWorld::AddForce(Ref<RigidBody2DForceGenerator> forceGenerator)
	{
		m_GlobalForces.push_back(forceGenerator);
	}
	
	void RigidBody2DWorld::VelocityVerletIntegration(F32 deltaTime)
	{
		// First update positions and rotations.
		for (auto& body : m_Bodies)
		{
			glm::vec3 newPosition = body->GetPosition() +
				glm::vec3{ body->GetLinearVelocity(), 0.0f } * deltaTime +
				glm::vec3{ body->GetLinearAcceleration(), 0.0f } * deltaTime * deltaTime * 0.5f;
			F32 deltaRotation = body->GetAngularVelocity() * deltaTime +
				body->GetAngularAcceleration() * deltaTime * deltaTime * 0.5f;
			body->AddRotation(deltaRotation);
			body->SetPosition(newPosition);
			body->SetRotation(glm::normalize(body->GetRotation()));
		}
		// Then apply forces.
		ApplyGlobalForces();
		m_Registry.ApplyForces();
		// Then update acceleration and velocity (both linear and angular).
		for (auto& body : m_Bodies)
		{
			glm::vec2 newLinAcceleration = glm::vec2{ 0.0f };
			F32 newAngAcceleration = 0.0f;
			// If body has finite mass, convert its force to acceleration.
			if (body->HasFiniteMass())
			{
				newLinAcceleration += m_Gravity;
				newLinAcceleration += body->GetForce() * body->GetInverseMass();
			}
			// Same logic for inertia tensor and angular acceleration.
			if (body->HasFiniteInertiaTensor())
			{
				newAngAcceleration += body->GetTorque();
			}
			glm::vec2 newLinVelocity = body->GetLinearVelocity() +
				(body->GetLinearAcceleration() + newLinAcceleration) * deltaTime * 0.5f;
			F32 newAngVelocity = body->GetAngularVelocity() +
				(body->GetAngularAcceleration() + newAngAcceleration) * deltaTime * 0.5f;

			body->SetLinearVelocity(newLinVelocity / (1.0f + deltaTime * body->GetLinearDamping()));
			body->SetLinearAcceleration(newLinAcceleration);

			body->SetAngularVelocity(newAngVelocity / (1.0f + deltaTime * body->GetAngularDamping()));
			body->SetAngularAcceleration(newAngAcceleration);
			
			body->ResetForce();
			body->ResetTorque();
		}
	}

	void RigidBody2DWorld::ApplyGlobalForces()
	{
		for (auto& globalForce : m_GlobalForces)
		{
			for (auto& body : m_Bodies)
			{
				globalForce->ApplyForce(*body);
			}
		}
	}

	void RigidBody2DWorld::SynchronizeBroadPhase(F32 deltaTime)
	{
		for (auto nodeId : m_BroadPhaseNodes)
		{
			RigidBody2D* body = reinterpret_cast<RigidBody2D*>(m_BroadPhase.GetPayload(nodeId));
			m_BroadPhase.MoveRigidBody(nodeId, body->GetCollider()->GenerateBounds(body->GetTransform()), body->GetLinearVelocity() * deltaTime);
		}
	}

	void RigidBody2DWorld::AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body)
	{
		m_Registry.Add(forceGenerator, &body);
	}
	
}

