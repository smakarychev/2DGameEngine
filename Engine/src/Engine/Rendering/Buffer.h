#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Types.h"

#include "RendererAPI.h"

#include <memory>

namespace Engine
{
	using namespace Types;

	enum class LayoutElement
	{
		None = 0, 
		Int, Int2, Int3, Int4, 
		UInt, UInt2, UInt3, UInt4,
		Float, Float2, Float3, Float4
	};

	inline U32 GetLayoutElementCount(LayoutElement type)
	{
		switch (type)
		{
		case Engine::LayoutElement::None:
			return 0;
		case Engine::LayoutElement::Int:
		case Engine::LayoutElement::UInt:
		case Engine::LayoutElement::Float:
			return 1;
		case Engine::LayoutElement::Int2:
		case Engine::LayoutElement::UInt2:
		case Engine::LayoutElement::Float2:
			return 2;
		case Engine::LayoutElement::Int3:
		case Engine::LayoutElement::UInt3:
		case Engine::LayoutElement::Float3:
			return 3;
		case Engine::LayoutElement::Int4:
		case Engine::LayoutElement::UInt4:
		case Engine::LayoutElement::Float4:
			return 4;
		}
		ENGINE_CORE_ERROR("Buffer: unknown type");
		return 0;
	}

	inline U32 GetLayoutElementSizeBytes(LayoutElement type)
	{
		switch (type)
		{
		case Engine::LayoutElement::None:
			return 0;
		case Engine::LayoutElement::Int:
		case Engine::LayoutElement::UInt:
		case Engine::LayoutElement::Float:
			return 4;
		case Engine::LayoutElement::Int2:
		case Engine::LayoutElement::UInt2:
		case Engine::LayoutElement::Float2:
			return 4 * 2;
		case Engine::LayoutElement::Int3:
		case Engine::LayoutElement::UInt3:
		case Engine::LayoutElement::Float3:
			return 4 * 3;
		case Engine::LayoutElement::Int4:
		case Engine::LayoutElement::UInt4:
		case Engine::LayoutElement::Float4:
			return 4 * 4;
		}
		ENGINE_CORE_ERROR("Buffer: unknown type");
		return 0;
	}

	U32 GetNativeAPIType(LayoutElement type);


	struct VertexLayoutElement
	{
		VertexLayoutElement(LayoutElement type, std::string name, bool normalized = false)
			: Type(type), Name(std::move(name)), Offset(0), Normalized(normalized)
		{
			Count = GetLayoutElementCount(type);
		}
		LayoutElement Type = LayoutElement::None;
		std::string Name = "Unnamed";
		U32 Count, Offset;
		bool Normalized;
	};

	struct VertexLayout
	{
		VertexLayout() : Stride(0) {}
		VertexLayout(std::vector<VertexLayoutElement> elements) : Elements(std::move(elements))
		{
			U32 offset = 0;
			for (auto& e : Elements)
			{
				e.Offset = offset;
				offset += GetLayoutElementSizeBytes(e.Type);
			}
			Stride = offset;
		}

		std::vector<VertexLayoutElement>::iterator begin() { return Elements.begin(); }
		std::vector<VertexLayoutElement>::iterator end() { return Elements.end(); }

		std::vector<VertexLayoutElement> Elements;
		U64 Stride;
	};


	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}
		virtual U32 GetId() const = 0;
		virtual void SetVertexLayout(const VertexLayout& layout) = 0;
		virtual VertexLayout& GetVertexLayout() = 0;
		virtual void SetData(void* data, U32 size, U32 offset = 0) = 0;

		static Ref<VertexBuffer> Create(void* data, U32 size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}
		virtual U32 GetId() const = 0;
		virtual U32 GetCount() const = 0;
		virtual void SetData(U32* data, U32 count, U32 offset = 0) = 0;

		static Ref<IndexBuffer> Create(U32* data, U32 count);
	};

	// TODO: see how bgfx avoids this in api.
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}
		virtual U32 GetId() const = 0;

		virtual void AddVertexBuffer(Ref<VertexBuffer> buffer) = 0;
		virtual void SetIndexBuffer(Ref<IndexBuffer> buffer) = 0;

		virtual Ref<IndexBuffer> GetIndexBuffer() = 0;
		virtual std::vector<Ref<VertexBuffer>>& GetVertexBuffers() = 0;

		virtual void Bind() const = 0;

		static Ref<VertexArray> Create();
	};

	class Texture;
	struct PixelData;
	class FrameBuffer
	{
	public:
		struct Spec
		{
			enum class AttachmentFormat { Color, Depth24Stencil8, RedInteger};
			enum class AttachmentCategory { Write, ReadWrite};
			struct AttachmentSpec
			{
				AttachmentFormat Type = AttachmentFormat::Color;
				AttachmentCategory Category = AttachmentCategory::ReadWrite;
			};
			U32 Width = 0;
			U32 Height = 0;
			std::vector<AttachmentSpec> Attachments = {
				{ AttachmentFormat::Color,			 AttachmentCategory::ReadWrite },
				{ AttachmentFormat::Depth24Stencil8, AttachmentCategory::Write }
			};
		};
	public:
		virtual ~FrameBuffer() {}
		
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Resize(U32 width, U32 height) = 0;
		virtual const Spec& GetSpec() const = 0;
		virtual U32 GetColorBufferId(U32 colorBufferIndex) const = 0;
		virtual Texture& GetColorBuffer(U32 colorBufferIndex) const = 0;
		virtual PixelData ReadPixel(U32 colorBufferIndex, U32 x, U32 y, RendererAPI::DataType dataType) const = 0;
		virtual void ClearAttachment(U32 colorBufferIndex, RendererAPI::DataType dataType, void* value) = 0;

		static Ref<FrameBuffer> Create(const Spec& spec);
	};

}
