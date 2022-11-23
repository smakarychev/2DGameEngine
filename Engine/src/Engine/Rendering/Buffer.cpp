#include "enginepch.h"

#include "Buffer.h"

#include "Engine/Memory/MemoryManager.h"

#include "Engine/Platform/OpenGL/OpenGLBuffer.h"

namespace Engine
{

    Ref<VertexBuffer> VertexBuffer::Create(void* data, U32 size)
    {
        return CreateRef<OpenGLVertexBuffer>(data, size);
    }

    Ref<IndexBuffer> IndexBuffer::Create(U32* data, U32 count)
    {
        return CreateRef<OpenGLIndexBuffer>(data, count);
    }

    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OpenGLVertexArray>();
    }

    U32 GetNativeAPIType(LayoutElement type)
    {
        return GetOpenGLAPIType(type);
    }

    Ref<FrameBuffer> FrameBuffer::Create(const Spec& spec)
    {
        return CreateRef<OpenGLFrameBuffer>(spec);
    }

}
