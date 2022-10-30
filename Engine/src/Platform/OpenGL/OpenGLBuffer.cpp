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

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_Id);
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

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_Id);
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

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_Id);
	}

	void OpenGLVertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer)
	{
		m_VertexBuffers.push_back(buffer);
		glVertexArrayVertexBuffer(m_Id, m_VertexBufferIndex, buffer->GetId(), 0, (GLsizei)buffer->GetVertexLayout().Stride);

		for (auto& layoutElement : buffer->GetVertexLayout())
		{
			glEnableVertexArrayAttrib(m_Id, m_AttributeIndex);
			glVertexArrayAttribBinding(m_Id, m_AttributeIndex, m_VertexBufferIndex);
			glVertexArrayAttribFormat(
				(GLint)m_Id,
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

	OpenGLFrameBuffer::OpenGLFrameBuffer(FrameBuffer::Spec spec) :
		m_Id(0), m_Spec(std::move(spec))
	{
		CreateBuffers();
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_Id);
		for (auto& texAttachment : m_TextureAttachments) texAttachment.reset();
		for (auto& rbAttachment : m_RenderBufferAttachments) glDeleteBuffers(1, &rbAttachment);
		m_TextureAttachments.clear();
		m_RenderBufferAttachments.clear();
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

	U32 OpenGLFrameBuffer::GetColorBufferId(U32 colorBufferIndex) const
	{
		return m_ColorAttachments[colorBufferIndex]->GetId();
	}

	void OpenGLFrameBuffer::CreateBuffers()
	{
		if (m_Id != 0)
		{
			glDeleteFramebuffers(1, &m_Id);
			for (auto& texAttachment : m_TextureAttachments) texAttachment.reset();
			for (auto& texAttachment : m_ColorAttachments) texAttachment.reset();
			for (auto rbAttachment : m_RenderBufferAttachments) glDeleteRenderbuffers(1, &rbAttachment);
			m_TextureAttachments.clear();
			m_RenderBufferAttachments.clear();
			m_ColorAttachments.clear();
		}
			
		glCreateFramebuffers(1, &m_Id);

		for (auto& attachment : m_Spec.Attachments)
		{
			switch (attachment.Category)
			{
			case Spec::AttachmentCategory::ReadWrite:
			{
				U32 currentIndex = (U32)m_TextureAttachments.size();
				Texture::TextureData data = CreateTextureDataFromAttachment(attachment.Type, currentIndex);
				m_TextureAttachments.emplace_back(Texture::Create(data));
				if (attachment.Type == Spec::AttachmentFormat::Color)
				{
					m_ColorAttachments.emplace_back(m_TextureAttachments.back());
				}
				glNamedFramebufferTexture(m_Id, GL_COLOR_ATTACHMENT0 + currentIndex, m_TextureAttachments.back()->GetId(), 0);
				break;
			}
			case Spec::AttachmentCategory::Write:
			{
				U32 currentIndex = (U32)m_RenderBufferAttachments.size();
				m_RenderBufferAttachments.emplace_back(CreateRenderBufferFromAttachment(attachment.Type, currentIndex));
				break;
			}
			}
		}
			
		if (glCheckNamedFramebufferStatus(m_Id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ENGINE_CORE_FATAL("Framebuffer is not complete. Status: {}", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	PixelData OpenGLFrameBuffer::ReadPixel(U32 textureBufferIndex, U32 x, U32 y, RendererAPI::DataType dataType)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT0 + textureBufferIndex);
		return GetAttachment(textureBufferIndex).ReadPixel(x, y, dataType);
	}

	Texture::TextureData OpenGLFrameBuffer::CreateTextureDataFromAttachment(Spec::AttachmentFormat type, U32 attachmentIndex)
	{
		Texture::TextureData data;
		data.Width = m_Spec.Width; data.Height = m_Spec.Height;
		data.Data = nullptr;
		data.WrapS = data.WrapT = Texture::WrapMode::None;
		data.GenerateMipmaps = false;
		switch (type)
		{
		case Engine::FrameBuffer::Spec::AttachmentFormat::Color:
			data.PixelFormat = Texture::PixelFormat::RGB;
			data.Minification = data.Magnification = Texture::Filter::Linear;
			data.Name = std::format("fb{}color{}", m_Id, attachmentIndex);
			break;
		case Engine::FrameBuffer::Spec::AttachmentFormat::Depth24Stencil8:
		{
			data.PixelFormat = Texture::PixelFormat::DepthStencil;
			data.Minification = data.Magnification = Texture::Filter::Nearest;
			data.Name = std::format("fb{}depthStencil{}", m_Id, attachmentIndex);
			break;
		}
		case Engine::FrameBuffer::Spec::AttachmentFormat::RedInteger:
			data.PixelFormat = Texture::PixelFormat::RedInteger;
			data.Minification = data.Magnification = Texture::Filter::Nearest;
			data.Name = std::format("fb{}red_integer{}", m_Id, attachmentIndex);
			break;
		default:
			break;
		}
		return data;
	}
	
	U32 OpenGLFrameBuffer::CreateRenderBufferFromAttachment(Spec::AttachmentFormat type, U32 attachmentIndex)
	{
		U32 renderBufferId = 0;

		glCreateRenderbuffers(1, &renderBufferId);
		switch (type)
		{
		case Engine::FrameBuffer::Spec::AttachmentFormat::Color:
			glNamedRenderbufferStorage(renderBufferId, GL_RGB8, m_Spec.Width, m_Spec.Height);
			glNamedFramebufferRenderbuffer(m_Id, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_RENDERBUFFER, renderBufferId);
			break;
		case Engine::FrameBuffer::Spec::AttachmentFormat::Depth24Stencil8:
		{
			glNamedRenderbufferStorage(renderBufferId, GL_DEPTH24_STENCIL8, m_Spec.Width, m_Spec.Height);
			glNamedFramebufferRenderbuffer(m_Id, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferId);
			break;
		}
		case Engine::FrameBuffer::Spec::AttachmentFormat::RedInteger:
			glNamedRenderbufferStorage(renderBufferId, GL_R32I, m_Spec.Width, m_Spec.Height);
			glNamedFramebufferRenderbuffer(m_Id, GL_COLOR_ATTACHMENT0 + attachmentIndex, GL_RENDERBUFFER, renderBufferId);
			break;
		default:
			break;
		}
		
		return renderBufferId;
	}
}


