#include "enginepch.h"

#include "RigidBody.h"

namespace Engine
{
	RigidBody2D::RigidBody2D(const RigidBodyDef2D& rbDef) :
		m_Position(rbDef.Position), m_Rotation(rbDef.Rotation.RotationVec),
		m_LinearVelocity(rbDef.LinearVelocity), m_LinearDamping(rbDef.LinearDamping),
		m_AngularVelocity(rbDef.AngularVelocty), m_AngularDamping(rbDef.AngularDamping),
		m_Force(0.0f), m_Torque(0.0f),
		m_InverseMass(1.0f / rbDef.Mass), m_InverseInertiaTensor(1.0f / rbDef.Inertia),
		m_Type(rbDef.Type),
		m_Collider(rbDef.ColliderDef.Collider->Clone()),
		m_PhysicsMaterial(rbDef.PhysicsMaterial)
	{
		if (rbDef.Flags & RigidBodyDef2D::RestrictRotation)
		{
			m_InverseInertiaTensor = 0.0f;
		}
		switch (m_Type)
		{
		case Engine::RigidBodyType2D::Dynamic:
			break;
		case Engine::RigidBodyType2D::Kinematic:
			break;
		case Engine::RigidBodyType2D::Static:
			m_InverseMass = 0.0f;
			m_InverseInertiaTensor = 0.0f;
			break;
		}
	}

	RigidBody2D::~RigidBody2D()
	{
		if (m_Collider != nullptr)
		{
			Collider2D::Destroy(m_Collider);
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
		AddTorque((point.x - m_Position.x) * force.y - (point.y - m_Position.y) * force.x);
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


