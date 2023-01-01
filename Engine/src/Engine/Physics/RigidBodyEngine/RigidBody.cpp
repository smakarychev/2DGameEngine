#include "enginepch.h"

#include "RigidBody.h"

#include "Engine/ECS/Components.h"
#include "Engine/Math/LinearAlgebra.h"

namespace Engine::Physics
{
	RigidBody2D::RigidBody2D(const RigidBodyDef2D& rbDef)
		:
		m_Type(rbDef.Type),
		m_BodyListEntry(nullptr), m_Flags(rbDef.Flags),
		m_UserData(rbDef.UserData), m_AttachedTransform(rbDef.AttachedTransform),
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
		m_AttachedCollider = nullptr;
	}

	RigidBody2D::~RigidBody2D()
	{
		OrphanCollider();
	}

	void RigidBody2D::SetCollider(Collider2D* collider)
	{
		if (m_AttachedCollider) OrphanCollider();
		collider->SetAttachedRigidBody(this);
		m_AttachedCollider = collider;
		if ((m_Flags & RigidBodyDef2D::UseSyntheticMass) != 0) return;
		if (m_Type != RigidBodyType2D::Dynamic) return;

		RecalculateMass();
	}

	void RigidBody2D::OrphanCollider()
	{
		m_AttachedCollider->SetAttachedRigidBody(nullptr);
	}

	void RigidBody2D::SetPosition(const glm::vec2& pos)	{ m_AttachedTransform->Position = pos; }

	const glm::vec2& RigidBody2D::GetPosition() const { return m_AttachedTransform->Position; }

	void RigidBody2D::RecalculateMass()
	{
		m_InverseMass = m_InverseInertiaTensor = 0.0f;
		m_CenterOfMass = glm::vec2{ 0.0f };
		if (m_Type != RigidBodyType2D::Dynamic) return;;
		
		F32 mass = 0.0f;
		F32 inertia = 0.0f;
		
		// Sensors do not contribute to mass.
		if (m_AttachedCollider->IsSensor()) return;
		MassInfo2D massInfo = m_AttachedCollider->CalculateMass();
		mass += massInfo.Mass;
		inertia += massInfo.Inertia;
		m_CenterOfMass += massInfo.CenterOfMass * massInfo.Mass;
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

	void RigidBody2D::SetRotation(const glm::vec2& rotVec) { m_AttachedTransform->Rotation = rotVec; }

	void RigidBody2D::SetRotation(F32 angleRad)	{ m_AttachedTransform->Rotation = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) }; }

	void RigidBody2D::AddRotation(F32 angleRad)
	{
		glm::vec2 delta = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) };
		m_AttachedTransform->Rotation = Math::CombineRotation(m_AttachedTransform->Rotation, delta);
	}

	const glm::vec2& RigidBody2D::GetRotation() const { return m_AttachedTransform->Rotation; }

	void RigidBody2D::ApplyForceLocal(const glm::vec2& force, const glm::vec2& point)
	{
		// Transform local space to world space.
		glm::vec2 transformedPoint = TransformToWorld(point);
		AddForce(force);
		AddTorque(transformedPoint.x * force.y - transformedPoint.y * force.x);
	}

	void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
	{
		AddForce(force);
		AddTorque((point.x - m_CenterOfMass.x) * force.y - (point.y - m_CenterOfMass.y) * force.x);
	}

	void RigidBody2D::SetAttachedTransform(Component::LocalToWorldTransform2D* transform)
	{
		m_AttachedTransform = transform;
	}

	Component::LocalToWorldTransform2D& RigidBody2D::GetTransform() const
	{
		return *m_AttachedTransform;
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
