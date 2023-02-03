#include "enginepch.h"

#include "RigidBody.h"

#include "Engine/ECS/Components.h"
#include "Engine/Math/LinearAlgebra.h"

namespace Engine::WIP::Physics
{
	RigidBody2D::RigidBody2D(const RigidBodyDef2D& rbDef)
		:
		m_Type(rbDef.Type),
		m_BodyListEntry(nullptr), m_Flags(rbDef.Flags),
		m_UserData(rbDef.UserData), m_Transform(rbDef.Transform),
		m_LinearVelocity(rbDef.LinearVelocity), m_LinearDamping(rbDef.LinearDamping),
		m_AngularVelocity(rbDef.AngularVelocity), m_AngularDamping(rbDef.AngularDamping),
		m_Force(0.0f), m_Torque(0.0f),
		m_InverseMass(1.0f / rbDef.Mass), m_InverseInertiaTensor(1.0f / rbDef.Inertia)
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
	}

	RigidBody2D::~RigidBody2D()
	{
		OrphanColliders();
	}

	Collider2D* RigidBody2D::AddCollider(const ColliderDef2D& colDef)
	{
		Collider2D* newCollider = m_ColliderList.Push(colDef);
		newCollider->SetAttachedRigidBody(this);
		
		if ((m_Flags & RigidBodyDef2D::UseSyntheticMass) == 0 && m_Type == RigidBodyType2D::Dynamic) RecalculateMass();
		return newCollider;
	}

	void RigidBody2D::RemoveCollider(Collider2D* collider)
	{
		m_ColliderList.Pop(collider);
	}

	void RigidBody2D::OrphanColliders()
	{
		for (auto& colEntry : m_ColliderList) colEntry.GetCollider()->SetAttachedRigidBody(nullptr);
	}

	void RigidBody2D::OrphanCollider(Collider2D* collider)
	{
		ENGINE_CORE_ASSERT(collider->GetAttachedRigidBody() == this, "Foreign collider.")
		collider->SetAttachedRigidBody(nullptr);
	}

	void RigidBody2D::SetPosition(const glm::vec2& pos)	{ m_Transform.Position = pos; }

	const glm::vec2& RigidBody2D::GetPosition() const { return m_Transform.Position; }

	void RigidBody2D::RecalculateMass()
	{
		if (m_Flags & RigidBodyDef2D::UseSyntheticMass) return;
		m_InverseMass = m_InverseInertiaTensor = 0.0f;
		m_CenterOfMass = glm::vec2{ 0.0f };
		if (m_Type != RigidBodyType2D::Dynamic) return;

		F32 mass = 0.0f;
		F32 inertia = 0.0f;
		for (auto& colEntry : m_ColliderList)
		{
			// Sensors do not contribute to mass.
			if (colEntry.GetCollider()->IsSensor()) continue;
			MassInfo2D massInfo = colEntry.GetCollider()->CalculateMass();
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

	void RigidBody2D::SetRotation(const glm::vec2& rotVec) { m_Transform.Rotation = rotVec; }

	void RigidBody2D::SetRotation(F32 angleRad)	{ m_Transform.Rotation = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) }; }

	void RigidBody2D::AddRotation(F32 angleRad)
	{
		glm::vec2 delta = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) };
		m_Transform.Rotation = Math::CombineRotation(m_Transform.Rotation, delta);
	}

	const glm::vec2& RigidBody2D::GetRotation() const { return m_Transform.Rotation; }

	void RigidBody2D::ApplyForceLocal(const glm::vec2& force, const glm::vec2& point)
	{
		// Transform local space to world space.
		glm::vec2 transformedPoint = TransformToWorld(point);
		ApplyForce(force, transformedPoint);
	}

	void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
	{
		AddForce(force);
		AddTorque((point.x - m_CenterOfMass.x) * force.y - (point.y - m_CenterOfMass.y) * force.x);
	}

	void RigidBody2D::SetTransform(const Transform2D& transform)
	{
		m_Transform = transform;
	}

	const Transform2D& RigidBody2D::GetTransform()const
	{
		return m_Transform;
	}

	Transform2D& RigidBody2D::GetTransform()
	{
		return m_Transform;
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
