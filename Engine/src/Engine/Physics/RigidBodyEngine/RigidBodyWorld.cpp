#include "enginepch.h"

#include "RigidBodyWorld.h"

#include "Engine/Memory/MemoryManager.h"

namespace Engine::Physics
{
	RigidBodyWorld2D::RigidBodyWorld2D(const glm::vec2& gravity)
		: m_NarrowPhase(m_BroadPhase), m_WarmStartEnabled(true), m_Gravity(gravity)
	{
	}

	RigidBodyWorld2D::~RigidBodyWorld2D()
	{
		Clear();
	}

	void RigidBodyWorld2D::Clear()
	{
		while (m_BodyList != nullptr)
		{
			RigidBodyListEntry2D* next = m_BodyList->Next;
			Delete<RigidBody2D>(m_BodyList->Body);
			Delete<RigidBodyListEntry2D>(m_BodyList);
			m_BodyList = next;
		}
		while (m_ColliderList != nullptr)
		{
			ColliderListEntry2D* next = m_ColliderList->Next;
			Collider2D::Destroy(m_ColliderList->Collider);
			Delete<ColliderListEntry2D>(m_ColliderList);
			m_ColliderList = next;
		}
		m_NarrowPhase.Clear();
		m_BroadPhase.Clear();
		m_BroadPhaseNodesMap.clear();
		m_BroadPhaseNodesToDelete.clear();
	}

	RigidBody2D* RigidBodyWorld2D::CreateBody(const RigidBodyDef2D& rbDef)
	{
		return AddBodyToList(rbDef);
	}

	void RigidBodyWorld2D::RemoveBody(RigidBody2D* body)
	{
		RemoveBodyFromList(body);
	}

	Collider2D* RigidBodyWorld2D::SetCollider(RigidBody2D* body, const ColliderDef2D& colliderDef)
	{
		Collider2D* newCollider = AddCollider(colliderDef);
		body->SetCollider(newCollider);
		return newCollider;
	}

	Collider2D* RigidBodyWorld2D::AddCollider(const ColliderDef2D& colliderDef)
	{
		ENGINE_CORE_ASSERT(colliderDef.Collider != nullptr, "Collider is unset")
		Collider2D* newCollider = AddColliderToList(colliderDef);
		I32 nodeId = m_BroadPhase.InsertCollider(newCollider, newCollider->GenerateBounds(*newCollider->GetAttachedTransform()));
		m_BroadPhaseNodesMap[newCollider] = nodeId;
		return newCollider;
	}

	void RigidBodyWorld2D::RemoveCollider(RigidBody2D* body, Collider2D* collider)
	{
		ENGINE_CORE_ASSERT(body->GetAttachedCollider() == collider, "Body has no such collider.")
		body->OrphanCollider();
	}

	void RigidBodyWorld2D::DeleteCollider(Collider2D* collider)
	{
		if (collider->GetAttachedRigidBody()) RemoveCollider(collider->GetAttachedRigidBody(), collider);
		auto it = m_BroadPhaseNodesMap.find(collider);
		if (it != m_BroadPhaseNodesMap.end()) m_BroadPhaseNodesToDelete.push_back(collider);
		RemoveColliderFromList(collider);
		Collider2D::Destroy(collider);
	}

	RigidBody2D* RigidBodyWorld2D::AddBodyToList(const RigidBodyDef2D& rbDef)
	{
		RigidBodyListEntry2D* newEntry = New<RigidBodyListEntry2D>();
		RigidBody2D* newBody = New<RigidBody2D>(rbDef);
		newEntry->Body = newBody;
		newBody->m_BodyListEntry = newEntry;
		newEntry->Next = m_BodyList;
		if (m_BodyList != nullptr)
		{
			m_BodyList->Prev = newEntry;
		}
		m_BodyList = newEntry;

		return newBody;
	}

	void RigidBodyWorld2D::RemoveBodyFromList(RigidBody2D* body)
	{
		RigidBodyListEntry2D* listEntry = body->m_BodyListEntry;
		// Delete node.
		if (listEntry == m_BodyList)
		{
			m_BodyList = listEntry->Next;
		}
		if (listEntry->Prev != nullptr)
		{
			listEntry->Prev->Next = listEntry->Next;
		}
		if (listEntry->Next != nullptr)
		{
			listEntry->Next->Prev = listEntry->Prev;
		}
		Delete<RigidBody2D>(body);
		Delete<RigidBodyListEntry2D>(listEntry);
	}

