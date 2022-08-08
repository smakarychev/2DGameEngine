#include "enginepch.h"

#include "Buffer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Engine
{

    std::shared_ptr<VertexBuffer> VertexBuffer::Create(void* data, U32 size)
    {
        return std::make_shared<OpenGLVertexBuffer>(data, size);
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::Create(void* data, U32 size)
    {
        return std::make_shared<OpenGLIndexBuffer>(data, size);
    }

    std::shared_ptr<VertexArray> VertexArray::Create()
    {
        return std::make_shared<OpenGLVertexArray>();
    }

    U32 GetNativeAPIType(LayoutElement type)
    {
        return GetOpenGLAPIType(type);
    }

}
