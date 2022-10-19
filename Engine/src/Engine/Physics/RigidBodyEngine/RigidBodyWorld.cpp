#include "enginepch.h"

#include "RigidBodyWorld.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	RigidBody2DWorld::RigidBody2DWorld(const glm::vec2& gravity, U32 iterations)
		: m_Gravity(gravity), m_NarrowPhase(m_BroadPhase), m_NarrowPhaseIterations(iterations), m_WarmStartEnabled(true)
	{
	}

	RigidBody2D& RigidBody2DWorld::CreateBody(const RigidBodyDef2D& rbDef)
	{
		m_Bodies.emplace_back(CreateRef<RigidBody2D>(rbDef));
		RigidBody2D& newBody = *m_Bodies.back();
		newBody.SetPhysicsMaterial(rbDef.PhysicsMaterial);
		if (rbDef.ColliderDef.Collider != nullptr)
		{
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
		SynchronizeBroadPhase(deltaTime);

		ApplyGlobalForces();
		m_Registry.ApplyForces();
		// Update velocities based on forces.
		for (auto& body : m_Bodies)
		{
			glm::vec2 linAcc{ 0.0f };
			F32 angAcc{ 0.0f };
			// If body has finite mass, convert its force to acceleration.
			if (body->HasFiniteMass())
			{
				linAcc += m_Gravity;
				linAcc += body->GetForce() * body->GetInverseMass();
			}
			// Same logic for inertia tensor and angular acceleration.
			if (body->HasFiniteInertiaTensor())
			{
				angAcc += body->GetTorque() * body->GetInverseInertia();
			}

			glm::vec2 newLinVel = body->GetLinearVelocity() + linAcc * deltaTime;
			F32 newAngVel = body->GetAngularVelocity() + angAcc * deltaTime;
			// Apply damping.
			newLinVel = newLinVel / (1.0f + deltaTime * body->GetLinearDamping());
			newAngVel = newAngVel / (1.0f + deltaTime * body->GetAngularDamping());
			// Update body vels.
			body->SetLinearVelocity(newLinVel);
			body->SetAngularVelocity(newAngVel);
		}

		// Perform warm start / pre steps. TODO
		m_BroadPhase.FindContacts([&narrowPhase = m_NarrowPhase](const PotentialContact2D& contact) {narrowPhase.Callback(contact); });
		m_NarrowPhase.Collide();

		ContactResolverDef crDef{
			.ContactList = m_NarrowPhase.GetContactInfoList(),
			.ContactListSize = m_NarrowPhase.GetContactsCount(),
			.WarmStartEnabled = m_WarmStartEnabled };

		ContactResolver::PreSolve(crDef);
		if (m_WarmStartEnabled)
		{
			ContactResolver::WarmStart();
		}

		// Resolve velocity constraints.
		for (U32 i = 0; i < m_NarrowPhaseIterations + 2; i++)
		{
			ContactResolver::ResolveVelocity();
		}
		// Update positions based on velocities.
		for (auto& body : m_Bodies)
		{
			glm::vec2 newPos = body->GetPosition() + body->GetLinearVelocity() * deltaTime;
			F32 deltaRot = body->GetAngularVelocity() * deltaTime;
			
			body->SetPosition(newPos);
			body->AddRotation(deltaRot);
			body->SetRotation(glm::normalize(body->GetRotation()));

			body->ResetForce();
			body->ResetTorque();
		}
		// Resolve position constraints. 
		for (U32 i = 0; i < m_NarrowPhaseIterations; i++)
		{
			if (ContactResolver::ResolvePosition())
			{
				break;
			}
		}
		ContactResolver::PostSolve();
	}

	void RigidBody2DWorld::AddForce(Ref<RigidBody2DForceGenerator> forceGenerator)
	{
		m_GlobalForces.push_back(forceGenerator);
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

