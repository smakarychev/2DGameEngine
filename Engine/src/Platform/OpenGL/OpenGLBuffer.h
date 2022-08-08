#pragma once

#include "Engine/Rendering/Buffer.h"

namespace Engine
{
	U32 GetOpenGLAPIType(LayoutElement type);

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(void* data, U32 size);
		U32 GetId() const override { return m_Id; }

		void SetVertexLayout(const VertexLayout& layout) override { m_Layout = layout; }
		VertexLayout& GetVertexLayout() override { return m_Layout; };

	private:
		void* m_Data;
		U32 m_Size;
		U32 m_Id;	
		VertexLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(void* data, U32 size);
		U32 GetId() const override { return m_Id; }
	private:
		void* m_Data;
		U32 m_Size;
		U32 m_Id;
	};

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();

		void AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer) override;
		void SetIndexBuffer(std::shared_ptr<IndexBuffer> buffer) override;

		void Bind() override;
		U32 GetId() const override { return m_Id; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		U32 m_VertexBufferIndex;
		U32 m_AttributeIndex;
		U32 m_Id;
	};
}