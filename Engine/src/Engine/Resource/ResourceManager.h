#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

#include <filesystem>

namespace Engine
{
	using namespace Types;

	class ResourceManager
	{
	public:
		static std::string ReadFile(const std::filesystem::path& path);
		static void ShutDown();
	};

	class ShaderLoader
	{
	public:
		static std::shared_ptr<Shader> LoadShaderFromFile(const std::filesystem::path& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, std::shared_ptr<Shader>> s_LoadedShaders;
	};

	class TextureLoader
	{
	public:
		static std::shared_ptr<Texture> LoadTextureFromFile(const std::filesystem::path& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, std::shared_ptr<Texture>> s_LoadedTextures;
	};
}