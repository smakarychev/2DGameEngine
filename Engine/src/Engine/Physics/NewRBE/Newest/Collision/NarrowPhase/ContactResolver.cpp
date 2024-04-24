#include "enginepch.h"

#include "ContactResolver.h"

#include "Engine/Core/Core.h"

namespace Engine::WIP::Physics::Newest
{
    void ContactResolver::PreSolve(PhysicsFrameContext* context)
    {
        m_Context = context;
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    void ContactResolver::WarmStart()
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    void ContactResolver::ResolveVelocity()
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    bool ContactResolver::ResolvePosition()
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
        return false;
    }

    void ContactResolver::PostSolve()
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    void ContactResolver::ResolveTangentVelocity(const ContactConstraint2D& constraint)
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }

    void ContactResolver::ResolveNormalVelocity(const ContactConstraint2D& constraint)
    {
        ENGINE_CORE_ASSERT(false, "Unimplemented.")
    }
}
