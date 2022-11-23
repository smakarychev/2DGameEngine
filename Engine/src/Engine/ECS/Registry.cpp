#include "enginepch.h"

#include "Registry.h"

namespace Engine
{
    Registry::~Registry()
    {
        // Temp.
        auto dense = m_EntityManager.m_EntitiesSparseSet.GetDense();
        std::ranges::reverse(dense);
        for (auto& e : dense)
        {
            DeleteEntity(e);
        }
    }
    
    Entity Registry::CreateEntity(const std::string& tag)
    {
        Entity entityId = m_EntityManager.AddEntity(tag);
        auto& tagC = m_ComponentManager.Add<Component::Tag>(entityId, tag);

        // TODO: this is temp.
        m_EntityManager.m_EntitiesMap[tagC.TagName].Push(entityId);
        // TODO: this is temp.
        
        return entityId;
    }

    Entity Registry::GetEntity(Entity entityId) const
    {
        ENGINE_CORE_ASSERT(entityId.GetIndex() < m_EntityManager.m_TotalEntities, "EntityId out of bounds")
        return m_EntityManager.m_EntitiesSparseSet[entityId.GetIndex()];
    }

    const std::vector<Entity>& Registry::GetEntities(const std::string& tag) const
    {
        static std::vector<Entity> fallback;
        auto it = m_EntityManager.m_EntitiesMap.find(tag);
        if (it != m_EntityManager.m_EntitiesMap.end()) return it->second.GetDense();
        return fallback;
    }

    void Registry::DeleteEntity(Entity entityId)
    {
        m_EntityManager.DeleteEntity(entityId);
        
        // TODO: this is temp.
        auto& tag = Get<Component::Tag>(entityId);
        m_EntityManager.m_EntitiesMap[tag.TagName].Pop(entityId);
        // TODO: this is temp.
        
        for (auto& componentPool : m_ComponentManager.m_Pools)
        {
            if (componentPool) componentPool->TryPop(entityId);
        }
    }

    bool Registry::IsComponentExists(U64 componentId) const
    {
        return componentId < m_ComponentManager.GetPoolCount() && m_ComponentManager.m_Pools[componentId] != nullptr;
    }
}
