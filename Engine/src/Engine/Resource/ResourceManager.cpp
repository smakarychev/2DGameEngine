#include "enginepch.h"

#include "ResourceManager.h"

#include "Engine/Core/Log.h"

#include <stb_image/stb_image.h>

namespace Engine
{
	std::string ResourceManager::ReadFile(const std::filesystem::path& path)
	{
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in)
		{
			return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		}
		ENGINE_CORE_ERROR("ResourceManager: error reading file {}", path.string());
		return "";
	}

	void ResourceManager::ShutDown()
	{
		ShaderLoader::ShutDown();
		TextureLoader::ShutDown();
	}

	std::unordered_map<std::string, std::shared_ptr<Shader>> ShaderLoader::s_LoadedShaders;

	std::shared_ptr<Shader> ShaderLoader::LoadShaderFromFile(const std::filesystem::path& path)
	{
		std::string pathString = path.string();
		auto it = s_LoadedShaders.find(pathString);
		if (it != s_LoadedShaders.end()) return it->second;

		std::string shaderFileContent = ResourceManager::ReadFile(path);

		std::string shaderName = path.filename().string();

		std::shared_ptr<Shader> shader = Shader::CreateShaderFromSource(shaderName, shaderFileContent);
		s_LoadedShaders.emplace(pathString, shader);
		return shader;
	}
	void ShaderLoader::ShutDown()
	{
		s_LoadedShaders.clear();
	}

	std::unordered_map<std::string, std::shared_ptr<Texture>> TextureLoader::s_LoadedTextures;

	std::shared_ptr<Texture> TextureLoader::LoadTextureFromFile(const std::filesystem::path& path)
	{
		// TODO: maybe move in somewhere else (it is one assignment).
		stbi_set_flip_vertically_on_load(true);

		std::string pathString = path.string();
		auto it = s_LoadedTextures.find(pathString);
		if (it != s_LoadedTextures.end()) return it->second;

		std::string textureName = path.filename().string();

		Texture::TextureData textureData;
		I32 width, height, channels;
		
		textureData.Data = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
		if (textureData.Data == nullptr) ENGINE_CORE_ERROR("Failed to load texture: {}", pathString);
		textureData.Width = U32(width);
		textureData.Height = U32(height);
		textureData.Channels = U32(channels);
		textureData.Name = textureName;

		std::shared_ptr<Texture> texture = Texture::Create(textureData);
		stbi_image_free(textureData.Data);

		s_LoadedTextures.emplace(pathString, texture);
		return texture;
	}
	
	void TextureLoader::ShutDown()
	{
		s_LoadedTextures.clear();
	}
}