#include "enginepch.h"

#include "Registry.h"

namespace Engine
{
    Registry::~Registry()
    {
        // Temp.
        for (const auto& e : m_EntityManager.m_EntitiesSparseSet)
        {
            DeleteEntity(e);
        }
    }
    
    Entity Registry::CreateEntity(const std::string& tag)
    {
        Entity entityId = m_EntityManager.AddEntity(tag);
        auto& tagC = m_ComponentManager.Add<Component::Tag>(entityId, tag);

        // TODO: this is temp.
        PushToMap(entityId, tagC.TagName);
        // TODO: this is temp.
        
        return entityId;
    }

    Entity Registry::GetEntity(Entity entityId) const
    {
        ENGINE_CORE_ASSERT(entityId.GetIndex() < m_EntityManager.m_TotalEntities, "EntityId out of bounds")
        return m_EntityManager.m_EntitiesSparseSet[entityId.GetIndex()];
    }

    EntityContainer& Registry::GetEntities(const std::string& tag) const
    {
        static EntityContainer fallback;
        auto it = m_EntityManager.m_EntitiesMap.find(tag);
        if (it != m_EntityManager.m_EntitiesMap.end()) return const_cast<EntityContainer&>(it->second);
        return fallback;
    }

    void Registry::DeleteEntity(Entity entityId)
    {
        m_EntityManager.DeleteEntity(entityId);
        
        // TODO: this is temp.
        auto& tag = Get<Component::Tag>(entityId);
        PopFromMap(entityId, tag.TagName);
        // TODO: this is temp.
        
        for (auto& componentPool : m_ComponentManager.m_Pools)
        {
            if (componentPool && componentPool->Has(entityId)) componentPool->Pop(entityId);
        }
    }

    void Registry::PopFromMap(Entity entityId, const std::string& tag)
    {
        m_EntityManager.m_EntitiesMap[tag].Pop(entityId);
    }

    void Registry::PushToMap(Entity entityId, const std::string& tag)
    {
        m_EntityManager.m_EntitiesMap[tag].Push(entityId);
    }

    bool Registry::IsComponentExists(U64 componentId) const
    {
        return componentId < m_ComponentManager.GetPoolCount() && m_ComponentManager.m_Pools[componentId] != nullptr;
    }
}
