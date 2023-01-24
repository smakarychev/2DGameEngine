#include "enginepch.h"

#include "RigidBodyWorld.h"


namespace Engine::WIP::Physics
{
	RigidBodyWorld2D::RigidBodyWorld2D(const glm::vec2& gravity)
		: m_NarrowPhase(m_BroadPhase), m_WarmStartEnabled(true), m_Gravity(gravity)
	{
		BodyListAllocators2D::Init();
		ColliderListAllocators2D::Init();
		ContactListAllocators2D::Init();
	}

	RigidBodyWorld2D::~RigidBodyWorld2D()
	{
		Clear();
		BodyListAllocators2D::ShutDown();
		ColliderListAllocators2D::ShutDown();
		ContactListAllocators2D::ShutDown();
	}

	void RigidBodyWorld2D::Clear()
	{
		for (auto& rbEntry : m_BodyList) rbEntry.GetRigidBody()->m_ColliderList.Clear();
		m_BodyList.Clear();
		m_NarrowPhase.Clear();
		m_BroadPhase.Clear();
	}

	RigidBody2D* RigidBodyWorld2D::CreateBody(const RigidBodyDef2D& rbDef)
	{
		return m_BodyList.Push(rbDef);
	}

	void RigidBodyWorld2D::RemoveBody(RigidBody2D* body)
	{
		m_CollidersToDelete.reserve(m_CollidersToDelete.size() + body->GetColliderList().Size());
		for (auto& colEntry : body->GetColliderList()) m_CollidersToDelete.push_back(colEntry.GetCollider());
		body->OrphanColliders();
		m_BodyList.Pop(body);
	}

	Collider2D* RigidBodyWorld2D::AddCollider(RigidBody2D* body, const ColliderDef2D& colliderDef)
	{
		Collider2D* newCollider = body->AddCollider(colliderDef);
		I32 node =  m_BroadPhase.InsertCollider(newCollider, newCollider->GenerateBounds(newCollider->GetAttachedTransform()));
		newCollider->SetBroadPhaseNode(node);
		return newCollider;
	}

	void RigidBodyWorld2D::RemoveCollider(RigidBody2D* body, Collider2D* collider)
	{
		m_CollidersToDelete.push_back(collider);
		body->OrphanCollider(collider);
	}

	void RigidBodyWorld2D::Update(F32 deltaTime, U32 velocityIters, U32 positionIters)
	{
		UpdateDeletedColliders();
		SynchronizeBroadPhase(deltaTime);

		// Update velocities based on forces.
		for (auto& bodyEntry : m_BodyList)
		{
			RigidBody2D* body = bodyEntry.GetRigidBody();
			if (body->GetType() == RigidBodyType2D::Static) continue;

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

		// Perform warm start / pre steps.
		m_BroadPhase.FindContacts([&narrowPhase = m_NarrowPhase](const PotentialContact2D& contact) { return narrowPhase.OnContactCreate(contact); });
		m_NarrowPhase.Collide();

		ContactResolverDef crDef{
			.ContactList = &m_NarrowPhase.GetContactInfoList(),
			.ContactListSize = m_NarrowPhase.GetContactsCount(),
			.WarmStartEnabled = m_WarmStartEnabled };

		ContactResolver::PreSolve(crDef);
		if (m_WarmStartEnabled)
		{
			ContactResolver::WarmStart();
		}

		// Resolve velocity constraints.
		for (U32 i = 0; i < velocityIters; i++)
		{
			ContactResolver::ResolveVelocity();
		}
		// Update positions based on velocities.
		for (auto& bodyEntry : m_BodyList)
		{
			RigidBody2D* body = bodyEntry.GetRigidBody();
			if (body->GetType() == RigidBodyType2D::Static) continue;

			glm::vec2 newPos = body->GetPosition() + body->GetLinearVelocity() * deltaTime;
			F32 deltaRot = body->GetAngularVelocity() * deltaTime;
			
			body->SetPosition(newPos);
			body->AddRotation(deltaRot);
			body->ResetForce();
			body->ResetTorque();
		}
		// Resolve position constraints. 
		for (U32 i = 0; i < positionIters; i++)
		{
			if (ContactResolver::ResolvePosition())
			{
				break;
			}
		}
		ContactResolver::PostSolve();
	}

	void RigidBodyWorld2D::SynchronizeBroadPhase(F32 deltaTime)
	{
		for (auto& rbEntry : m_BodyList)
		{
			RigidBody2D* body = rbEntry.GetRigidBody();
			for (auto& colEntry : body->GetColliderList())
			{
				Collider2D* col = colEntry.GetCollider();
				m_BroadPhase.MoveCollider(col->GetBroadPhaseNode(), col->GenerateBounds(body->GetTransform()),  body->GetLinearVelocity() * deltaTime);
			}
		}
	}

	void RigidBodyWorld2D::UpdateDeletedColliders()
	{
		m_NarrowPhase.DeleteInvalidContacts();
		for (auto* collider : m_CollidersToDelete)
		{
			I32 node = collider->GetBroadPhaseNode();
			m_BroadPhase.RemoveCollider(node);
		}
		m_CollidersToDelete.clear();
	}

}
