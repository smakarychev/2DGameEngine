#include "enginepch.h"
#include "CompoundCollider2D.h"

#include "Engine/Physics/NewRBE/Newest/Transform.h"

namespace Engine::WIP::Physics::Newest
{
    CompoundCollider2D::CompoundCollider2D(const CompoundColliderDesc2D& colDesc)
        : Collider2D(colDesc),
        m_SubColliders(colDesc.SubColliders)
    {
        if (!m_SubColliders.empty())
        {
            RecomputeBounds(); RecomputeMassInfo();
        }
    }

    AABB2D CompoundCollider2D::GenerateBounds(const Transform2D& transform) const
    {
        // We have two options:
        //  - Generate bounds based on rotated local bounds
        //  - Generate bounds as combination of rotated local bounds of subcolliders
        // The latter will produce tighter result, but requires complete recalculation,
        // so we use it only when there is not that many shapes.
        if (m_SubColliders.size() <= COMPOUND_SHAPE_RECALCULATION_THRESHOLD)
        {
            AABB2D bounds{glm::vec2{0.0f}, glm::vec2{0.0f}};
            for (const auto& subCol : m_SubColliders)
            {
                bounds = AABB2D{bounds, subCol->GenerateBounds(transform)};
            }
            return bounds;
        }
        return m_Bounds.Rotate(transform.Rotation).Translate(transform.Position);
    }

    AABB2D CompoundCollider2D::GenerateLocalBounds() const
    {
        return m_Bounds;
    }

    MassInfo2D CompoundCollider2D::CalculateMass() const
    {
        return m_MassInfo;
    }

    glm::vec2 CompoundCollider2D::GetCenterOfMass() const
    {
        return m_MassInfo.CenterOfMass;
    }

    void CompoundCollider2D::AddCollider(Collider2D* collider, AddRemoveSubColliderPolicy policy)
    {
        if (policy.CheckFlag(AddRemoveSubColliderPolicy::UniqueCheck) &&
            std::ranges::find(m_SubColliders, collider) != m_SubColliders.end())
        {
            ENGINE_CORE_WARN("Compound collider: trying to add collider, that alredy exists.");
            return;
        }
        m_SubColliders.push_back(collider);
        if (policy.CheckFlag(AddRemoveSubColliderPolicy::CalculateMassAndInertia)) RecomputeMassInfo();
        if (policy.CheckFlag(AddRemoveSubColliderPolicy::CalculateBounds)) RecomputeBounds();
    }

    void CompoundCollider2D::RemoveCollider(Collider2D* collider, AddRemoveSubColliderPolicy policy)
    {
        auto it = std::ranges::find(m_SubColliders, collider);
        if (it == m_SubColliders.end())
        {
            ENGINE_CORE_WARN("Compound collider: trying to delete collider, that doesn't exist.");
            return;
        }
        m_SubColliders.erase(it);
        if (policy.CheckFlag(AddRemoveSubColliderPolicy::CalculateMassAndInertia)) RecomputeMassInfo();
        if (policy.CheckFlag(AddRemoveSubColliderPolicy::CalculateBounds)) RecomputeBounds();
    }

    void CompoundCollider2D::RecomputeBounds()
    {
        AABB2D bounds{glm::vec2{0.0f}, glm::vec2{0.0f}};
        for (const auto& subCol : m_SubColliders)
        {
            bounds = AABB2D{bounds, subCol->GenerateLocalBounds()};
        }
        m_Bounds = bounds;
    }

    void CompoundCollider2D::RecomputeMassInfo()
    {
        auto& [mass, inertia, center] = m_MassInfo;
        center = glm::vec2{ 0.0f };
        mass = 0.0f;
        inertia = 0.0f;
        for (auto& subCol : m_SubColliders)
        {
            // Sensors do not contribute to mass.
            if (subCol->IsSensor()) continue;
            MassInfo2D massInfo = subCol->CalculateMass();
            mass += massInfo.Mass;
            inertia += massInfo.Inertia;
            center += massInfo.CenterOfMass * massInfo.Mass;
        } 
        if (mass > 0.0f) center *= 1.0f / mass;
        if (inertia > 0.0f) inertia -= mass * glm::dot(center, center);
    }
}
