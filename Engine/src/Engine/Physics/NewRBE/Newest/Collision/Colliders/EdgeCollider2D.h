#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    struct EdgeColliderDesc2D : ColliderDesc2D
    {
        EdgeColliderDesc2D() : ColliderDesc2D(Collider2DType::Edge) {}
        glm::vec2 Start{};
        glm::vec2 End{};
    };
    
    // Line segment.
    class EdgeCollider2D : public Collider2D
    {
        friend class PhysicsFactory;
    public:
        EdgeCollider2D(const EdgeColliderDesc2D& colDesc);
        AABB2D GenerateBounds(const Transform2D& transform) const override;
        AABB2D GenerateLocalBounds() const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
    private:
        // Normal is computed when needed, as an outward normal from `Start` to `End`.
        // Relative to rigidbody.
        glm::vec2 m_Start{};
        glm::vec2 m_End{};
    };
}
