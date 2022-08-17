#include "enginepch.h"

#include "Texture.h"

#include "Engine/Resource/ResourceManager.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	std::shared_ptr<Texture> Texture::LoadTextureFromFile(const std::filesystem::path& path)
	{
		return TextureLoader::LoadTextureFromFile(path);
	}
	
	std::shared_ptr<Texture> Texture::Create(const TextureData& textureData)
	{
		return std::shared_ptr<OpenGLTexture>(New<OpenGLTexture>(textureData), Delete<OpenGLTexture>);
	}
	std::shared_ptr<Texture> Texture::GetSubTexture(Texture& texture, const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize)
	{
		return texture.GetSubTexture(tileSize, subtexCoords, subtexSize);
	}
}