#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Core/Log.h"

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
		VertexLayoutElement(LayoutElement type, const std::string& name, bool normalized = false)
			: Type(type), Name(name), Normalized(normalized), Offset(0)
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
		VertexLayout(const std::vector<VertexLayoutElement>& elements) : Elements(elements)
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

		static std::shared_ptr<VertexBuffer> Create(void* data, U32 size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}
		virtual U32 GetId() const = 0;
	
		virtual U32 GetCount() const = 0;

		static std::shared_ptr<IndexBuffer> Create(void* data, U32 count, U32 size);
	};

	// TODO: see how bgfx avoids this in api.
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}
		virtual U32 GetId() const = 0;

		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer> buffer) = 0;
		virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer> buffer) = 0;

		virtual std::shared_ptr<IndexBuffer> GetIndexBuffer() = 0;

		virtual void Bind() = 0;

		static std::shared_ptr<VertexArray> Create();
	};
}
