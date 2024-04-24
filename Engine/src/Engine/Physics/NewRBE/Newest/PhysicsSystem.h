#pragma once

#include "BodyManager.h"
#include "IslandManager.h"
#include "PhysicsSettings.h"
#include "PhysicsFrameContext.h"
#include "Collision/BroadPhase/BroadPhase.h"

namespace Engine
{
    class RigidBodyWorldDrawer;
}

namespace Engine::WIP::Physics::Newest
{
    class PhysicsSystem
    {
        //TODO: temp.
        friend class ::Engine::RigidBodyWorldDrawer; 
    public:
        // TODO: provide max active bodies.
        void Init(U32 maxBodies, Ref<BroadPhaseLayers> bpLayers, Ref<BodyToBroadPhaseLayerFilter> bpFilter);
        void ShutDown();
        void Update(F32 dt);

        // Shall be called when some collider props are changed.
        void UpdateBodyCollider(RigidBodyId2D bodyId);

        BodyManager& GetBodyManager() { return m_BodyManager; }
        BroadPhase2D& GetBroadPhase() { return m_BroadPhase; }
    private:
        void UpdateContext(F32 dt);
        void SynchronizeBroadPhase();
        void IntegrateVelocities();
        void ProcessCollisions();
        void ProcessPairs(const std::vector<BroadContactPair>& pairs);
        void IntegratePositions();
    private:
        BodyManager m_BodyManager;
        BroadPhase2D m_BroadPhase;
        IslandManager m_IslandManager;
        PhysicsFrameContext m_FrameContext{};
        PhysicsSettings m_Settings{};
    };
}
