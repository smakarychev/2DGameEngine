#pragma once

#include "Components.h"
#include "EntityId.h"

#include "Engine/Memory/MemoryUtils.h"

namespace Engine
{
    // Used to get all the components unique id.
    // This may not work sadly.
    class ComponentFamily
    {
        inline static U64 s_Counter = 0;
    public:
        template <typename ComponentType>
        inline static const U64 TYPE = s_Counter++;
    };
    
	class ComponentPool
    {
    public:
        ComponentPool(U64 typeSizeBytes)
            : m_Allocator(MemoryManager::GetPoolAllocator(typeSizeBytes))
        {
            m_Allocator.GetUnderlyingAllocator()->SetForceContinuous(true);
            m_BaseTypeSizeBytes = m_Allocator.GetBaseTypeSize(); 
        }

        template <typename T, typename ... Args>
        T& Add(Entity entityId, Args&&... args);

        bool Has(Entity entityId) const;

	    template <typename T>
        const T& Get(Entity entityId) const;
        template <typename T>
        T& Get(Entity entityId);

        template <typename T>
        void Pop(Entity entityId);

        template <typename T>
        void TryPop(Entity entityId);
        void TryPop(Entity entityId);

        template <typename T>
        const T& GetComponent(U32 componentIndex) const;
        template <typename T>
        T& GetComponent(U32 componentIndex);
        void* GetComponentAddress(U32 componentIndex) const;

        U32 GetComponentCount() const { return static_cast<U32>(m_SparseSet.GetDense().size()); }

        const std::vector<U32>& GetSparseEntities() const { return m_SparseSet.GetSparse(); }
        const std::vector<Entity>& GetDenseEntities() const { return m_SparseSet.GetDense(); }
        MemoryManager::ManagedPoolAllocator& GetAllocator() const { return m_Allocator; }

	    void SetDebugName(const std::string& name) { m_DebugName = name; }
	private:
	    void PopWithComponent(Entity entityId);
    public:
        static constexpr U32 NULL_INDEX = std::numeric_limits<U32>::max();
    private:
	    SparseSet<U32, Entity, EntityIdDecomposer> m_SparseSet;
	    U64 m_BaseTypeSizeBytes;
        MemoryManager::ManagedPoolAllocator& m_Allocator;

	    std::string m_DebugName = "Cmp pool";
    };

    inline bool ComponentPool::Has(Entity entityId) const
    {
        return m_SparseSet.Has(entityId);
    }

    template <typename T, typename ... Args>
    T& ComponentPool::Add(Entity entityId, Args&&... args)
    {
        // Here we could check if entityId is within the bounds,
        // but it is more convenient to assume that provided entityId
        // is correct, and instead enlarge the sparse vector if it is out of bounds.
        void* componentAddress = nullptr;
        m_SparseSet.Push(entityId, [&componentAddress, this](U32 index)
        {
            m_Allocator.Alloc<T>();
            PoolAllocator* underlying = m_Allocator.GetUnderlyingAllocator();
            componentAddress = static_cast<U8*>(underlying->GetPoolHead()) + underlying->GetBaseTypeSize() * index;
        });
        new(componentAddress) T(std::forward<Args>(args)...);
        return *static_cast<T*>(componentAddress);
    }

    template <typename T>
    const T& ComponentPool::Get(Entity entityId) const
    {
        ENGINE_CORE_ASSERT(m_SparseSet.Has(entityId), "Entity has no such component")
        return GetComponent<T>(m_SparseSet.GetSparse()[entityId.GetIndex()]);
    }

    template <typename T>
    T& ComponentPool::Get(Entity entityId)
    {
        return const_cast<T&>(const_cast<const ComponentPool*>(this)->Get<T>(entityId));
    }

    template <typename T>
    void ComponentPool::Pop(Entity entityId)
    {
        ENGINE_CORE_ASSERT(m_SparseSet.Has(entityId), "Entity has no such component")
        PopWithComponent(entityId);
    }

    template <typename T>
    void ComponentPool::TryPop(Entity entityId)
    {
        if (!m_SparseSet.Has(entityId)) return;
        PopWithComponent(entityId);
    }

    inline void ComponentPool::TryPop(Entity entityId)
    {
        if (!m_SparseSet.Has(entityId)) return;
        PopWithComponent(entityId);
    }

    inline void* ComponentPool::GetComponentAddress(U32 componentIndex) const
    {
        PoolAllocator* underlying = m_Allocator.GetUnderlyingAllocator();
        return static_cast<U8*>(underlying->GetPoolHead()) + underlying->GetBaseTypeSize() * componentIndex;
    }

