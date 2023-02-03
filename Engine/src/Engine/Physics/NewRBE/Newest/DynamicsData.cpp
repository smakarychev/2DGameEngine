#include "enginepch.h"

#include "DynamicsData.h"

namespace Engine::WIP::Physics::Newest
{
    DynamicsData2D::DynamicsData2D(const DynamicsDataDesc2D& ddDesc)
        : m_InverseMass(1.0f / ddDesc.Mass), m_InverseInertia(1.0f / ddDesc.Inertia),
        m_LinearVelocity(ddDesc.LinearVelocity), m_AngularVelocity(ddDesc.AngularVelocity),
        m_LinearDamping(ddDesc.LinearDamping), m_AngularDamping(ddDesc.AngularDamping),
        m_Force(ddDesc.Force), m_Torque(ddDesc.Torque), m_GravityMultiplier(ddDesc.GravityMultiplier),
        m_IndexInActiveBodies(ddDesc.IndexInActiveBodies), m_IslandIndex(ddDesc.IslandIndex),
        m_DynamicsFlags(ddDesc.Flags)
    {
        if (m_DynamicsFlags.CheckFlag(DynamicsFlags2D::RestrictPos)) m_InverseMass = 0.0f;
        if (m_DynamicsFlags.CheckFlag(DynamicsFlags2D::RestrictRotation)) m_InverseInertia = 0.0f;
    }

    void DynamicsData2D::RecalculateMass(MassCalculation massCalculation)
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    void DynamicsData2D::SetTestSpheres(const std::array<glm::vec2, 2>& points)
    {
        m_TestSpheres[0] = CircleBounds2D(points[0], 0.0f);
        m_TestSpheres[1] = CircleBounds2D(points[1], 0.0f);
    }

    SleepState DynamicsData2D::CheckTestSpheres(const std::array<glm::vec2, 2>& points, F32 maxMoveDistance)
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
        return SleepState::CannotSleep;
    }
}