#pragma once

#include <glm/glm.hpp>

#include "Collision/Colliders/Bounds2D.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
	using namespace Types;

	// Used for mass, inertia and center of mass calculation, based on colliders that body has.
	struct MassInfo2D
	{
		F32 Mass{0.0f};
		F32 Inertia{0.0f};
		glm::vec2 CenterOfMass{0.0f};
	};
	
	struct DynamicsFlags2D
	{
		enum DFlags
		{
			None = 0,
			RestrictRotation = Bit(1),
			RestrictX = Bit(2), RestrictY = Bit(3), RestrictPos = RestrictX | RestrictY,
			RestrictPosAndRotation = RestrictRotation | RestrictPos,
			DisallowSleep = Bit(4),
			UseSyntheticMass = Bit(5), UseSyntheticInertia = Bit(6),
			EnableSelfCollision = Bit(7)
		};
		DFlags Flags;
		bool CheckFlag(DFlags flag) const { return (Flags & flag) == flag; }
	};

	enum class ForceMode { Force, Impulse };
	enum class SleepState { CanSleep, CannotSleep };
	enum class MassCalculation { MassAndInertia, Mass, Inertia };

	constexpr auto DD_INVALID_INDEX = std::numeric_limits<U32>::max();
	
	struct DynamicsDataDesc2D
	{
		F32 Mass{1.0f};
		F32 Inertia{1.0f};

		glm::vec2 LinearVelocity{};
		F32 LinearDamping{0.0f};
			
		F32 AngularVelocity{};
		F32 AngularDamping{0.0f};
			
		glm::vec2 Force{};
		F32 Torque{};
		F32 GravityMultiplier{1.0f};

		U32 IndexInActiveBodies{DD_INVALID_INDEX};
		U32 IslandIndex{DD_INVALID_INDEX};
		
		DynamicsFlags2D Flags{DynamicsFlags2D::None};
	};
	
	class DynamicsData2D
	{
		friend class BodyManager;
	public:
		DynamicsData2D(const DynamicsDataDesc2D& ddDesc);

		F32 GetInverseMass() const { return m_InverseMass; }
		void SetInverseMass(F32 inverseMass) { m_InverseMass = inverseMass; }
		F32 GetMass() const { return 1.0f / m_InverseMass; }
		void SetMass(F32 mass) { m_InverseMass = mass > 0.0f ? 1.0f / mass : 0.0f; }

		bool HasFiniteMass() const { return m_InverseMass > 0.0f; }
		bool HasFiniteInertia() const { return m_InverseInertia > 0.0f; }

		F32 GetInverseInertia() const { return m_InverseInertia; }
		void SetInverseInertia(F32 inverseInertia) { m_InverseInertia = inverseInertia; }
		F32 GetInertia() const { return 1.0f / m_InverseInertia; }
		void SetInertia(F32 inertia) { m_InverseInertia = inertia > 0.0f ? 1.0f / inertia : 0.0f; }

		void SetMassInfo(const MassInfo2D& massInfo) { m_InverseMass = massInfo.Mass > 0.0f ? 1.0f / massInfo.Mass : 0.0f; m_InverseInertia = massInfo.Inertia > 0.0f ? 1.0f / massInfo.Inertia : 0.0f; }
		
		const glm::vec2& GetLinearVelocity() const { return m_LinearVelocity; }
		void SetLinearVelocity(const glm::vec2& linearVelocity) { m_LinearVelocity = linearVelocity; }

		F32 GetAngularVelocity() const { return m_AngularVelocity; }
		void SetAngularVelocity(const F32 angularVelocity) { m_AngularVelocity = angularVelocity; }

		F32 GetLinearDamping() const { return m_LinearDamping; }
		void SetLinearDamping(F32 linearDamping) { m_LinearDamping = linearDamping; }
		F32 GetAngularDamping() const { return m_AngularDamping; }
		void SetAngularDamping(F32 angularDamping) { m_AngularDamping = angularDamping; }

		const glm::vec2& GetForce() const { return m_Force; }
		void SetForce(const glm::vec2& force) { m_Force = force; }
		void AddForce(const glm::vec2& force) { m_Force += force; }
		void AddForce(const glm::vec2& force, ForceMode mode) { switch (mode){ case ForceMode::Force: AddForce(force); break; case ForceMode::Impulse: m_LinearVelocity += force * m_InverseMass; break; }}
		void ResetForce() { m_Force = glm::vec2{0.0f}; }

		F32 GetTorque() const { return m_Torque; }
		void SetTorque(F32 torque) { m_Torque = torque; }
		void AddTorque(F32 torque) { m_Torque += torque; }
		void ResetTorque() { m_Torque = 0.0f; }

		F32 GetGravityMultiplier() const { return m_GravityMultiplier; }
		void SetGravityMultiplier(F32 gravityMultiplier) { m_GravityMultiplier = gravityMultiplier; }

		U32 GetIndexInActiveBodies() const { return m_IndexInActiveBodies; }
		void SetIndexInActiveBodies(U32 indexInActiveBodies) { m_IndexInActiveBodies = indexInActiveBodies; }
		U32 GetIslandIndex() const { return m_IslandIndex; }
		void SetIslandIndex(U32 islandIndex) { m_IslandIndex = islandIndex; }

		DynamicsFlags2D GetDynamicsFlags() const { return m_DynamicsFlags; }
		void SetDynamicsFlags(DynamicsFlags2D dynamicsFlags) { m_DynamicsFlags = dynamicsFlags; }

		void SetTestSpheres(const std::array<glm::vec2, 2>& points);
		SleepState CheckTestSpheres(const std::array<glm::vec2, 2>& points, F32 maxMoveDistance);
		
		bool IsInActiveBodies() const { return m_IndexInActiveBodies != DD_INVALID_INDEX; }
		bool IsInIsland() const { return m_IslandIndex != DD_INVALID_INDEX; }

	private:
		F32 m_InverseMass{1.0f};
		F32 m_InverseInertia{1.0f};

		glm::vec2 m_LinearVelocity{};
	    F32 m_AngularVelocity{};
			
	    F32 m_LinearDamping{0.0f};
	    F32 m_AngularDamping{0.0f};
			
	    glm::vec2 m_Force{};
	    F32 m_Torque{};
		F32 m_GravityMultiplier{1.0f};

		U32 m_IndexInActiveBodies{DD_INVALID_INDEX};
		U32 m_IslandIndex{DD_INVALID_INDEX};
		
		DynamicsFlags2D m_DynamicsFlags{DynamicsFlags2D::None};

		std::array<CircleBounds2D, 2> m_TestSpheres{};
	};
}
