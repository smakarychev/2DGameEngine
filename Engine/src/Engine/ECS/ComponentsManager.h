#pragma once

#include "Components.h"
#include "EntityId.h"
#include "Engine/Common/SparseSetPaged.h"

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
        ComponentPool(U32 typeSizeBytes);
        
        virtual ~ComponentPool();

        template <typename T, typename ... Args>
        T& Add(Entity entityId, Args&&... args);

        bool Has(Entity entityId) const;
        
        template <typename T>
        const T& Get(Entity entityId) const;
        template <typename T>
        T& Get(Entity entityId);

        template <typename T>
        void Pop(Entity entityId);
	    virtual void Pop(Entity entityId) = 0;

        template <typename T>
        const T& GetComponent(U32 componentIndex) const;
        template <typename T>
        T& GetComponent(U32 componentIndex);

        U32 GetComponentCount() const { return static_cast<U32>(m_SparseSet.GetDense().size()); }
        const std::vector<Entity> GetDenseEntities() const { return m_SparseSet.GetDense(); }

	    void SetDebugName(const std::string& name) { m_DebugName = name; }
	    const std::string& GetDebugName() const { return m_DebugName; }
	    
    private:
        U8* GetOrCreate(U32 index);
        const U8* TryGet(U32 index) const;
        U8* TryGet(U32 index);
	    template <typename T>
        void PopWithComponent(Entity entityId);
        void* GetComponentAddress(U32 componentIndex) const;
    private:
        std::vector<U8*> m_ComponentsPaged;
        U32 m_TypeSizeBytes{};
        SparseSetPaged<U32, Entity, EntityIdDecomposer> m_SparseSet;

	    std::string m_DebugName{"Default"};
    };

    inline ComponentPool::ComponentPool(U32 typeSizeBytes)
        : m_TypeSizeBytes(typeSizeBytes)
    {
    }

    template <typename T, typename ... Args>
    T& ComponentPool::Add(Entity entityId, Args&&... args)
    {
        U32 componentIndex = m_SparseSet.Push(entityId);
        U8* componentPage = GetOrCreate(componentIndex);
        void* componentAddress = componentPage + static_cast<U64>(m_TypeSizeBytes * Math::FastMod(componentIndex, SPARSE_SET_PAGE_SIZE));
        new(componentAddress) T(std::forward<Args>(args)...);
        return *static_cast<T*>(componentAddress);
    }

    template <typename T>
    const T& ComponentPool::Get(Entity entityId) const
    {
        U32 componentIndex = m_SparseSet.GetIndexOf(entityId);
        return GetComponent<T>(componentIndex);
    }

    template <typename T>
    T& ComponentPool::Get(Entity entityId)
    {
        return const_cast<T&>(const_cast<const ComponentPool*>(this)->Get<T>(entityId));
    }

    template <typename T>
    const T& ComponentPool::GetComponent(U32 componentIndex) const
    {
        return *static_cast<T*>(GetComponentAddress(componentIndex));
    }

    template <typename T>
    T& ComponentPool::GetComponent(U32 componentIndex)
    {
        return const_cast<T&>(const_cast<const ComponentPool*>(this)->GetComponent<T>(componentIndex));
    }

    template <typename T>
    void ComponentPool::Pop(Entity entityId)
    {
        ENGINE_CORE_ASSERT(m_SparseSet.Has(entityId), "Entity has no such component")
        PopWithComponent<T>(entityId);
    }


    template <typename T>
    void ComponentPool::PopWithComponent(Entity entityId)
    {
        auto popCallback = [this](U32 index)
        {
            void* address = GetComponentAddress(index);
            static_cast<T*>(address)->~T();
        };
        auto swapCallback = [this](U32 a, U32 b)
        {
            void* aAddress = GetComponentAddress(a);
            void* bAddress = GetComponentAddress(b);
            std::swap(*static_cast<T*>(aAddress), *static_cast<T*>(bAddress));
        };
        m_SparseSet.Pop(entityId, popCallback, swapCallback);
    }
    
    inline void* ComponentPool::GetComponentAddress(U32 componentIndex) const
    {
        const U8* componentPage = TryGet(componentIndex);
        const void* componentAddress = componentPage + static_cast<U64>(m_TypeSizeBytes * Math::FastMod(componentIndex, SPARSE_SET_PAGE_SIZE));
        return const_cast<void*>(componentAddress);
    }

    inline ComponentPool::~ComponentPool()
    {
        for (auto* page : m_ComponentsPaged)
        {
            if (page)
            {
                DeleteArr(page, static_cast<U32>(SPARSE_SET_PAGE_SIZE * m_TypeSizeBytes));
            }
        }
    }

    inline bool ComponentPool::Has(Entity entityId) const
    {
        return m_SparseSet.Has(entityId);
    }

    inline U8* ComponentPool::GetOrCreate(U32 index)
    {
        U32 pageNum = index >> SPARSE_SET_PAGE_SIZE_LOG;
        if (pageNum >= m_ComponentsPaged.size())
        {
            m_ComponentsPaged.resize(pageNum + 1);
        }
        if (!m_ComponentsPaged[pageNum])
        {
            m_ComponentsPaged[pageNum] = NewArr<U8>(static_cast<U32>(SPARSE_SET_PAGE_SIZE * m_TypeSizeBytes));
        }
        return m_ComponentsPaged[pageNum];
    }

    inline const U8* ComponentPool::TryGet(U32 index) const
    {
        U32 pageNum = index >> SPARSE_SET_PAGE_SIZE_LOG;
        if (pageNum >= m_ComponentsPaged.size())
        {
            return nullptr;
        }
        return m_ComponentsPaged[pageNum];
    }
    
    inline U8* ComponentPool::TryGet(U32 index)
    {
        return const_cast<U8*>(const_cast<const ComponentPool*>(this)->TryGet(index));
    }

    template <typename T>
    class TComponentPool : public ComponentPool
    {
    public:
        TComponentPool(U32 typeSizeBytes);
        void Pop(Entity entityId) override;
    };

    template <typename T>
    TComponentPool<T>::TComponentPool(U32 typeSizeBytes)
        : ComponentPool(typeSizeBytes)
    {
    }

    template <typename T>
    void TComponentPool<T>::Pop(Entity entityId)
    {
        static_cast<ComponentPool*>(this)->Pop<T>(entityId);
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
        bool Has(Entity entityId);

        template <typename T>
        const T& Get(Entity entityId) const;
        template <typename T>
        T& Get(Entity entityId);

        bool DoesPoolExist(U64 componentId) const;
        
        template <typename T>
        const ComponentPool& GetComponentPool() const;
        const ComponentPool& GetComponentPool(U64 componentId) const;

        std::vector<Ref<ComponentPool>>& GetPools() { return m_Pools; }
        U32 GetPoolCount() const { return static_cast<U32>(m_Pools.size()); }

    private:
        std::vector<Ref<ComponentPool>> m_Pools;
    };

    inline bool ComponentManager::DoesPoolExist(U64 componentId) const
    {
        return componentId < m_Pools.size() && m_Pools[componentId] != nullptr;
    }

    inline const ComponentPool& ComponentManager::GetComponentPool(U64 componentId) const
    {
        ENGINE_CORE_ASSERT(componentId < m_Pools.size() && m_Pools[componentId], "No pool for that component exists")
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
            const Ref<ComponentPool> newPool = CreateRef<TComponentPool<T>>(sizeof(T));
            newPool->SetDebugName(typeid(T).name());
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

