#include "enginepch.h"

#include "Engine/ECS/EntityManager.h"

namespace Engine
{
    Entity EntityManager::AddEntity(const std::string& tag)
    {
        Entity newEntity;
        if (!m_FreeEntities.empty())
        {
            // Reuse entity.
            newEntity = m_FreeEntities.back(); m_FreeEntities.pop_back();
        }
        else
        {
            // Generate new entity.
            newEntity = Entity(static_cast<U32>(m_EntitiesSparseSet.GetDense().size()), 0);
        }
        m_EntitiesSparseSet.Push(newEntity);
        m_TotalEntities++;
        return newEntity;
    }

    void EntityManager::DeleteEntity(Entity entityId)
    {
        m_FreeEntities.push_back({entityId.GetIndex(), entityId.GetGeneration() + 1});
        m_EntitiesSparseSet.Pop(entityId);
        m_TotalEntities--;
    }

    bool EntityManager::IsAlive(Entity entityId)
    {
        return m_EntitiesSparseSet.Has(entityId);
    }
}
