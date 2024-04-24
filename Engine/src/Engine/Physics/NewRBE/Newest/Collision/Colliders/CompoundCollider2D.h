#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    static constexpr auto COMPOUND_SHAPE_RECALCULATION_THRESHOLD = 10;

    struct CompoundColliderDesc2D : ColliderDesc2D
    {
        CompoundColliderDesc2D() : ColliderDesc2D(Collider2DType::Compound) {}
        std::vector<Collider2D*> SubColliders{};
    };
    
    struct AddRemoveSubColliderPolicy
    {
        enum SPolicy
        {
            None = 0, CalculateMassAndInertia = Bit(1), CalculateBounds = Bit(2),
            UniqueCheck = Bit(3),
            Safe = CalculateMassAndInertia | UniqueCheck
        };
        SPolicy Policy;
        bool CheckFlag(SPolicy flag) const { return (Policy & flag) == flag; }
    };
    
    class CompoundCollider2D : public Collider2D
    {
        friend class PhysicsFactory;
    public:
        CompoundCollider2D(const CompoundColliderDesc2D& colDesc);
        AABB2D GenerateBounds(const Transform2D& transform) const override;
        AABB2D GenerateLocalBounds() const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
        void AddCollider(Collider2D* collider, AddRemoveSubColliderPolicy policy = {AddRemoveSubColliderPolicy::Safe});
        void RemoveCollider(Collider2D* collider, AddRemoveSubColliderPolicy policy = {AddRemoveSubColliderPolicy::Safe});
        void RecomputeBounds();
        void RecomputeMassInfo();
    private:
        std::vector<Collider2D*> m_SubColliders{};
        // Bounds and mass may be to costly to recompute, so we cache it.
        AABB2D m_Bounds{};
        MassInfo2D m_MassInfo{};
    };
}