#pragma once
#include "ComponentsManager.h"
#include "EntityId.h"
#include "EntityManager.h"

namespace Engine
{
    class Registry
    {
    public:
        ~Registry();
        Entity CreateEntity(const std::string& tag = "Default");
        Entity GetEntity(Entity entityId) const;
        EntityContainer& GetEntities(const std::string& tag) const;
        void DeleteEntity(Entity entityId);
        U32 TotalEntities() const { return m_EntityManager.m_TotalEntities; }

        /* TODO: TEMP */
        void PopFromMap(Entity entityId, const std::string& tag);
        void PushToMap(Entity entityId, const std::string& tag);
        /* TODO: TEMP */
        
        // Add specified component to entity.
        template <typename T, typename ... Args>
        T& Add(Entity entityId, Args&&... args);

        // Remove specified component from entity.
        template <typename T>
        void Remove(Entity entityId);

        // Check if entity has specified component.
        template <typename T>
        bool Has(Entity entityId) const;
        bool Has(U64 componentId, Entity entityId) const;

        // Check if registry has such component.
        bool IsComponentExists(U64 componentId) const;

        // Returns the entities component.
        template<typename T>
        const T& Get(Entity entityId) const;
        template<typename T>
        T& Get(Entity entityId);
        
        template <typename T>
        const ComponentPool& GetComponentPool() const;
        const ComponentPool& GetComponentPool(U64 componentId) const;

        const EntityManager& GetEntityManager() const { return m_EntityManager; }
        
    private:
        ComponentManager m_ComponentManager{};
        EntityManager m_EntityManager{};
    };

    template <typename T, typename ... Args>
    T& Registry::Add(Entity entityId, Args&&... args)
    {
        ENGINE_CORE_ASSERT(m_EntityManager.IsAlive(entityId), "Entity no longer exists, or haven't existed at all.")
        return m_ComponentManager.Add<T, Args...>(entityId, std::forward<Args>(args)...);
    }

    template <typename T>
    void Registry::Remove(Entity entityId)
    {
        m_ComponentManager.Remove<T>(entityId);
    }

    template <typename T>
    bool Registry::Has(Entity entityId) const
    {
        U64 componentId = ComponentFamily::TYPE<T>;
        return Has(componentId, entityId);
    }

    template <typename T>
    const T& Registry::Get(Entity entityId) const
    {
        return m_ComponentManager.GetComponentPool<T>().Get<T>(entityId);
    }

    template <typename T>
    T& Registry::Get(Entity entityId)
    {
        return const_cast<T&>(const_cast<const Registry*>(this)->Get<T>(entityId));
    }

    inline bool Registry::Has(U64 componentId, Entity entityId) const
    {
        ENGINE_CORE_ASSERT(m_ComponentManager.GetPoolCount() > componentId, "No such component exists.")
        return m_ComponentManager.GetComponentPool(componentId).Has(entityId);
    }

    template <typename T>
    const ComponentPool& Registry::GetComponentPool() const
    {
        return m_ComponentManager.GetComponentPool<T>();
    }

    inline const ComponentPool& Registry::GetComponentPool(U64 componentId) const
    {
        return m_ComponentManager.GetComponentPool(componentId);
    }
}


