#pragma once

#include "Engine/Core/Types.h"


#include <filesystem>

namespace Engine
{
	using namespace Types;

	class ResourceManager
	{
	public:
		static std::string ReadFile(const std::string& path);
	};

	class Shader;
	class ShaderLoader
	{
	public:
		static std::shared_ptr<Shader> LoadShaderFromFile(const std::string name, const std::string& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, std::shared_ptr<Shader>> s_LoadedShaders;
	};
}