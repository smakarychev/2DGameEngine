#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    class CompoundCollider2D : public Collider2D
    {
    public:
        CompoundCollider2D();
        DefaultBounds2D GenerateBounds(const Transform2D& transform) const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
    private:
        void RecomputeBounds();
    public:
        std::vector<Collider2D*> SubColliders;
        glm::vec2 Center;
        // Bounds may be to costly to recompute, so we cache it.
        AABB2D Bounds;
    };
}