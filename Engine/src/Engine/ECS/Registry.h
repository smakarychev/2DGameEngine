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
        void Clear();
        Entity CreateEntity(const std::string& tag = "Default");
        Entity GetEntity(Entity entity) const;
        void DeleteEntity(Entity entity);
        U32 TotalEntities() const { return m_EntityManager.m_TotalEntities; }

        // Add specified component to entity.
        template <typename T, typename ... Args>
        T& Add(Entity entity, Args&&... args);

        // Remove specified component from entity.
        template <typename T>
        void Remove(Entity entity);

        // Check if entity has specified component.
        template <typename T>
        bool Has(Entity entity) const;
        bool Has(U64 componentId, Entity entity) const;

        // Check if registry has such component.
        bool IsComponentExists(U64 componentId) const;
        bool IsEntityExists(Entity entity) const;

        // Returns the entities component.
        template<typename T>
        const T& Get(Entity entity) const;
        template<typename T>
        T& Get(Entity entity);

        template<typename T>
        T& AddOrGet(Entity entity);
        
        template <typename T>
        const ComponentPool& GetComponentPool() const;
        const ComponentPool& GetComponentPool(U64 componentId) const;

        const EntityManager& GetEntityManager() const { return m_EntityManager; }
        
    private:
        ComponentManager m_ComponentManager{};
        EntityManager m_EntityManager{};
    };

    template <typename T, typename ... Args>
    T& Registry::Add(Entity entity, Args&&... args)
    {
        ENGINE_CORE_ASSERT(m_EntityManager.IsAlive(entity), "Entity no longer exists, or haven't existed at all.")
        return m_ComponentManager.Add<T, Args...>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    void Registry::Remove(Entity entity)
    {
        m_ComponentManager.Remove<T>(entity);
    }

    template <typename T>
    bool Registry::Has(Entity entity) const
    {
        U64 componentId = ComponentFamily::TYPE<T>;
        return Has(componentId, entity);
    }

    template <typename T>
    const T& Registry::Get(Entity entity) const
    {
        return m_ComponentManager.GetComponentPool<T>().Get<T>(entity);
    }

    template <typename T>
    T& Registry::Get(Entity entity)
    {
        return const_cast<T&>(const_cast<const Registry*>(this)->Get<T>(entity));
    }

    template <typename T>
    T& Registry::AddOrGet(Entity entity)
    {
        if (Has<T>(entity)) return Get<T>(entity);
        return Add<T>(entity);
    }

    inline bool Registry::Has(U64 componentId, Entity entity) const
    {
        if (!m_ComponentManager.DoesPoolExist(componentId)) return false;
        return m_ComponentManager.GetComponentPool(componentId).Has(entity);
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


