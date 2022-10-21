#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Physics/RigidBodyEngine/PhysicsMaterial.h"
#include "Engine/Physics/RigidBodyEngine/Collision/Collider.h"
#include "Engine/Common/Geometry2D.h"

#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	// Note: currently the only distinction is static / non static.
	enum class RigidBodyType2D { Dynamic, Kinematic, Static };

	struct RigidBodyDef2D
	{
		enum Flags { None = 0, RestrictRotation = Bit(1) };
		struct Rotation
		{
			glm::vec2 RotationVec;
			Rotation(const glm::vec2& rotation) : RotationVec(rotation) {}
			Rotation(F32 angleRad) : RotationVec(glm::cos(angleRad), glm::sin(angleRad)) {}
		};
		glm::vec2 Position { 0.0f };
		Rotation Rotation { 0.0f };
		F32 Mass = 1.0f;
		F32 Inertia = 1.0f;
		glm::vec2 LinearVelocity { 0.0f };
		F32 AngularVelocty { 0.0f };
		F32 LinearDamping { 0.0f };
		F32 AngularDamping { 0.0f };

		RigidBodyType2D Type { RigidBodyType2D::Static };
		PhysicsMaterial PhysicsMaterial {};
		ColliderDef2D ColliderDef {};
		Flags Flags = None;
	};

	enum class ForceMode
	{
		Force, Impulse
	};

	class RigidBody2D
	{
	public:
		RigidBody2D(const RigidBodyDef2D& rbDef);
		~RigidBody2D();

		RigidBodyType2D GetType() const { return m_Type; }

		void SetPhysicsMaterial(const PhysicsMaterial& material) { m_PhysicsMaterial = material; }
		const PhysicsMaterial& GetPhysicsMaterial() const { return m_PhysicsMaterial; }

		void SetCollider(Collider2D* collider) { m_Collider = collider; }
		Collider2D* GetCollider() { return m_Collider; }

		void SetPosition(const glm::vec2& pos) { m_Position = pos; }
		const glm::vec2& GetPosition() const { return m_Position; }

		void SetLinearVelocity(const glm::vec2& vel) { m_LinearVelocity = vel; }
		const glm::vec2& GetLinearVelocity() const { return m_LinearVelocity; }

		void SetMass(F32 mass) { m_InverseMass = 1.0f / mass; }
		void SetInverseMass(F32 invMass) { m_InverseMass = invMass; }
		F32 GetMass() const { return 1.0f / m_InverseMass; }
		F32 GetInverseMass() const { return m_InverseMass; }
		bool HasFiniteMass() const { return m_InverseMass != 0.0f; }
		void SetInfiniteMass() { m_InverseMass = 0.0f; }

		void SetLinearDamping(F32 damping) { m_LinearDamping = damping; }
		F32 GetLinearDamping() const { return m_LinearDamping; }

		void AddForce(const glm::vec2& force) { m_Force += force; }
		void AddForce(const glm::vec2& force, ForceMode mode);
		void ResetForce() { m_Force = glm::vec2{ 0.0f }; }
		const glm::vec2& GetForce() const { return m_Force; }

		void SetRotation(const glm::vec2& rotVec) { m_Rotation = rotVec; }
		void SetRotation(F32 angleRad) { m_Rotation = glm::vec2{ glm::cos(angleRad), glm::sin(angleRad) }; }
		// Requires normalization.
		void AddRotation(F32 angleRad);
		const glm::vec2& GetRotation() const { return m_Rotation; }

		void SetAngularVelocity(F32 vel) { m_AngularVelocity = vel; }
		F32 GetAngularVelocity() const { return m_AngularVelocity; }

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

		// Returns tranform, associated with rigidbody.
		Transform2D GetTransform() const;
		glm::vec2 TransformToWorld(const glm::vec2& point) const;
		glm::vec2 TransformDirectionToWorld(const glm::vec2& dir) const;
		glm::vec2 TransformToLocal(const glm::vec2& point) const;
		glm::vec2 TransformDirectionToLocal(const glm::vec2& dir) const;
	private:
		RigidBodyType2D m_Type;
		PhysicsMaterial m_PhysicsMaterial;
		Collider2D* m_Collider;

		glm::vec2 m_Position;
		glm::vec2 m_Rotation;

		glm::vec2 m_LinearVelocity;
		F32 m_LinearDamping;
		
		F32 m_AngularVelocity;
		F32 m_AngularDamping;
		
		glm::vec2 m_Force;
		F32 m_Torque;

		F32 m_InverseMass;
		// Tensor in 2d case is just a scalar.
		F32 m_InverseInertiaTensor;
	};
}