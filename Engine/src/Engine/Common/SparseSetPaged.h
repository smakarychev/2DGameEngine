﻿#pragma once

#include "SparseSet.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    static const U32 SPARSE_SET_PAGE_SIZE = 1024;
    static_assert(Math::IsPowerOf2(SPARSE_SET_PAGE_SIZE), "Page size must be a power of 2");
    static const U32 SPARSE_SET_PAGE_SIZE_LOG = Math::Log2(SPARSE_SET_PAGE_SIZE);
    
    
    // ST - sparse type, DT - dense type, Dec - dense type decomposer.
    template <typename ST, typename DT, typename Dec>
    class SparseSetPaged
    {
        using SparseSetPage = std::vector<ST>;
        struct Iterator
        {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = I32;
            using value_type = DT;
            using pointer = value_type*;
            using reference = value_type&;

            Iterator(std::vector<DT>& dense, U32 index) : m_Dense(dense), m_Index(index) { }
            
            reference operator*() const { return m_Dense[m_Index]; }
            pointer operator->() { return &m_Dense[m_Index]; }
            Iterator operator++() { m_Index--; return *this; }
            friend bool operator==(const Iterator& a, const Iterator& b) { return a.m_Index == b.m_Index; }
            friend bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); }
        private:
            U32 m_Index;
            std::vector<DT>& m_Dense;
        };
    public:
        SparseSetPaged(ST nullFlag = static_cast<ST>(NULL_ENTITY));
        ST Push(DT value);
        template <typename PushCallback>
        ST Push(DT value, PushCallback callback = [](ST value){});
        void Pop(DT value);
        template <typename PopCallback, typename SwapCallback>
        void Pop(DT value, PopCallback popCallback = [](ST value){}, SwapCallback swapCallback = [](ST a, ST b) {});
        // Checks if value, that was generated by this set, still belongs to it. 
        bool Has(DT value) const;

        ST GetIndexOf(DT value) const;
        
        DT& operator[](DT value);
        const DT& operator[](DT value) const;
        
        Iterator begin() { return Iterator(m_Dense, static_cast<U32>(m_Dense.size() - 1)); }
        Iterator end() { return Iterator(m_Dense, -1); }
    
        ST GetNullFlag() const { return m_NullFlag; }

        const std::vector<DT>& GetDense() const { return m_Dense; }
        
    private:
        std::vector<ST>* GetOrCreate(U32 index);
        const std::vector<ST>* TryGet(U32 index) const;
        std::vector<ST>* TryGet(U32 index);
    private:
        std::vector<Ref<std::vector<ST>>> m_SparsePaged;
        std::vector<DT> m_Dense;
        const ST m_NullFlag;
    };

    template <typename ST, typename DT, typename Dec>
    SparseSetPaged<ST, DT, Dec>::SparseSetPaged(ST nullFlag)
    : m_NullFlag(nullFlag)
    {
    }

    template <typename ST, typename DT, typename Dec>
    ST SparseSetPaged<ST, DT, Dec>::Push(DT value)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = GetOrCreate(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(!Has(value), "Value is already set.")
        ENGINE_CORE_ASSERT((*sparseSet)[mappedIndex] == m_NullFlag, "Catastrophic failure.")
        (*sparseSet)[mappedIndex] = static_cast<ST>(m_Dense.size());
        m_Dense.emplace_back(value);
        return (*sparseSet)[mappedIndex];
    }

    template <typename ST, typename DT, typename Dec>
    template <typename PushCallback>
    ST SparseSetPaged<ST, DT, Dec>::Push(DT value, PushCallback callback)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = GetOrCreate(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(!Has(value), "Value is already set.")
        ENGINE_CORE_ASSERT((*sparseSet)[mappedIndex] == m_NullFlag, "Catastrophic failure.")
        (*sparseSet)[mappedIndex] = static_cast<ST>(m_Dense.size());
        m_Dense.emplace_back(value);
        callback((*sparseSet)[mappedIndex]);
        return (*sparseSet)[mappedIndex];
    }

    template <typename ST, typename DT, typename Dec>
    void SparseSetPaged<ST, DT, Dec>::Pop(DT value)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(sparseSet, "Invalid value.")
        ENGINE_CORE_ASSERT((*sparseSet)[mappedIndex] != m_NullFlag, "No such value.")
        // We keep the continuous layout,
        // w/o need to maintain original order,
        // so instead of classic shift left (vector-like) operation,
        // we swap with the last and pop the last.
        if (m_Dense.size() > 1)
        {
            DT lastVal = m_Dense.back();
            auto&& [lvGen, lvIndex] = Dec::Decompose(lastVal);
            auto* lvSparseSet = TryGet(lvIndex);
            auto mappedLvIndex = Math::FastMod(lvIndex, SPARSE_SET_PAGE_SIZE);
            ENGINE_CORE_ASSERT((*lvSparseSet)[mappedLvIndex] != m_NullFlag, "Catastrophic failure.")
            std::swap(m_Dense[(*sparseSet)[mappedIndex]], m_Dense.back());
            std::swap((*sparseSet)[mappedIndex], (*lvSparseSet)[mappedLvIndex]);
        }
        ENGINE_CORE_ASSERT(!m_Dense.empty(), "Set is empty")
        (*sparseSet)[mappedIndex] = m_NullFlag;
        m_Dense.pop_back();
    }

    template <typename ST, typename DT, typename Dec>
    template <typename PopCallback, typename SwapCallback>
    void SparseSetPaged<ST, DT, Dec>::Pop(DT value, PopCallback popCallback, SwapCallback swapCallback)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(sparseSet, "Invalid value.")
        ENGINE_CORE_ASSERT((*sparseSet)[mappedIndex] != m_NullFlag, "No such value.")
        // We keep the continuous layout,
        // w/o need to maintain original order,
        // so instead of classic shift left (vector-like) operation,
        // we swap with the last and pop the last.
        if (m_Dense.size() > 1)
        {
            DT lastVal = m_Dense.back();
            auto&& [lvGen, lvIndex] = Dec::Decompose(lastVal);
            auto* lvSparseSet = TryGet(lvIndex);
            auto mappedLvIndex = Math::FastMod(lvIndex, SPARSE_SET_PAGE_SIZE);
            ENGINE_CORE_ASSERT((*lvSparseSet)[mappedLvIndex] != m_NullFlag, "Catastrophic failure.")
            popCallback((*sparseSet)[mappedIndex]);
            swapCallback((*sparseSet)[mappedIndex], static_cast<ST>(m_Dense.size() - 1));
            std::swap(m_Dense[(*sparseSet)[mappedIndex]], m_Dense.back());
            std::swap((*sparseSet)[mappedIndex], (*lvSparseSet)[mappedLvIndex]);
        }
        ENGINE_CORE_ASSERT(!m_Dense.empty(), "Set is empty")
        (*sparseSet)[mappedIndex] = m_NullFlag;
        m_Dense.pop_back();
    }

    template <typename ST, typename DT, typename Dec>
    bool SparseSetPaged<ST, DT, Dec>::Has(DT value) const
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        if (sparseSet == nullptr) return false;
        ST sparseI = (*sparseSet)[mappedIndex];
        return sparseI < static_cast<ST>(m_Dense.size()) && m_Dense[sparseI] == value;
    }

    template <typename ST, typename DT, typename Dec>
    ST SparseSetPaged<ST, DT, Dec>::GetIndexOf(DT value) const
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        ENGINE_CORE_ASSERT(sparseSet, "No such value.")
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        return (*sparseSet)[mappedIndex];
    }

    template <typename ST, typename DT, typename Dec>
    DT& SparseSetPaged<ST, DT, Dec>::operator[](DT value)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(sparseSet, "Invalid index.")
        return (*sparseSet)[mappedIndex];
    }

    template <typename ST, typename DT, typename Dec>
    const DT& SparseSetPaged<ST, DT, Dec>::operator[](DT value) const
    {
        auto&& [gen, index] = Dec::Decompose(value);
        auto* sparseSet = TryGet(index);
        auto mappedIndex = Math::FastMod(index, SPARSE_SET_PAGE_SIZE);
        ENGINE_CORE_ASSERT(sparseSet, "Invalid index.")
        return (*sparseSet)[mappedIndex];
    }

    template <typename ST, typename DT, typename Dec>
    std::vector<ST>* SparseSetPaged<ST, DT, Dec>::GetOrCreate(U32 index)
    {
        U32 pageNum = index >> SPARSE_SET_PAGE_SIZE_LOG;
        if (pageNum >= m_SparsePaged.size())
        {
            m_SparsePaged.resize(pageNum + 1);
        }
        if (!m_SparsePaged[pageNum])
        {
            m_SparsePaged[pageNum] = CreateRef<std::vector<ST>>(SPARSE_SET_PAGE_SIZE, m_NullFlag);
        }
        return m_SparsePaged[pageNum].get();
    }

    template <typename ST, typename DT, typename Dec>
    const std::vector<ST>* SparseSetPaged<ST, DT, Dec>::TryGet(U32 index) const
    {
        U32 pageNum = index >> SPARSE_SET_PAGE_SIZE_LOG;
        if (pageNum >= m_SparsePaged.size())
        {
            return nullptr;
        }
        return m_SparsePaged[pageNum].get();
    }

    template <typename ST, typename DT, typename Dec>
    std::vector<ST>* SparseSetPaged<ST, DT, Dec>::TryGet(U32 index)
    {
        return const_cast<std::vector<ST>*>(const_cast<const SparseSetPaged*>(this)->TryGet(index));
    }
}