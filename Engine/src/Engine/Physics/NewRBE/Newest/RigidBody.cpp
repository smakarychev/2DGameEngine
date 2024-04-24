#include "enginepch.h"

#include "RigidBody.h"

#include "Collision/Colliders/Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    RigidBody2D::RigidBody2D(const RigidBodyDesc2D& rbDesc)
        : m_Type(rbDesc.BodyType), m_Transform(rbDesc.Transform), m_CollisionLayer(rbDesc.CollisionLayer)
    {
    }
    
    void RigidBody2D::RecalculateBounds()
    {
        ENGINE_CORE_ASSERT(m_Collider != nullptr, "Body without collider has no bounds.")
        m_Bounds = m_Collider->GenerateBounds(m_Transform);
        
        glm::vec2 largestAxis = m_Bounds.HalfSize.x > m_Bounds.HalfSize.y ?
            glm::vec2{m_Bounds.HalfSize.x, 0.0f} :
            glm::vec2{0.0f, m_Bounds.HalfSize.y};
        m_DynamicsData->SetTestSpheres({m_Bounds.Center, m_Bounds.Center + largestAxis});
    }

    void RigidBody2D::RecalculateMass()
    {
        MassInfo2D mi = m_Collider->CalculateMass();
        m_DynamicsData->SetMassInfo(mi);
    }

    void RigidBody2D::RecalculateCenterOfMass()
    {
        if (m_Collider != nullptr) m_CenterOfMass = m_Collider->GetCenterOfMass();
        else m_CenterOfMass = glm::vec2{0.0f}; 
    }

    const DynamicsData2D& RigidBody2D::GetDynamicsData() const
    {
        ENGINE_CORE_ASSERT(m_Type != BodyType::Static, "Body is static.")
        ENGINE_CORE_ASSERT(m_DynamicsData != nullptr, "Dynamics data is unset.")
        return *m_DynamicsData;
    }

    DynamicsData2D& RigidBody2D::GetDynamicsData()
    {
        ENGINE_CORE_ASSERT(m_Type != BodyType::Static, "Body is static.")
        ENGINE_CORE_ASSERT(m_DynamicsData != nullptr, "Dynamics data is unset.")
        return *m_DynamicsData;
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

    bool RigidBody2D::IsInActiveBodies() const
    {
        return m_DynamicsData == nullptr ? false : m_DynamicsData->IsInActiveBodies();
    }

    bool RigidBody2D::IsInIsland() const
    {
        return m_DynamicsData == nullptr ? false : m_DynamicsData->IsInIsland();
    }

    bool RigidBody2D::CanSleep() const
    {
        if (m_Type == BodyType::Static) return true;
        ENGINE_CORE_ASSERT(m_DynamicsData != nullptr, "Rigid and Kinematic bodies must have DynamicsData attached.")
        return !m_DynamicsData->GetDynamicsFlags().CheckFlag(DynamicsFlags2D::DisallowSleep);
    }

    const glm::vec2& RigidBody2D::GetLinearVelocity() const
    {
        static constexpr auto nullVec2 =glm::vec2{0.0f}; 
        return m_DynamicsData == nullptr ? nullVec2 : m_DynamicsData->GetLinearVelocity();
    }

    const glm::vec2& RigidBody2D::GetLinearVelocityU() const
    {
        return m_DynamicsData->GetLinearVelocity();
    }

    void RigidBody2D::SetLinearVelocity(const glm::vec2& linearVelocity)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetLinearVelocity(linearVelocity);
    }

    F32 RigidBody2D::GetAngularVelocity() const
    {
        return m_DynamicsData == nullptr ? 0.0f : m_DynamicsData->GetAngularVelocity();
    }

    F32 RigidBody2D::GetAngularVelocityU() const
    {
        return m_DynamicsData->GetAngularVelocity();
    }

    void RigidBody2D::SetAngularVelocity(F32 angularVelocity)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetAngularVelocity(angularVelocity);
    }

    F32 RigidBody2D::GetLinearDamping() const
    {
        return m_DynamicsData == nullptr ? 0.0f : m_DynamicsData->GetLinearDamping();
    }

    F32 RigidBody2D::GetLinearDampingU() const
    {
        return m_DynamicsData->GetLinearDamping();
    }

    void RigidBody2D::SetLinearDamping(F32 linearDamping)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetLinearDamping(linearDamping);
    }

    F32 RigidBody2D::GetAngularDamping() const
    {
        return m_DynamicsData == nullptr ? 0.0f : m_DynamicsData->GetAngularDamping();
    }

    F32 RigidBody2D::GetAngularDampingU() const
    {
        return m_DynamicsData->GetAngularDamping();
    }

    void RigidBody2D::SetAngularDamping(F32 angularDamping)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetAngularDamping(angularDamping);
    }

    const glm::vec2& RigidBody2D::GetForce() const
    {
        static constexpr auto nullVec2 =glm::vec2{0.0f}; 
        return m_DynamicsData == nullptr ? nullVec2 : m_DynamicsData->GetForce();
    }

    const glm::vec2& RigidBody2D::GetForceU() const
    {
        return m_DynamicsData->GetForce();
    }

    void RigidBody2D::SetForce(const glm::vec2& force)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetForce(force);
    }

    void RigidBody2D::AddForce(const glm::vec2& force)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->AddForce(force);
    }

    void RigidBody2D::AddForce(const glm::vec2& force, ForceMode mode)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->AddForce(force, mode);
    }

    void RigidBody2D::ResetForce()
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->ResetForce();
    }

    void RigidBody2D::ApplyForceLocal(const glm::vec2& force, const glm::vec2& point)
    {
        // Transform local space to world space.
        glm::vec2 transformedPoint = TransformToWorld(point);
        ApplyForce(force, transformedPoint);
    }

    void RigidBody2D::ApplyForce(const glm::vec2& force, const glm::vec2& point)
    {
        const glm::vec2& CoM = GetCenterOfMass(); 
        AddForce(force);
        AddTorque((point.x - CoM.x) * force.y - (point.y - CoM.y) * force.x);
    }

    F32 RigidBody2D::GetTorque() const
    {
        return m_DynamicsData == nullptr ? 0.0f : m_DynamicsData->GetTorque();
    }

    F32 RigidBody2D::GetTorqueU() const
    {
        return m_DynamicsData->GetTorque();
    }

    void RigidBody2D::SetTorque(F32 torque)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetTorque(torque);
    }

    void RigidBody2D::AddTorque(F32 torque)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->AddTorque(torque);
    }

    void RigidBody2D::ResetTorque()
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->ResetTorque();
    }

    F32 RigidBody2D::GetGravityMultiplier() const
    {
        return m_DynamicsData == nullptr ? 0.0f : m_DynamicsData->GetGravityMultiplier();
    }

    F32 RigidBody2D::GetGravityMultiplierU() const
    {
        return m_DynamicsData->GetGravityMultiplier();
    }

    void RigidBody2D::SetGravityMultiplier(F32 gravityMultiplier)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetGravityMultiplier(gravityMultiplier);
    }

    U32 RigidBody2D::GetIndexInActiveBodies() const
    {
        return m_DynamicsData == nullptr ? DD_INVALID_INDEX : m_DynamicsData->GetIndexInActiveBodies();
    }

    U32 RigidBody2D::GetIndexInActiveBodiesU() const
    {
        return m_DynamicsData->GetIndexInActiveBodies();
    }

    void RigidBody2D::SetIndexInActiveBodies(U32 indexInActiveBodies)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetIndexInActiveBodies(indexInActiveBodies);
    }

    U32 RigidBody2D::GetIslandIndex() const
    {
        return m_DynamicsData == nullptr ? DD_INVALID_INDEX : m_DynamicsData->GetIslandIndex();
    }

    U32 RigidBody2D::GetIslandIndexU() const
    {
        return m_DynamicsData->GetIslandIndex();
    }

    void RigidBody2D::SetIslandIndex(U32 islandIndex)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetIslandIndex(islandIndex);
    }

    DynamicsFlags2D RigidBody2D::GetDynamicsFlags() const
    {
        return m_DynamicsData == nullptr ? DynamicsFlags2D{ DynamicsFlags2D::None } : m_DynamicsData->GetDynamicsFlags();
    }

    DynamicsFlags2D RigidBody2D::GetDynamicsFlagsU() const
    {
        return m_DynamicsData->GetDynamicsFlags();
    }

    void RigidBody2D::SetDynamicsFlags(DynamicsFlags2D dynamicsFlags)
    {
        ENGINE_CORE_ASSERT(m_DynamicsData, "DynamicsData unset.")
        m_DynamicsData->SetDynamicsFlags(dynamicsFlags);
    }
}
