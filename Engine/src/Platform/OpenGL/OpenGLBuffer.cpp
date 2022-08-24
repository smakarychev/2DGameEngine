#include "enginepch.h"
#include "OpenGLBuffer.h"

#include "Engine/Rendering/Texture.h"

#include <glad/glad.h>

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

	void OpenGLVertexBuffer::SetData(void* data, U32 size, U32 offset)
	{
		glNamedBufferSubData(m_Id, offset, size, data);
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(U32* data, U32 count) :
		m_Data(data), m_Count(count)
	{
		glCreateBuffers(1, &m_Id);
		// I guess I saw this in bgfx.
		GLenum usage = (data == nullptr) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
		glNamedBufferData(m_Id, sizeof(U32) * count, data, usage);
	}

	void OpenGLIndexBuffer::SetData(U32* data, U32 count, U32 offset)
	{
		glNamedBufferSubData(m_Id, offset, sizeof(U32) * count, data);
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

	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBuffer::Spec& spec) :
		m_Id(0), m_Spec(spec)
	{
		CreateBuffers();
	}

	void OpenGLFrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Id);
	}

	void OpenGLFrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(U32 width, U32 height)
	{
		m_Spec.Width = width;
		m_Spec.Height = height;
		CreateBuffers();
	}

	U32 OpenGLFrameBuffer::GetColorBufferId() const
	{
		return m_ColorBuffer->GetId();
	}

	void OpenGLFrameBuffer::CreateBuffers()
	{
		if (m_Id != 0)
		{
			glDeleteFramebuffers(1, &m_Id);
			m_ColorBuffer->~Texture();
			glDeleteRenderbuffers(1, &m_DepthStencilBufferId);
		}
			
		glCreateFramebuffers(1, &m_Id);

		Texture::TextureData data;
		data.Channels = 3;
		data.Width = m_Spec.Width;	data.Height = m_Spec.Height;
		data.Data = nullptr;
		data.Name = std::format("fb{}color", m_Id);
		data.WrapS = data.WrapT = Texture::WrapMode::None;
		data.Minification = data.Magnification =  Texture::Filter::Linear;
		data.GenerateBitmaps = false;
		m_ColorBuffer = Texture::Create(data);
		glNamedFramebufferTexture(m_Id, GL_COLOR_ATTACHMENT0, m_ColorBuffer->GetId(), 0);

		glCreateRenderbuffers(1, &m_DepthStencilBufferId);
		glNamedRenderbufferStorage(m_DepthStencilBufferId, GL_DEPTH24_STENCIL8, m_Spec.Width, m_Spec.Height);
		glNamedFramebufferRenderbuffer(m_Id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthStencilBufferId);

		if (glCheckNamedFramebufferStatus(m_Id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ENGINE_CORE_FATAL("Framebuffer is not complete. Status: {}", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}


