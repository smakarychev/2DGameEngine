#pragma once

#include <glm/glm.hpp>

#include "Shader.h"

namespace Engine
{
	class VertexArray;
	class RendererAPI
	{
	public:
		enum class APIType 
		{
			None = 0, OpenGL
		};

		enum class Mode
		{
			Read, ReadWrite
		};

		enum class PrimitiveType
		{
			Triangle, Line
		};

		enum class TranslucencyType
		{
			Opaque, Translucent
		};

		enum class CullFrontFace
		{
			Clockwise, CounterClockwise
		};

		enum class DataType
		{
			UByte, Byte, UInt, Int, Float
		};

	public:
		virtual ~RendererAPI() = default;
		
		virtual void Init() = 0;
		
		virtual void ClearScreen() = 0;
		virtual void SetClearColor(const glm::vec3& color) = 0;

		virtual void DrawIndexed(Ref<VertexArray> vertexArray) = 0;
		virtual void DrawIndexed(Ref<VertexArray> vertexArray, U32 count) = 0;

		virtual void DrawIndexed(Ref<VertexArray> vertexArray, PrimitiveType type) = 0;
		virtual void DrawIndexed(Ref<VertexArray> vertexArray, U32 count, PrimitiveType type) = 0;

		virtual void EnableCull(bool enable) = 0;
		virtual void SetCullFrontFace(CullFrontFace face) = 0;
		virtual void SetDepthTestMode(Mode mode) = 0;
		virtual void SetViewport(U32 width, U32 height) = 0;

		static U32 GetNativeDataType(RendererAPI::DataType type);

		static APIType Get() { return s_APIType; }

		static Ref<RendererAPI> Create();
	private:
		static APIType s_APIType;
	};
}
