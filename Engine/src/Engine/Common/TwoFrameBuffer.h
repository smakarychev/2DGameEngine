#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"

namespace Engine
{
    using namespace Types;
    
    template <typename Container>
    class TwoFrameBuffer
    {
    public:
        const Container& GetReadBuffer() const { return m_Buffers[m_ReadBufferIndex]; }
        Container& GetReadBuffer() { return m_Buffers[m_ReadBufferIndex]; }
        const Container& GetWriteBuffer() const { return m_Buffers[m_ReadBufferIndex ^ 1]; }
        Container& GetWriteBuffer() { return m_Buffers[m_ReadBufferIndex ^ 1]; }
        const std::array<Container, 2>& GetReadWriteBuffers() const { return {m_Buffers[m_ReadBufferIndex], m_Buffers[m_ReadBufferIndex ^ 1]}; }
        std::array<Container, 2>& GetReadWriteBuffers() { return {m_Buffers[m_ReadBufferIndex], m_Buffers[m_ReadBufferIndex ^ 1]}; }

        // Clears read buffer.
        void Clear() { m_Buffers[m_ReadBufferIndex].clear(); }
        // Swaps buffers and clears old read buffer.
        void Swap() { Clear(); m_ReadBufferIndex ^= 1; }
    private:
        std::array<Container, 2> m_Buffers{};
        U32 m_ReadBufferIndex{0};
    };
}

