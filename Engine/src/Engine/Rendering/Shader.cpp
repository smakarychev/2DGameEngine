#include "enginepch.h"

#include "Shader.h"

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Resource/ResourceManager.h"

#include "Engine/Platform/OpenGL/OpenGLShader.h"

namespace Engine
{
	Ref<Shader> Shader::ReadShaderFromFile(const std::filesystem::path& path)
	{

		return ShaderLoader::LoadShaderFromFile(path);
	}

	Ref<Shader> Shader::CreateShaderFromSource(const std::string& name, const std::string& source)
	{

		return CreateRef<OpenGLShader>(name, source);
	}
}


