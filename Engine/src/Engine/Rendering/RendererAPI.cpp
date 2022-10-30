#include "enginepch.h"

#include "RendererAPI.h"
#include "Buffer.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Engine
{
	RendererAPI::APIType RendererAPI::s_APIType = RendererAPI::APIType::OpenGL;
	Ref<RendererAPI> RendererAPI::Create()
	{
		return std::make_shared<OpenGLRendererAPI>();
	}
	
	U32 RendererAPI::GetNativeDataType(RendererAPI::DataType type)
	{
		return OpenGLRendererAPI::GetNativeDataType(type);
	}
}