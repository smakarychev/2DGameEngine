#include "enginepch.h"

#include "CircleCollider2D.h"

#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    CircleCollider2D::CircleCollider2D(const CircleColliderDesc2D& colDesc)
        : Collider2D(colDesc),
        m_Radius(colDesc.Radius), m_Center(colDesc.Center)
    {
    }

    AABB2D CircleCollider2D::GenerateBounds(const Transform2D& transform) const
    {
        return AABB2D{ transform.Transform(m_Center), {m_Radius, m_Radius} };
    }

    AABB2D CircleCollider2D::GenerateLocalBounds() const
    {
        return AABB2D{ m_Center, {m_Radius, m_Radius} };
    }

    MassInfo2D CircleCollider2D::CalculateMass() const
    {
        static constexpr auto inertiaCoefficient = 1.0f / 2.0f;
        F32 mass = Math::Pi() * m_Radius * m_Radius * m_PhysicsMaterial.Density;
        F32 inertia = mass * inertiaCoefficient * m_Radius * m_Radius;
        return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = m_Center };
    }

    glm::vec2 CircleCollider2D::GetCenterOfMass() const
    {
        return m_Center;
    }
}
