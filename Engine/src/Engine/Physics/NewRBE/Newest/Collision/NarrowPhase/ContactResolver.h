#pragma once

namespace Engine::WIP::Physics::Newest
{
    struct ContactConstraint2D;
    struct PhysicsFrameContext;
    
    class ContactResolver
    {
    public:
        void PreSolve(PhysicsFrameContext* context);
        void WarmStart();
        void ResolveVelocity();
        bool ResolvePosition();
        void PostSolve();
    private:
        void ResolveTangentVelocity(const ContactConstraint2D& constraint);
        void ResolveNormalVelocity(const ContactConstraint2D& constraint);
    private:
        PhysicsFrameContext* m_Context{nullptr};
    };
}
