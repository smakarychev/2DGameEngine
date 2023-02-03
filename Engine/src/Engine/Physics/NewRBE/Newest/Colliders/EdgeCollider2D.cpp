#include "enginepch.h"

#include "EdgeCollider2D.h"

#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    EdgeCollider2D::EdgeCollider2D(const glm::vec2& start, const glm::vec2& end)
        : Collider2D(Collider2DType::Edge),
        Start(start), End(end)
    {
    }

    DefaultBounds2D EdgeCollider2D::GenerateBounds(const Transform2D& transform) const
    {
        glm::vec2 worldStart = transform.Transform(Start);
        glm::vec2 worldEnd = transform.Transform(End);
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

    MassInfo2D EdgeCollider2D::CalculateMass() const
    {
        static constexpr auto inertiaCoefficient = 1.0f / 12.0f;
        F32 edgeLen2 = glm::length2(End - Start);
        F32 mass = Math::Sqrt(edgeLen2) * m_PhysicsMaterial.Density;
        F32 inertia = mass * edgeLen2 * inertiaCoefficient;
        return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = glm::vec2{0.0f} };
    }

    glm::vec2 EdgeCollider2D::GetCenterOfMass() const
    {
        return (End + Start) * 0.5f;
    }
}
