#include "enginepch.h"

#include "Texture.h"

#include "Engine/Resource/ResourceManager.h"

#include "Platform/OpenGL/OpenGLTexture.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
	Ref<Texture> Texture::LoadTextureFromFile(const std::filesystem::path& path)
	{
		return TextureLoader::LoadTextureFromFile(path);
	}
	
	Ref<Texture> Texture::Create(const TextureData& textureData)
	{
		return CreateRef<OpenGLTexture>(textureData);
	}
	Ref<Texture> Texture::GetSubTexture(Texture& texture, const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize)
	{
		return texture.GetSubTexture(tileSize, subtexCoords, subtexSize);
	}
	Ref<Texture> GetSubTexturePixels(Texture& texture, const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx)
	{
		return texture.GetSubTexturePixels(subtexCoordsPx, subtexSizePx);
	}
}