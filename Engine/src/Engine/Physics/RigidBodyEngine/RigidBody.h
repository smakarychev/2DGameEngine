#pragma once

#include "Engine/Core/Types.h"

#include "Engine/Physics/RigidBodyEngine/Collision/Collider.h"

#include <glm/glm.hpp>


namespace Engine::Physics
{
	using namespace Types;

	// Note: currently the only distinction is static / non static.
	enum class RigidBodyType2D { Dynamic, Kinematic, Static, None };

	// Used for mass, inertia and center of mass calculation,
	// based on colliders that body has.
	struct MassInfo2D
	{
		F32 Mass;
		F32 Inertia;
		glm::vec2 CenterOfMass;
	};

	struct RigidBodyDef2D
	{
		enum BodyFlags { None = 0, RestrictRotation = Bit(1), UseSyntheticMass = Bit(2)	};
		Component::LocalToWorldTransform2D* AttachedTransform{nullptr};
		F32 Mass = 1.0f;
		F32 Inertia = 1.0f;
		glm::vec2 LinearVelocity { 0.0f };
		F32 AngularVelocity { 0.0f };
		F32 LinearDamping { 0.0f };
		F32 AngularDamping { 0.0f };

		RigidBodyType2D Type { RigidBodyType2D::Static };
		BodyFlags Flags = None;
		void* UserData = nullptr;
	};

	enum class ForceMode
	{
		Force, Impulse
	};

	class RigidBody2D;
	struct RigidBodyListEntry2D
	{
		RigidBody2D* Body = nullptr;
		RigidBodyListEntry2D* Next = nullptr;
		RigidBodyListEntry2D* Prev = nullptr;
	};

	class RigidBody2D
	{
		using ColliderList = ColliderListEntry2D;
		friend class RigidBodyWorld2D;
		FRIEND_MEMORY_FN
	public:
		void SetType(RigidBodyType2D type) { m_Type = type; }
		RigidBodyType2D GetType() const { return m_Type; }
		void SetUserData(void* userData) { m_UserData = userData; }
   		void* GetUserData() const { return m_UserData; }
		void SetFlags(RigidBodyDef2D::BodyFlags flags) { m_Flags = flags; }
		RigidBodyDef2D::BodyFlags GetFlags() const { return m_Flags; }

		Collider2D* GetAttachedCollider() { return m_AttachedCollider; }

		void SetPosition(const glm::vec2& pos);
		const glm::vec2& GetPosition() const;

		const glm::vec2& GetCenterOfMass() const { return m_CenterOfMass; }

		void SetLinearVelocity(const glm::vec2& vel) { m_LinearVelocity = vel; }
		const glm::vec2& GetLinearVelocity() const { return m_LinearVelocity; }

		void RecalculateMass();
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

		void SetRotation(const glm::vec2& rotVec);
		void SetRotation(F32 angleRad);
		void AddRotation(F32 angleRad);
		const glm::vec2& GetRotation() const;

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

		void SetAttachedTransform(Component::LocalToWorldTransform2D* transform);
		// Returns transform, associated with rigidbody.
		Component::LocalToWorldTransform2D& GetTransform() const;
		glm::vec2 TransformToWorld(const glm::vec2& point) const;
		glm::vec2 TransformDirectionToWorld(const glm::vec2& dir) const;
		glm::vec2 TransformToLocal(const glm::vec2& point) const;
		glm::vec2 TransformDirectionToLocal(const glm::vec2& dir) const;
	private:
		RigidBody2D(const RigidBodyDef2D& rbDef);
		~RigidBody2D();
		void SetCollider(Collider2D* collider);
		void OrphanCollider();
	private:
		RigidBodyType2D m_Type;
		RigidBodyListEntry2D* m_BodyListEntry;
		RigidBodyDef2D::BodyFlags m_Flags;
		void* m_UserData{nullptr};

		Collider2D* m_AttachedCollider{nullptr};
		
		Component::LocalToWorldTransform2D* m_AttachedTransform{nullptr};
		
		glm::vec2 m_CenterOfMass{};

		glm::vec2 m_LinearVelocity{};
		F32 m_LinearDamping{};
		
		F32 m_AngularVelocity{};
		F32 m_AngularDamping{};
		
		glm::vec2 m_Force{};
		F32 m_Torque{};

		F32 m_InverseMass{};
		// Tensor in 2d case is just a scalar.
		F32 m_InverseInertiaTensor{};
	};
}
