#include "enginepch.h"

#include "RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Engine
{
	RendererAPI::APIType RendererAPI::s_APIType = RendererAPI::APIType::OpenGL;
	std::shared_ptr<RendererAPI> RendererAPI::Create()
	{
		return std::make_shared<OpenGLRendererAPI>();
	}
}