    inline void ComponentPool::PopWithComponent(Entity entityId)
    {
        auto popCallback = [this](U32 index)
        {
            PoolAllocator* underlying = m_Allocator.GetUnderlyingAllocator();
            void* componentAddress = GetComponentAddress(index);
            m_Allocator.Dealloc(componentAddress);
        };
        auto swapCallback = [this](U32 a, U32 b)
        {
            PoolAllocator* underlying = m_Allocator.GetUnderlyingAllocator();
            // This is temp (not the name :) ).
            void* aAddress = GetComponentAddress(a);
            void* bAddress = GetComponentAddress(b);
            U32 typeSizeBytes = static_cast<U32>(underlying->GetBaseTypeSize());
            U8* temp = NewArr<U8>(typeSizeBytes);
            MemoryUtils::Copy(temp, aAddress, typeSizeBytes);
            MemoryUtils::Copy(aAddress, bAddress, typeSizeBytes);
            MemoryUtils::Copy(bAddress, temp, typeSizeBytes);
            DeleteArr<U8>(temp, typeSizeBytes);
        };
        m_SparseSet.Pop(entityId, popCallback, swapCallback);
    }

    template <typename T>
    const T& ComponentPool::GetComponent(U32 componentIndex) const
    {
        ENGINE_CORE_ASSERT(componentIndex < m_SparseSet.GetDense().size(), "Invalid component index.")
        PoolAllocator* underlying = m_Allocator.GetUnderlyingAllocator();
        void* componentAddress = static_cast<U8*>(underlying->GetPoolHead()) +
            underlying->GetBaseTypeSize() * componentIndex;
        return *static_cast<T*>(componentAddress);
    }

    template <typename T>
    T& ComponentPool::GetComponent(U32 componentIndex)
    {
        return const_cast<T&>(const_cast<const ComponentPool*>(this)->GetComponent<T>(componentIndex));
    }

    class ComponentManager
    {
        friend class Registry;
    public:
        template <typename T, typename ... Args>
        T& Add(Entity entityId, Args&&... args);

        template <typename T>
        void Remove(Entity entityId);

        template <typename T>
        void TryRemove(Entity entityId);

        template <typename T>
        bool Has(Entity entityId);

        template <typename T>
        const T& Get(Entity entityId) const;
        template <typename T>
        T& Get(Entity entityId);

        template <typename T>
        const ComponentPool& GetComponentPool() const;
        const ComponentPool& GetComponentPool(U64 componentId) const;

        std::vector<Ref<ComponentPool>>& GetPools() { return m_Pools; }
        U32 GetPoolCount() const { return static_cast<U32>(m_Pools.size()); }

    private:
        std::vector<Ref<ComponentPool>> m_Pools;
    };

    inline const ComponentPool& ComponentManager::GetComponentPool(U64 componentId) const
    {
        ENGINE_CORE_ASSERT(componentId < m_Pools.size(), "No pool for that component exists")
        return *m_Pools[componentId];
    }

    template <typename T, typename ... Args>
    T& ComponentManager::Add(Entity entityId, Args&&... args)
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        if (componentId >= m_Pools.size())
        {
            // No pool for that component exists yet.
            m_Pools.resize(componentId + 1);
        }
        if (!m_Pools[componentId])
        {
            // No pool for that component exists yet.
            const Ref<ComponentPool> newPool = CreateRef<ComponentPool>(sizeof(T));
            newPool->SetDebugName(typeid(T).name());
            newPool->GetAllocator().GetUnderlyingAllocator()->SetDebugName(typeid(T).name());
            m_Pools[componentId] = newPool;
        }

        ComponentPool& pool = *m_Pools[componentId];
        return pool.Add<T>(entityId, std::forward<Args>(args)...);
    }

    template <typename T>
    void ComponentManager::Remove(Entity entityId)
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        ENGINE_CORE_ASSERT(componentId < m_Pools.size(), "No pool for that component exists")
        ComponentPool& pool = *m_Pools[componentId];
        pool.Pop<T>(entityId);
    }

    template <typename T>
    void ComponentManager::TryRemove(Entity entityId)
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        if (componentId >= m_Pools.size()) return;
        ComponentPool& pool = *m_Pools[componentId];
        pool.TryPop<T>(entityId);
    }

    template <typename T>
    bool ComponentManager::Has(Entity entityId)
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        ENGINE_CORE_ASSERT(componentId < m_Pools.size(), "No pool for that component exists")
        ComponentPool& pool = *m_Pools[componentId];
        return pool.Has(entityId);
    }

    template <typename T>
    const T& ComponentManager::Get(Entity entityId) const
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        ENGINE_CORE_ASSERT(componentId < m_Pools.size(), "No pool for that component exists")
        ComponentPool& pool = *m_Pools[componentId];
        return pool.Get<T>(entityId);
    }

    template <typename T>
    T& ComponentManager::Get(Entity entityId)
    {
        return const_cast<T&>(const_cast<ComponentManager*>(this)->Get<T>(entityId));
    }

    template <typename T>
    const ComponentPool& ComponentManager::GetComponentPool() const
    {
        const U64 componentId = ComponentFamily::TYPE<T>;
        return GetComponentPool(componentId);
    }
}
