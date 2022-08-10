#include "enginepch.h"
#include "OpenGLBuffer.h"

#include "glad/glad.h"

namespace Engine
{
	OpenGLVertexBuffer::OpenGLVertexBuffer(void* data, U32 size) :
		m_Data(data), m_Size(size)
	{
		glCreateBuffers(1, &m_Id);
		// I guess I saw this in bgfx.
		GLenum usage = (data == nullptr) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glNamedBufferData(m_Id, size, data, usage);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(void* data, U32 count, U32 size) :
		m_Data(data), m_Count(count), m_Size(size)
	{
		glCreateBuffers(1, &m_Id);
		// I guess I saw this in bgfx.
		GLenum usage = (data == nullptr) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glNamedBufferData(m_Id, size, data, usage);
	}

	U32 GetOpenGLAPIType(LayoutElement type)
	{

		switch (type)
		{
		case Engine::LayoutElement::Int:
		case Engine::LayoutElement::Int2:
		case Engine::LayoutElement::Int3:
		case Engine::LayoutElement::Int4:
			return GL_INT;
		case Engine::LayoutElement::UInt:
		case Engine::LayoutElement::UInt2:
		case Engine::LayoutElement::UInt3:
		case Engine::LayoutElement::UInt4:
			return GL_UNSIGNED_INT;
		case Engine::LayoutElement::Float:
		case Engine::LayoutElement::Float2:
		case Engine::LayoutElement::Float3:
		case Engine::LayoutElement::Float4:
			return GL_FLOAT;
		}
		ENGINE_CORE_ERROR("Buffer: unknown type");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray() : m_VertexBufferIndex(0), m_AttributeIndex(0)
	{
		glCreateVertexArrays(1, &m_Id);
	}

	void OpenGLVertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer)
	{
		m_VertexBuffers.push_back(buffer);
		glVertexArrayVertexBuffer(m_Id, m_VertexBufferIndex, buffer->GetId(), 0, (GLsizei)buffer->GetVertexLayout().Stride);

		for (auto& layoutElement : buffer->GetVertexLayout())
		{
			glEnableVertexArrayAttrib(m_Id, m_AttributeIndex);
			glVertexArrayAttribBinding(m_Id, m_AttributeIndex, m_VertexBufferIndex);
			glVertexArrayAttribFormat(m_Id,
				m_AttributeIndex,
				layoutElement.Count,
				GetOpenGLAPIType(layoutElement.Type),
				layoutElement.Normalized ? GL_TRUE : GL_FALSE,
				layoutElement.Offset
			);
			m_AttributeIndex++;
		}
		m_VertexBufferIndex++;
	}
	
	void OpenGLVertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> buffer)
	{
		m_IndexBuffer = buffer;
		glVertexArrayElementBuffer(m_Id, buffer->GetId());
	}
	
	void OpenGLVertexArray::Bind()
	{
		glBindVertexArray(m_Id);
	}
}


