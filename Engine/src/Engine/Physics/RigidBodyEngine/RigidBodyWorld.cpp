#include "enginepch.h"

#include "RigidBodyWorld.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	RigidBody2DWorld::RigidBody2DWorld(const glm::vec2& gravity, U32 iterations)
		: m_Gravity(gravity), m_NarrowPhase(m_BroadPhase), m_NarrowPhaseIterations(iterations), m_WarmStartEnabled(true)
	{
	}

	RigidBody2DWorld::~RigidBody2DWorld()
	{
		while (m_BodyList != nullptr)
		{
			RigidBodyNode2D* next = m_BodyList->Next;
			Delete<RigidBody2D>(m_BodyList->Body);
			Delete<RigidBodyNode2D>(m_BodyList);
			m_BodyList = next;
		}
	}

	RigidBody2D* RigidBody2DWorld::AddBodyToList(const RigidBodyDef2D& rbDef)
	{
		RigidBodyNode2D* newNode = New<RigidBodyNode2D>();
		RigidBody2D* newBody = New<RigidBody2D>(rbDef);
		newNode->Body = newBody;
		if (rbDef.ColliderDef.Collider != nullptr)
		{
			newBody->GetCollider()->SetAttachedRigidBody(newBody);
			// Add body to broad phase and safe returned nodeId for later modification of bvh tree.
			// TODO: store only bodies that are dynamic / kinematic.
			I32 nodeId = m_BroadPhase.InsertRigidBody(newBody, newBody->GetCollider()->GenerateBounds(newBody->GetTransform()));
			m_BroadPhaseNodes.push_back(nodeId);
		}
		newNode->Next = m_BodyList;
		if (m_BodyList != nullptr)
		{
			m_BodyList->Prev = newNode;
		}
		m_BodyList = newNode;

		return newBody;
	}

	void RigidBody2DWorld::RemoveBody(RigidBodyNode2D* bodyNode)
	{
		// Delete node.
		if (bodyNode == m_BodyList)
		{
			m_BodyList = bodyNode->Next;
		}
		if (bodyNode->Prev != nullptr)
		{
			bodyNode->Prev->Next = bodyNode->Next;
		}
		if (bodyNode->Next != nullptr)
		{
			bodyNode->Next->Prev = bodyNode->Prev;
		}
		Delete<RigidBody2D>(bodyNode->Body);
		Delete<RigidBodyNode2D>(bodyNode);
	}

	RigidBody2D* RigidBody2DWorld::CreateBody(const RigidBodyDef2D& rbDef)
	{
		return AddBodyToList(rbDef);
	}

	void RigidBody2DWorld::Update(F32 deltaTime)
	{
		SynchronizeBroadPhase(deltaTime);

		ApplyGlobalForces();
		m_Registry.ApplyForces();
		// Update velocities based on forces.
		RigidBodyNode2D* currentNode = m_BodyList;
		while (currentNode != nullptr)
		{
			RigidBody2D* body = currentNode->Body;

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

			currentNode = currentNode->Next;
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
		currentNode = m_BodyList;
		while (currentNode != nullptr)
		{
			RigidBody2D* body = currentNode->Body;

			glm::vec2 newPos = body->GetPosition() + body->GetLinearVelocity() * deltaTime;
			F32 deltaRot = body->GetAngularVelocity() * deltaTime;
			
			body->SetPosition(newPos);
			body->AddRotation(deltaRot);
			body->SetRotation(glm::normalize(body->GetRotation()));

			body->ResetForce();
			body->ResetTorque();

			currentNode = currentNode->Next;
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
			RigidBodyNode2D* currentNode = m_BodyList;
			while (currentNode != nullptr)
			{
				RigidBody2D* body = currentNode->Body;
				globalForce->ApplyForce(*body);
				currentNode = currentNode->Next;
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

