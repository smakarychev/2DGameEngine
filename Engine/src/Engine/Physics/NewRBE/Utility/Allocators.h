#pragma once
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    using ListAllocator = MemoryManager::ManagedPoolAllocator;
    using NarrowContactAllocator = StackAllocator;
    
    
    template <typename Entry>
    struct GeneralListAllocators2D
    {
        static void Init();
        static void ShutDown();
        static Ref<MemoryManager::ManagedPoolAllocator> s_ElementAllocator;
        static Ref<MemoryManager::ManagedPoolAllocator> s_ListEntryAllocator;
    };

    template <typename Entry>
    Ref<MemoryManager::ManagedPoolAllocator> GeneralListAllocators2D<Entry>::s_ElementAllocator{};
    template <typename Entry>
    Ref<MemoryManager::ManagedPoolAllocator> GeneralListAllocators2D<Entry>::s_ListEntryAllocator{};

    template <typename Entry>
    void GeneralListAllocators2D<Entry>::Init()
    {
        s_ElementAllocator = MemoryManager::GetPoolAllocatorRef(Entry::GetElementSize());
        s_ElementAllocator->GetUnderlyingAllocator()->SetDebugName(typeid(Entry::ElementType).name());
        s_ListEntryAllocator = MemoryManager::GetPoolAllocatorRef(sizeof(Entry));
        s_ListEntryAllocator->GetUnderlyingAllocator()->SetDebugName(typeid(Entry).name());
    }

    template <typename Entry>
    void GeneralListAllocators2D<Entry>::ShutDown()
    {
        s_ElementAllocator.reset();
        s_ListEntryAllocator.reset();
    }
    
}
