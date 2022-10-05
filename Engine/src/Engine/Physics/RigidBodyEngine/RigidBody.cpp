#include "enginepch.h"

#include "RigidBody.h"

namespace Engine
{
	RigidBody2D::RigidBody2D(const glm::vec3& position, F32 mass, F32 inertia) :
		m_Position(position), m_Rotation(1.0f, 0.0f),
		m_LinearVelocity(0.0f), m_LinearAcceleration(0.0f), m_LinearDamping(0.0f),
		m_AngularVelocity(0.0f), m_AngularAcceleration(0.0f), m_AngularDamping(0.0f),
		m_Force(0.0f), m_Torque(0.0f),
		m_InverseMass(1.0f / mass), m_InverseInertiaTensor(1.0f / inertia),
		m_Collider(nullptr)
	{
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
		// TODO: scale?
		glm::vec2 transformedPoint = { point.x * m_Rotation.x - point.y * m_Rotation.y, point.x * m_Rotation.y + point.y * m_Rotation.x };
		AddForce(force);
		AddTorque(transformedPoint.x * force.y - transformedPoint.y * force.x);
	}

	void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
	{
		AddForce(force);
		AddTorque((point.x - m_Position.x) * force.y - (point.y - m_Position.y) * force.x);
	}
	
	glm::vec2 RigidBody2D::TransformToWorld(const glm::vec2& point)
	{
		return glm::vec2{
			point.x * m_Rotation.x - point.y * m_Rotation.y + m_Position.x, 
			point.x * m_Rotation.y + point.y * m_Rotation.x + m_Position.y
		};
	}
	
	glm::vec2 RigidBody2D::TransformDirectionToWorld(const glm::vec2& dir)
	{
		return glm::vec2{
			dir.x * m_Rotation.x - dir.y * m_Rotation.y,
			dir.x * m_Rotation.y + dir.y * m_Rotation.x
		};
	}

	glm::vec2 RigidBody2D::TransformToLocal(const glm::vec2& point)
	{
		glm::vec2 translated = point - glm::vec2(m_Position);
		return glm::vec2{
			 translated.x * m_Rotation.x + translated.y * m_Rotation.y,
			-translated.x * m_Rotation.y + translated.y * m_Rotation.x
		};
	}

	glm::vec2 RigidBody2D::TransformDirectionToLocal(const glm::vec2& dir)
	{
		return glm::vec2{
			 dir.x * m_Rotation.x + dir.y * m_Rotation.y,
			-dir.x * m_Rotation.y + dir.y * m_Rotation.x
		};
	}
	
	
}


