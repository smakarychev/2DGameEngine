#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    // Line segment.
    class EdgeCollider2D : public Collider2D
    {
    public:
        EdgeCollider2D(const glm::vec2& start = glm::vec2{ -1.0f, 0.0f }, const glm::vec2& end = glm::vec2{ 1.0f, 0.0f });
        DefaultBounds2D GenerateBounds(const Transform2D& transform) const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
    public:
        // Normal is computed when needed, as an outward normal from `Start` to `End`.
        // Relative to rigidbody.
        glm::vec2 Start;
        glm::vec2 End;
    };
}
