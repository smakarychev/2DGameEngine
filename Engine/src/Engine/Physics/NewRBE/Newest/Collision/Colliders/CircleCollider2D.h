#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    struct CircleColliderDesc2D : ColliderDesc2D
    {
        CircleColliderDesc2D() : ColliderDesc2D(Collider2DType::Circle) {}
        glm::vec2 Center{};
        F32 Radius{};
    };
    
    class CircleCollider2D : public Collider2D
    {
        friend class PhysicsFactory;
    public:
        CircleCollider2D(const CircleColliderDesc2D& colDesc);
        AABB2D GenerateBounds(const Transform2D& transform) const override;
        AABB2D GenerateLocalBounds() const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
    private:
        F32 m_Radius{};
        glm::vec2 m_Center{};
    };
}
