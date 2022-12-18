﻿#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

namespace  Engine
{
    using namespace Types;

    // ST - sparse type, DT - dense type, Dec - dense type decomposer.
    template <typename ST, typename DT, typename Dec>
    class SparseSet
    {
    public:
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
        SparseSet(ST nullFlag = std::numeric_limits<ST>::max());
        void Reserve(U32 count);
        ST Push(DT value);
        template <typename PushCallback>
        ST Push(DT value, PushCallback callback = [](ST value){});
        void Pop(DT value);
        template <typename PopCallback, typename SwapCallback>
        void Pop(DT value, PopCallback popCallback = [](ST value){}, SwapCallback swapCallback = [](ST a, ST b) {});
        // Checks if value, that was generated by this set, still belongs to it. 
        bool Has(DT value) const;
        
        DT& operator[](DT value);
        const DT& operator[](DT value) const;

        Iterator begin() { return Iterator(m_Dense, static_cast<U32>(m_Dense.size() - 1)); }
        Iterator end() { return Iterator(m_Dense, -1); }
        
        const std::vector<ST>& GetSparse() const { return m_Sparse; }
        const std::vector<DT>& GetDense() const { return m_Dense; }
        ST GetNullFlag() const { return m_NullFlag; }
    private:
        void Enlarge(ST index);
    private:
        std::vector<ST> m_Sparse;
        std::vector<DT> m_Dense;
        const ST m_NullFlag;
    };

    template <typename ST, typename DT, typename Dec>
    SparseSet<ST, DT, Dec>::SparseSet(ST nullFlag)
        : m_NullFlag(nullFlag)
    {
        m_Sparse.resize(16, nullFlag);
    }

    template <typename ST, typename DT, typename Dec>
    void SparseSet<ST, DT, Dec>::Reserve(U32 count)
    {
        m_Sparse.resize(count, m_NullFlag);
        m_Dense.reserve(count);
    }

    template <typename ST, typename DT, typename Dec>
    ST SparseSet<ST, DT, Dec>::Push(DT value)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        if (index >= static_cast<ST>(m_Sparse.size()))
        {
            Enlarge(index);
        }
        ST sparseI = m_Sparse[index];
        ENGINE_CORE_ASSERT(!Has(value), "Value is already set.")
        if (sparseI == m_NullFlag)
        {
            m_Sparse[index] = static_cast<ST>(m_Dense.size());
            m_Dense.emplace_back(value);
        }
        else
        {
            m_Dense[sparseI] = value;
        }
        return sparseI;
    }

    template <typename ST, typename DT, typename Dec>
    template <typename PushCallback>
    ST SparseSet<ST, DT, Dec>::Push(DT value, PushCallback callback)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        if (index >= static_cast<ST>(m_Sparse.size()))
        {
            Enlarge(index);
        }
        ST sparseI = m_Sparse[index];
        ENGINE_CORE_ASSERT(!Has(value), "Value is already set.")
        ENGINE_CORE_ASSERT(m_Sparse[index] == m_NullFlag, "Catastrophic failure.")
        m_Sparse[index] = static_cast<ST>(m_Dense.size());
        m_Dense.emplace_back(value);
        callback(m_Sparse[index]);
        return sparseI;
    }

    template <typename ST, typename DT, typename Dec>
    void SparseSet<ST, DT, Dec>::Pop(DT value)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        ENGINE_CORE_ASSERT(index < m_Sparse.size(), "Index out of bounds.")
        ENGINE_CORE_ASSERT(m_Sparse[index] != m_NullFlag, "No such value.")
        // We keep the continuous layout,
        // w/o need to maintain original order,
        // so instead of classic shift left (vector-like) operation,
        // we swap with the last and pop the last.
        if (m_Dense.size() > 1)
        {
            DT lastVal = m_Dense.back();
            auto&& [lvGen, lvIndex] = Dec::Decompose(lastVal);
            ENGINE_CORE_ASSERT(m_Sparse[lvIndex] != m_NullFlag, "Catastrophic failure.")
            std::swap(m_Dense[m_Sparse[index]], m_Dense.back());
            std::swap(m_Sparse[index], m_Sparse[lvIndex]);
        }
        ENGINE_CORE_ASSERT(!m_Dense.empty(), "Set is empty")
        m_Sparse[index] = m_NullFlag;
        m_Dense.pop_back();
    }

    template<typename ST, typename DT, typename Dec>
    template<typename PopCallback, typename SwapCallback>
    inline void SparseSet<ST, DT, Dec>::Pop(DT value, PopCallback popCallback, SwapCallback swapCallback)
    {
        auto&& [gen, index] = Dec::Decompose(value);
        ENGINE_CORE_ASSERT(index < m_Sparse.size(), "Index out of bounds.")
        ENGINE_CORE_ASSERT(m_Sparse[index] != m_NullFlag, "No such value.")
        if (m_Dense.size() > 1)
        {
            DT lastVal = m_Dense.back();
            auto&& [lvGen, lvIndex] = Dec::Decompose(lastVal);
            ENGINE_CORE_ASSERT(m_Sparse[lvIndex] != m_NullFlag, "Catastrophic failure.")
            swapCallback(m_Sparse[index], static_cast<ST>(m_Dense.size() - 1));
            std::swap(m_Dense[m_Sparse[index]], m_Dense.back());
            std::swap(m_Sparse[index], m_Sparse[lvIndex]);
        }
        ENGINE_CORE_ASSERT(!m_Dense.empty(), "Set is empty.")
        popCallback(m_Sparse[index]);
        m_Sparse[index] = m_NullFlag;
        m_Dense.pop_back();
    }

    template <typename ST, typename DT, typename Dec>
    bool SparseSet<ST, DT, Dec>::Has(DT value) const
    {
        auto&& [gen, index] = Dec::Decompose(value);
        if (index >= m_Sparse.size()) return false;
        ST sparseI = m_Sparse[index];
        return sparseI < static_cast<ST>(m_Dense.size()) && m_Dense[sparseI] == value;
    }

    template <typename ST, typename DT, typename Dec>
    DT& SparseSet<ST, DT, Dec>::operator[](DT value)
    {
        return const_cast<DT>(const_cast<const SparseSet*>(this)[value]);
    }

    template <typename ST, typename DT, typename Dec>
    const DT& SparseSet<ST, DT, Dec>::operator[](DT value) const
    {
        ENGINE_CORE_ASSERT(Has(value), "No such value.")
        auto&& [gen, index] = Dec::Decompose(value);
        return m_Dense[m_Sparse[index]];
    }

    template <typename ST, typename DT, typename Dec>
    void SparseSet<ST, DT, Dec>::Enlarge(ST index)
    {
        U32 currentCapacity = static_cast<U32>(m_Sparse.size());
        while (currentCapacity <= index)
        {
            currentCapacity *= 2;
        }
        m_Sparse.resize(currentCapacity, m_NullFlag);
    }
}

