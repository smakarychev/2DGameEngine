#include "enginepch.h"

#include "Registry.h"

namespace Engine
{
    Registry::~Registry()
    {
        Clear();
    }

    void Registry::Clear()
    {
        for (const auto& e : m_EntityManager.m_EntitiesSparseSet)
        {
            DeleteEntity(e);
        }
        m_EntityManager.m_FreeEntities.clear();
    }

    Entity Registry::CreateEntity(const std::string& tag)
    {
        Entity entityId = m_EntityManager.AddEntity(tag);
        m_ComponentManager.Add<Component::Name>(entityId, tag);
        return entityId;
    }

    Entity Registry::GetEntity(Entity entityId) const
    {
        ENGINE_CORE_ASSERT(entityId.GetIndex() < m_EntityManager.m_TotalEntities, "EntityId out of bounds")
        return m_EntityManager.m_EntitiesSparseSet[entityId.GetIndex()];
    }

    void Registry::DeleteEntity(Entity entityId)
    {
        m_EntityManager.DeleteEntity(entityId);
        
        for (auto& componentPool : m_ComponentManager.m_Pools)
        {
            if (componentPool && componentPool->Has(entityId)) componentPool->Pop(entityId);
        }
    }

    bool Registry::IsComponentExists(U64 componentId) const
    {
        return componentId < m_ComponentManager.GetPoolCount() && m_ComponentManager.m_Pools[componentId] != nullptr;
    }

    bool Registry::IsEntityExists(Entity entity) const
    {
        return m_EntityManager.m_EntitiesSparseSet.Has(entity);
    }
}
