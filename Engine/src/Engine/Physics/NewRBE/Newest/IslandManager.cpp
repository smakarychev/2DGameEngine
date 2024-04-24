#include "enginepch.h"

#include "IslandManager.h"

namespace Engine::WIP::Physics::Newest
{
    void IslandManager::Init(U32 maxActiveBodies)
    {
        m_MaxBodies = maxActiveBodies;
        m_IslandStarts.resize(m_MaxBodies);
        m_BodyLinks.resize(m_MaxBodies);
        m_BodyIslands.resize(m_MaxBodies);
        for (U32 i = 0; i < m_BodyLinks.size(); i++) m_BodyLinks[i].Link = i;
    }

    void IslandManager::LinkBodies(U32 first, U32 second)
    {
        // Check that both bodies are active (not static).
        if (first >= m_MaxBodies || second >= m_MaxBodies) return;
        U32 rootLinkA = GetLowestLinkIndex(first);
        U32 rootLinkB = GetLowestLinkIndex(second);
        // If those are same, bodies already linked.
        if (rootLinkA != rootLinkB)
        {
            if (rootLinkA < rootLinkB) m_BodyLinks[rootLinkB].Link = rootLinkA;
            else m_BodyLinks[rootLinkA].Link = rootLinkB;
        }

        // Simplify links.
        U32 smallest = Math::Min(rootLinkA, rootLinkB);
        m_BodyLinks[first].Link = smallest;
        m_BodyLinks[second].Link = smallest;
    }

    void IslandManager::Clear()
    {
        for (U32 i = 0; i < m_BodyLinks.size(); i++) m_BodyLinks[i].Link = i;
        m_IslandStarts.clear();
        m_BodyIslands.clear();
        m_IslandCount = 0;
    }

    void IslandManager::Finalize(const std::vector<RigidBodyId2D>& activeBodies)
    {
        BuildIslands(activeBodies);
    }

    U32 IslandManager::GetLowestLinkIndex(U32 body)
    {
        U32 linkedTo = m_BodyLinks[body].Link;
        while (m_BodyLinks[linkedTo].Link != linkedTo) linkedTo = m_BodyLinks[linkedTo].Link;
        return linkedTo;
    }

    void IslandManager::BuildIslands(const std::vector<RigidBodyId2D>& activeBodies)
    {
        U32 currentIsland = 0;
        U32 currentCount = 0;
        for (auto active : activeBodies)
        {
            IslandBodyLink& link = m_BodyLinks[active];
            // If body is linked to itself, it marks the new island
            if (link.Link == active)
            {
                link.IslandIndex = currentIsland;
                m_IslandStarts[currentIsland] = currentCount;
                currentIsland++;
            }
            else
            {
                ENGINE_CORE_ASSERT(m_BodyLinks[link.Link].IslandIndex != ISLAND_INVALID_ID, "Catastrophic error.")
                link.IslandIndex = m_BodyLinks[link.Link].IslandIndex;
            }
            currentCount++;
        }
        m_IslandCount = currentIsland;
        // Push bodies to islands.
        for (auto active : activeBodies)
        {
            IslandBodyLink& link = m_BodyLinks[active];
            U32& start = m_IslandStarts[link.IslandIndex];
            m_BodyIslands[start] = active;
            start++;
        }
    }
}
