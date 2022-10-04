#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Physics/RigidBodyEngine/PhysicsMaterial.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Collider.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	class RigidBody2D
	{
	public:
		RigidBody2D(const glm::vec3& position = glm::vec3{ 0.0f }, F32 mass = 1.0f, F32 inertia = 1.0f);

		void SetPhysicsMaterial(const PhysicsMaterial& material) { m_PhysicsMaterial = material; }
		const PhysicsMaterial& GetPhysicsMaterial() const { return m_PhysicsMaterial; }

		void SetCollider(Collider2D* collider) { m_Collider = collider; }
		Collider2D* GetCollider() { return m_Collider; }


		void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		const glm::vec3& GetPosition() const { return m_Position; }

		void SetLinearVelocity(const glm::vec2& vel) { m_LinearVelocity = vel; }
		const glm::vec2& GetLinearVelocity() const { return m_LinearVelocity; }

		void SetLinearAcceleration(const glm::vec2& acc) { m_LinearAcceleration = acc; }
		const glm::vec2& GetLinearAcceleration() const { return m_LinearAcceleration; }

		void SetMass(F32 mass) { m_InverseMass = 1.0f / mass; }
		void SetInverseMass(F32 invMass) { m_InverseMass = invMass; }
		F32 GetMass() const { return 1.0f / m_InverseMass; }
		F32 GetInverseMass() const { return m_InverseMass; }
		bool HasFiniteMass() const { return m_InverseMass != 0.0f; }
		void SetInfiniteMass() { m_InverseMass = 0.0f; }

		void SetLinearDamping(F32 damping) { m_LinearDamping = damping; }
		F32 GetLinearDamping() const { return m_LinearDamping; }

		void AddForce(const glm::vec2& force) { m_Force += force; }
		void ResetForce() { m_Force = glm::vec2{ 0.0f }; }
		const glm::vec2& GetForce() const { return m_Force; }

		void SetRotation(const glm::vec2& rotVec) { m_Rotation = rotVec; }
		void SetRotation(F32 angleRad) { m_Rotation = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) }; }
		// Requires normalization.
		void AddRotation(F32 angleRad);
		const glm::vec2& GetRotation() const { return m_Rotation; }

		void SetAngularVelocity(F32 vel) { m_AngularVelocity = vel; }
		F32 GetAngularVelocity() const { return m_AngularVelocity; }

		void SetAngularAcceleration(F32 acc) { m_AngularAcceleration = acc; }
		F32 GetAngularAcceleration() const { return m_AngularAcceleration; }

		void SetIntertiaTensor(F32 inertia) { m_InverseInertiaTensor = 1.0f / inertia; }
		void SetInverseInertiaTensor(F32 invIntertia) { m_InverseInertiaTensor = invIntertia; }
		F32 GetInertiaTensor() const { return 1.0f / m_InverseInertiaTensor; }
		F32 GetInverseInertiaTensor() const { return m_InverseInertiaTensor; }
		bool HasFiniteInertiaTensor() const { return m_InverseInertiaTensor != 0.0f; }
		void SetInfiniteInertiaTensor() { m_InverseInertiaTensor = 0.0f; }

		void SetInertia(F32 inertia) { m_InverseInertiaTensor = 1.0f / inertia; }
		void SetInverseInertia(F32 invInertia) { m_InverseInertiaTensor = invInertia; }
		F32 GetInertia() const { return 1.0f / m_InverseInertiaTensor; }
		F32 GetInverseInertia() const { return m_InverseInertiaTensor; }
		bool HasFiniteInertia() const { return m_InverseInertiaTensor != 0.0f; }
		void SetInfiniteInertia() { m_InverseInertiaTensor = 0.0f; }

		void SetAngularDamping(F32 damping) { m_AngularDamping = damping; }
		F32 GetAngularDamping() const { return m_AngularDamping; }

		void AddTorque(F32 torque) { m_Torque += torque; }
		void ResetTorque() { m_Torque = 0.0f; }
		F32 GetTorque() const { return m_Torque; }

		// Apply the given force at some point (in local space).
		void ApplyForceLocal(const glm::vec2& force, const glm::vec2& point);
		// Apply the given force at some point (in world space).
		void ApplyForce(const glm::vec2& force, const glm::vec2& point);

		glm::vec2 TransformToWorld(const glm::vec2& point);
		glm::vec2 TransformDirectionToWorld(const glm::vec2& dir);
		glm::vec2 TransformToLocal(const glm::vec2& point);
		glm::vec2 TransformDirectionToLocal(const glm::vec2& dir);
	private:
		PhysicsMaterial m_PhysicsMaterial;
		Collider2D* m_Collider;

		// vec3 for position is a bit strange, for now 3rd value is used as layer.
		glm::vec3 m_Position;
		glm::vec2 m_Rotation;

		glm::vec2 m_LinearVelocity;
		glm::vec2 m_LinearAcceleration;
		F32 m_LinearDamping;
		
		F32 m_AngularVelocity;
		F32 m_AngularAcceleration;
		F32 m_AngularDamping;
		
		glm::vec2 m_Force;
		F32 m_Torque;

		F32 m_InverseMass;
		// Tensor in 2d case is just a scalar.
		F32 m_InverseInertiaTensor;
	};
}