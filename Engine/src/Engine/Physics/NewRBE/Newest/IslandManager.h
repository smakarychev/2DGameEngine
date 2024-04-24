#pragma once

#include "RigidBody.h"
#include "Engine/Core/Types.h"

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    static constexpr auto ISLAND_INVALID_ID = std::numeric_limits<U32>::max();
    
    struct IslandBodyLink
    {
        U32 Link{ISLAND_INVALID_ID};
        U32 IslandIndex{ISLAND_INVALID_ID};
    };
    
    class IslandManager
    {
    public:
        void Init(U32 maxActiveBodies);
        void LinkBodies(U32 first, U32 second);

        U32 GetIslandCount() const { return m_IslandCount; }
        void Clear();
        void Finalize(const std::vector<RigidBodyId2D>& activeBodies);
    private:
        U32 GetLowestLinkIndex(U32 body);
        void BuildIslands(const std::vector<RigidBodyId2D>& activeBodies);
    private:
        U32 m_MaxBodies{0};
        U32 m_IslandCount{0};
        std::vector<IslandBodyLink> m_BodyLinks{};
        std::vector<U32> m_IslandStarts{};
        std::vector<RigidBodyId2D> m_BodyIslands{}; 
    };
}
