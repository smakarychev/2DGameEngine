#pragma once

#include "Engine/Rendering/Buffer.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
	U32 GetOpenGLAPIType(LayoutElement type);

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* data, U32 size);
		~OpenGLVertexBuffer();
		U32 GetId() const override { return m_Id; }

		void SetVertexLayout(const VertexLayout& layout) override { m_Layout = layout; }
		VertexLayout& GetVertexLayout() override { return m_Layout; };

		void SetData(void* data, U32 size, U32 offset = 0) override;

	private:
		void* m_Data;
		U32 m_Size;
		U32 m_Id;	
		VertexLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(U32* data, U32 count);
		~OpenGLIndexBuffer();
		U32 GetId() const override { return m_Id; }

		U32 GetCount() const override { return m_Count; }

		void SetData(U32* data, U32 count, U32 offset = 0) override;

	private:
		void* m_Data;
		U32 m_Id;
		U32 m_Count;
	};

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray();

		void AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer) override;
		void SetIndexBuffer(std::shared_ptr<IndexBuffer> buffer) override;
		
		std::shared_ptr<IndexBuffer> GetIndexBuffer() override { return m_IndexBuffer; };
		std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() { return m_VertexBuffers; }

		void Bind() override;
		U32 GetId() const override { return m_Id; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		U32 m_VertexBufferIndex;
		U32 m_AttributeIndex;
		U32 m_Id;
	};

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(FrameBuffer::Spec spec);
		~OpenGLFrameBuffer();
		void Bind() override;
		void Unbind() override;
		void Resize(U32 width, U32 height) override;
		const Spec& GetSpec() const override { return m_Spec; }
		U32 GetColorBufferId(U32 colorBufferIndex) const override;
		Texture& GetColorBuffer(U32 colorBufferIndex) const override { return *m_TextureAttachments[colorBufferIndex]; }
		PixelData ReadPixel(U32 textureBufferIndex, U32 x, U32 y, RendererAPI::DataType dataType) const override;
		void ClearAttachment(U32 colorBufferIndex, RendererAPI::DataType dataType, void* value) override;
	private:
		void CreateBuffers();
		Texture::TextureData CreateTextureDataFromAttachment(Spec::AttachmentFormat type, U32 attachmentIndex = 0);
		U32 CreateRenderBufferFromAttachment(Spec::AttachmentFormat type, U32 attachmentIndex = 0) const;
	private:
		U32 m_Id;
		std::vector<Ref<Texture>> m_TextureAttachments;
		std::vector<U32> m_RenderBufferAttachments;
		Spec m_Spec;
	};

}