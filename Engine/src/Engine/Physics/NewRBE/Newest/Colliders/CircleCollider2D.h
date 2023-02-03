#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    class CircleCollider2D : public Collider2D
    {
    public:
        CircleCollider2D(const glm::vec2& center = glm::vec2{ 0.0f }, F32 radius = 1.0f);
        DefaultBounds2D GenerateBounds(const Transform2D& transform) const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
    public:
        F32 Radius;
        glm::vec2 Center;
    };
}
