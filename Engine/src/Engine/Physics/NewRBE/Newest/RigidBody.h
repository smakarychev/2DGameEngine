#pragma once

#include "CollisionLayer.h"
#include "DynamicsData.h"
#include "Engine/Math/LinearAlgebra.h"
#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    class DynamicsData2D;
    
    
    enum class BodyType { Static, Dynamic, Kinematic };
    enum class StartUpBehaviour { SetActive, SetInactive }; // TODO: move to body manager.
    
    struct RigidBodyDesc2D
    {
        Transform2D Transform{};
        BodyType BodyType{BodyType::Static};
        CollisionLayer CollisionLayer{CL_INVALID_LAYER};
    };

    using RigidBodyId2D = U32;
    static constexpr RigidBodyId2D RB_INVALID_ID = std::numeric_limits<U32>::max();
    
    class RigidBody2D
    {
    public:
        RigidBody2D(const RigidBodyDesc2D& rbDesc);
        
        RigidBodyId2D GetId() const { return m_Id; }
        
        BodyType GetType() const { return m_Type; }
        bool IsStatic() const { return m_Type == BodyType::Static; }
        bool IsDynamic() const { return m_Type == BodyType::Dynamic; }
        bool IsKinematic() const { return m_Type == BodyType::Kinematic; }

		Collider2D* GetCollider() const { return m_Collider; }
    	void SetCollider(Collider2D* collider) { m_Collider = collider; }
    	
    	void SetUserData(void* userData) { m_UserData = userData; }
    	void* GetUserData() const { return m_UserData; }

    	const glm::vec2& GetCenterOfMass() const { return m_CenterOfMass; }
    	void RecalculateCenterOfMass();

        void SetPosition(const glm::vec2& pos) { m_Transform.Position = pos; }
    	const glm::vec2& GetPosition() const { return m_Transform.Position; }
    	void SetRotation(const glm::vec2& rotVec) { m_Transform.Rotation = rotVec; }
    	void SetRotation(F32 angleRad) { m_Transform.Rotation = angleRad; }
    	const glm::vec2& GetRotation() const { return m_Transform.Rotation; }
    	void AddRotation(F32 angleRad) { m_Transform.Rotation = Math::CombineRotation(m_Transform.Rotation, Rotation(angleRad)); }

    	void SetTransform(const Transform2D& transform) { m_Transform = transform; }
    	const Transform2D& GetTransform() const { return m_Transform; }
    	Transform2D& GetTransform() { return m_Transform; }
    	glm::vec2 TransformToWorld(const glm::vec2& point) const;
    	glm::vec2 TransformDirectionToWorld(const glm::vec2& dir) const;
    	glm::vec2 TransformToLocal(const glm::vec2& point) const;
    	glm::vec2 TransformDirectionToLocal(const glm::vec2& dir) const;

        CollisionLayer GetCollisionLayer() const { return m_CollisionLayer; }
        void SetDynamicsData(DynamicsData2D* data) { m_DynamicsData = data; }

    	const AABB2D& GetBounds() const { return m_Bounds; }
    	void SetBounds(const AABB2D& bounds) { m_Bounds = bounds; }
    	void RecalculateBounds();
    	
    	// **** Dynamics data related **********************************************
        const DynamicsData2D& GetDynamicsData() const;
        DynamicsData2D& GetDynamicsData();
    	
    	bool IsInActiveBodies() const;
        bool IsInIsland() const;
    	bool CanSleep() const;

        const glm::vec2& GetLinearVelocity() const;
        const glm::vec2& GetLinearVelocityU() const;
        void SetLinearVelocity(const glm::vec2& linearVelocity);

        F32 GetAngularVelocity() const;
        F32 GetAngularVelocityU() const;
        void SetAngularVelocity(F32 angularVelocity);

        F32 GetLinearDamping() const;
        F32 GetLinearDampingU() const;
        void SetLinearDamping(F32 linearDamping);
        F32 GetAngularDamping() const;
        F32 GetAngularDampingU() const;
        void SetAngularDamping(F32 angularDamping);

        const glm::vec2& GetForce() const;
        const glm::vec2& GetForceU() const;
        void SetForce(const glm::vec2& force);
        void AddForce(const glm::vec2& force);
        void AddForce(const glm::vec2& force, ForceMode mode);
        void ResetForce();
        // Apply the given force at some point (in local space).
    	void ApplyForceLocal(const glm::vec2& force, const glm::vec2& point);
    	// Apply the given force at some point (in world space).
    	void ApplyForce(const glm::vec2& force, const glm::vec2& point);

    	F32 GetTorque() const;
        F32 GetTorqueU() const;
        void SetTorque(F32 torque);
        void AddTorque(F32 torque);
        void ResetTorque();

        F32 GetGravityMultiplier() const;
        F32 GetGravityMultiplierU() const;
        void SetGravityMultiplier(F32 gravityMultiplier);

        U32 GetIndexInActiveBodies() const;
        U32 GetIndexInActiveBodiesU() const;
        void SetIndexInActiveBodies(U32 indexInActiveBodies);
        U32 GetIslandIndex() const;
        U32 GetIslandIndexU() const;
        void SetIslandIndex(U32 islandIndex);

        DynamicsFlags2D GetDynamicsFlags() const;
        DynamicsFlags2D GetDynamicsFlagsU() const;
        void SetDynamicsFlags(DynamicsFlags2D dynamicsFlags);
        // **** Dynamics data related **********************************************
    private:
        BodyType m_Type{BodyType::Static};
        RigidBodyId2D m_Id{RB_INVALID_ID};

    	Transform2D m_Transform{};
    	glm::vec2 m_CenterOfMass{0.0f};
    	
        DynamicsData2D* m_DynamicsData{nullptr};
    	Collider2D* m_Collider{nullptr};

    	CollisionLayer m_CollisionLayer{CL_INVALID_LAYER};

    	// Bounds in world space.
    	AABB2D m_Bounds{}; 

		void* m_UserData{nullptr};
    	
        //TODO: collider.
    };
}


