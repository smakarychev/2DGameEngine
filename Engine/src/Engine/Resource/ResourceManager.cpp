#include "enginepch.h"

#include "ResourceManager.h"

#include "Engine/Core/Log.h"
#include "Engine/Rendering/Shader.h"

namespace Engine
{
	std::string ResourceManager::ReadFile(const std::string& path)
	{
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in)
		{
			return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		}
		ENGINE_CORE_ERROR("ResourceManager: error reading file {}", path);
		return "";
	}

	std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderLoader::s_LoadedShaders;

	std::shared_ptr<Shader> ShaderLoader::LoadShaderFromFile(const std::string name, const std::string& path)
	{
		auto it = s_LoadedShaders.find(path);
		if (it != s_LoadedShaders.end()) return it->second;

		std::string shaderFileContent = ResourceManager::ReadFile(path);

		std::shared_ptr<Shader> shader = Shader::CreateShaderFromSource(name, shaderFileContent);
		s_LoadedShaders.emplace(path, shader);
		return shader;
	}
	void ShaderLoader::ShutDown()
	{
		s_LoadedShaders.clear();
	}
}