	Collider2D* RigidBodyWorld2D::AddColliderToList(const ColliderDef2D& colliderDef)
	{
		Collider2D* newCollider = colliderDef.Clone();
		ColliderListEntry2D* newEntry = New<ColliderListEntry2D>();
		newEntry->Collider = newCollider;
		newCollider->m_ColliderListEntry2D = newEntry;
		newEntry->Next = m_ColliderList;
		if (m_ColliderList != nullptr) m_ColliderList->Prev = newEntry;
		m_ColliderList = newEntry;

		return newCollider;
	}

	void RigidBodyWorld2D::RemoveColliderFromList(Collider2D* collider)
	{
		ColliderListEntry2D* entry = collider->m_ColliderListEntry2D;
		if (entry == m_ColliderList) m_ColliderList = entry->Next;
		if (entry->Prev) entry->Prev->Next = entry->Next;
		if (entry->Next) entry->Next->Prev = entry->Prev;
		collider->SetAttachedRigidBody(nullptr);
		Delete<ColliderListEntry2D>(entry);
	}

	void RigidBodyWorld2D::Update(F32 deltaTime, U32 velocityIters, U32 positionIters)
	{
		SynchronizeBroadPhase(deltaTime);

		ApplyGlobalForces();
		m_Registry.ApplyForces();
		// Update velocities based on forces.
		for (RigidBodyListEntry2D* bodyEntry = m_BodyList; bodyEntry != nullptr; bodyEntry = bodyEntry->Next)
		{
			RigidBody2D* body = bodyEntry->Body;
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
		m_BroadPhase.FindContacts([&narrowPhase = m_NarrowPhase](const PotentialContact2D& contact) { return narrowPhase.OnPotentialContactCreate(contact); });
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
		for (U32 i = 0; i < velocityIters; i++)
		{
			ContactResolver::ResolveVelocity();
		}
		// Update positions based on velocities.
		for (RigidBodyListEntry2D* bodyEntry = m_BodyList; bodyEntry != nullptr; bodyEntry = bodyEntry->Next)
		{
			RigidBody2D* body = bodyEntry->Body;
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

	void RigidBodyWorld2D::AddForce(Ref<RigidBody2DForceGenerator> forceGenerator)
	{
		m_GlobalForces.push_back(forceGenerator);
	}

	void RigidBodyWorld2D::ApplyGlobalForces()
	{
		for (auto& globalForce : m_GlobalForces)
		{
			for (RigidBodyListEntry2D* bodyEntry = m_BodyList; bodyEntry != nullptr; bodyEntry = bodyEntry->Next)
			{
				RigidBody2D* body = bodyEntry->Body;
				globalForce->ApplyForce(*body);
			}
		}
	}

	void RigidBodyWorld2D::SynchronizeBroadPhase(F32 deltaTime)
	{
		for (auto* toDel : m_BroadPhaseNodesToDelete)
		{
			auto it = m_BroadPhaseNodesMap.find(toDel);
			ENGINE_CORE_ASSERT(it != m_BroadPhaseNodesMap.end(), "Unknown collider.")

			m_BroadPhase.RemoveCollider(it->second,
				[&narrowPhase = m_NarrowPhase](void* contact)
				{
					narrowPhase.OnPotentialContactDestroy(static_cast<ContactInfoEntry2D*>(contact));
				}
			);
			m_BroadPhaseNodesMap.erase(it);
		}
		for (auto& nodeId : m_BroadPhaseNodesMap | std::views::values)
		{
			Collider2D* collider = static_cast<Collider2D*>(m_BroadPhase.GetPayload(nodeId));
			const RigidBody2D* body = collider->GetAttachedRigidBody();
			if (body) m_BroadPhase.MoveCollider(nodeId, collider->GenerateBounds(body->GetTransform()), body->GetLinearVelocity() * deltaTime);
			else m_BroadPhase.MoveCollider(nodeId, collider->GenerateBounds(collider->GetTransform()), {0.0f, 0.0f});
		}
		m_BroadPhaseNodesToDelete.clear();
	}

	void RigidBodyWorld2D::AddForce(Ref<RigidBody2DForceGenerator> forceGenerator, RigidBody2D& body)
	{
		m_Registry.Add(forceGenerator, &body);
	}
}