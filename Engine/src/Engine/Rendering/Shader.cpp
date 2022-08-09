#include "enginepch.h"

#include "Shader.h"

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Resource/ResourceManager.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Engine
{
	std::shared_ptr<Shader> Shader::ReadShaderFromFile(const std::string name, const std::string& path)
	{

		return ShaderLoader::LoadShaderFromFile(name, path);
	}

	std::shared_ptr<Shader> Shader::CreateShaderFromSource(const std::string name, const std::string& source)
	{

		return std::shared_ptr<Shader>(New<OpenGLShader>(name, source), Delete<OpenGLShader>);
	}
}


