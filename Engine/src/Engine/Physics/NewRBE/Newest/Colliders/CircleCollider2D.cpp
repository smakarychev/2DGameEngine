#include "enginepch.h"

#include "CircleCollider2D.h"

#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    CircleCollider2D::CircleCollider2D(const glm::vec2& center, F32 radius)
        : Collider2D(Collider2DType::Circle),
        Radius(radius), Center(center)
    {
    }


    DefaultBounds2D CircleCollider2D::GenerateBounds(const Transform2D& transform) const
    {
        return AABB2D{ transform.Transform(Center), {Radius, Radius} };
    }

    MassInfo2D CircleCollider2D::CalculateMass() const
    {
        static constexpr auto inertiaCoefficient = 1.0f / 2.0f;
        F32 mass = Math::Pi() * Radius * Radius * m_PhysicsMaterial.Density;
        F32 inertia = mass * inertiaCoefficient * Radius * Radius;
        return MassInfo2D{ .Mass = mass, .Inertia = inertia, .CenterOfMass = Center };
    }

    glm::vec2 CircleCollider2D::GetCenterOfMass() const
    {
        return Center;
    }
}
