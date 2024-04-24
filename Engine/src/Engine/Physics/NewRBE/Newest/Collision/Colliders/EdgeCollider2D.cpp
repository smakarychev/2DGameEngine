#include "enginepch.h"

#include "EdgeCollider2D.h"

#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    EdgeCollider2D::EdgeCollider2D(const EdgeColliderDesc2D& colDesc)
        : Collider2D(colDesc),
        m_Start(colDesc.Start), m_End(colDesc.End)
    {
    }

    AABB2D EdgeCollider2D::GenerateBounds(const Transform2D& transform) const
    {
        glm::vec2 worldStart = transform.Transform(m_Start);
        glm::vec2 worldEnd = transform.Transform(m_End);
        glm::vec2 max{
            Math::Max(worldEnd.x, worldStart.x),
            Math::Max(worldEnd.y, worldStart.y)
        };
        glm::vec2 min{
            Math::Min(worldEnd.x, worldStart.x),
            Math::Min(worldEnd.y, worldStart.y)
        };
        return AABB2D{ (max + min) * 0.5f, (max - min) * 0.5f };
    }

    AABB2D EdgeCollider2D::GenerateLocalBounds() const
    {
        return AABB2D{ (m_Start + m_End) * 0.5f, (m_End - m_Start) * 0.5f };
    }

    MassInfo2D EdgeCollider2D::CalculateMass() const
    {
        static constexpr auto inertiaCoefficient = 1.0f / 12.0f;
        F32 edgeLen2 = glm::length2(m_End - m_Start);
        F32 mass = Math::Sqrt(edgeLen2) * m_PhysicsMaterial.Density;
        F32 inertia = mass * edgeLen2 * inertiaCoefficient;
        return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = glm::vec2{0.0f} };
    }

    glm::vec2 EdgeCollider2D::GetCenterOfMass() const
    {
        return (m_End + m_Start) * 0.5f;
    }
}
