#include "enginepch.h"

#include "RigidBody.h"

namespace Engine::Physics
{
	RigidBody2D::RigidBody2D(const RigidBodyDef2D& rbDef) :
		m_Position(rbDef.Position), m_Rotation(rbDef.Rotation.RotationVec),
		m_LinearVelocity(rbDef.LinearVelocity), m_LinearDamping(rbDef.LinearDamping),
		m_AngularVelocity(rbDef.AngularVelocty), m_AngularDamping(rbDef.AngularDamping),
		m_Force(0.0f), m_Torque(0.0f),
		m_InverseMass(1.0f / rbDef.Mass), m_InverseInertiaTensor(1.0f / rbDef.Inertia),
		m_Type(rbDef.Type), m_Flags(rbDef.Flags),
		m_BodyListEntry(nullptr), m_UserData(rbDef.UserData)
	{
		m_CenterOfMass = glm::vec2{ 0.0f };
		if (rbDef.Flags & RigidBodyDef2D::RestrictRotation)
		{
			m_InverseInertiaTensor = 0.0f;
		}
		switch (m_Type)
		{
		case RigidBodyType2D::Dynamic:
			break;
		case RigidBodyType2D::Kinematic:
			break;
		case RigidBodyType2D::Static:
			m_InverseMass = 0.0f;
			m_InverseInertiaTensor = 0.0f;
			break;
		}
		m_ColliderList = nullptr;
	}

	RigidBody2D::~RigidBody2D()
	{
		while (m_ColliderList != nullptr)
		{
			ColliderListEntry2D* next = m_ColliderList->Next;
			Collider2D::Destroy(m_ColliderList->Collider);
			Delete<ColliderListEntry2D>(m_ColliderList);
			m_ColliderList = next;
		}
	}

	Collider2D* RigidBody2D::AddCollider(const ColliderDef2D& colDef)
	{
		Collider2D* newCollider = colDef.Clone();
		newCollider->SetAttachedRigidBody(this);
		ColliderListEntry2D* newEntry = New<ColliderListEntry2D>();
		newEntry->Collider = newCollider;
		newCollider->m_ColliderListEntry2D = newEntry;
		newEntry->Next = m_ColliderList;
		if (m_ColliderList != nullptr)
		{ 
			m_ColliderList->Prev = newEntry;
		}
		m_ColliderList = newEntry;

		if ((m_Flags & RigidBodyDef2D::UseSyntheticMass) != 0) return newEntry->Collider;
		if (m_Type != RigidBodyType2D::Dynamic) return newEntry->Collider;

		RecalculateMass();

		// Very smart visual studio.
		if (newEntry == nullptr) return nullptr;
		return newEntry->Collider;
	}

	void RigidBody2D::RemoveCollider(Collider2D* collider)
	{
		ColliderListEntry2D* entry = collider->m_ColliderListEntry2D;
		if (entry == m_ColliderList) m_ColliderList = entry->Next;
		if (entry->Prev) entry->Prev->Next = entry->Next;
		if (entry->Next) entry->Next->Prev = entry->Prev;
		Collider2D::Destroy(collider);
		Delete<ColliderListEntry2D>(entry);

		if ((m_Flags & RigidBodyDef2D::UseSyntheticMass) != 0) return;
		if (m_Type != RigidBodyType2D::Dynamic) return;
		RecalculateMass();
	}

	void RigidBody2D::RecalculateMass()
	{
		m_InverseMass = m_InverseInertiaTensor = 0.0f;
		m_CenterOfMass = glm::vec2{ 0.0f };

		F32 mass = 0.0f;
		F32 inertia = 0.0f;
		for (ColliderListEntry2D* colliderEntry = m_ColliderList; colliderEntry != nullptr; colliderEntry = colliderEntry->Next)
		{
			// Sensors do not contribute to mass.
			if (colliderEntry->Collider->IsSensor()) continue;
			MassInfo2D massInfo = colliderEntry->Collider->CalculateMass();
			mass += massInfo.Mass;
			inertia += massInfo.Inertia;
			m_CenterOfMass += massInfo.CenterOfMass * massInfo.Mass;
		}
		if (mass > 0.0f)
		{
			m_InverseMass = 1.0f / mass;
			m_CenterOfMass *= m_InverseMass;
		}

		if (inertia > 0.0f && (m_Flags & RigidBodyDef2D::RestrictRotation) == 0)
		{
			inertia -= mass * glm::dot(m_CenterOfMass, m_CenterOfMass);
			m_InverseInertiaTensor = 1.0f / inertia;
		}

	}

	void RigidBody2D::AddForce(const glm::vec2& force, ForceMode mode)
	{
		switch (mode)
		{
		case ForceMode::Force: 
			AddForce(force);
			break;
		case ForceMode::Impulse: 
			m_LinearVelocity += force * m_InverseMass;
			break;
		}
	}

	void RigidBody2D::AddRotation(F32 angleRad)
	{
		glm::vec2 delta = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) };
		m_Rotation = {
			m_Rotation.x * delta.x - m_Rotation.y * delta.y,
			m_Rotation.x * delta.y + m_Rotation.y * delta.x
		};
	}

	void RigidBody2D::ApplyForceLocal(const glm::vec2& force, const glm::vec2& point)
	{
		// Transform local space to world space.
		glm::vec2 transformedPoint = { point.x * m_Rotation.x - point.y * m_Rotation.y, point.x * m_Rotation.y + point.y * m_Rotation.x };
		AddForce(force);
		AddTorque(transformedPoint.x * force.y - transformedPoint.y * force.x);
	}

	void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
	{
		AddForce(force);
		AddTorque((point.x - m_CenterOfMass.x) * force.y - (point.y - m_CenterOfMass.y) * force.x);
	}
	
	Transform2D RigidBody2D::GetTransform() const
	{
		return { .Translation {m_Position.x, m_Position.y}, .Rotation{m_Rotation.x, m_Rotation.y} };
	}

	glm::vec2 RigidBody2D::TransformToWorld(const glm::vec2& point) const
	{
		return GetTransform().Transform(point);
	}
	
	glm::vec2 RigidBody2D::TransformDirectionToWorld(const glm::vec2& dir) const
	{
		return GetTransform().TransformDirection(dir);
	}

	glm::vec2 RigidBody2D::TransformToLocal(const glm::vec2& point) const
	{
		return GetTransform().InverseTransform(point);
	}

	glm::vec2 RigidBody2D::TransformDirectionToLocal(const glm::vec2& dir) const
	{
		return GetTransform().InverseTransformDirection(dir);
	}	
}