#pragma once

#include <filesystem>

#include "Engine/Core/Types.h"
#include <glm/glm.hpp>

namespace Engine
{
	using namespace Types;

	class Texture
	{
	public:
		struct TextureData
		{
			std::string  Name = "Default";
			U8* Data = nullptr;
			U32 Width = 0, Height = 0, Channels = 0;
		};

		enum class PixelFormat
		{
			None = 0,
			Red, RedInteger,
			RG, RGInteger,
			RGB, RGBInteger,
			RGBA, RGBAInteger,
			Depth, DepthStencil,
			Alpha
		};

	public:
		virtual ~Texture() {}
		static std::shared_ptr<Texture> LoadTextureFromFile(const std::filesystem::path& path);
		static std::shared_ptr<Texture> Create(const TextureData& textureData);

		virtual U32 GetId() const = 0;

		virtual void Bind(U32 slot = 0) = 0;
		virtual void UpdateData(void* data, PixelFormat format = PixelFormat::RGBA) = 0;
		virtual void UpdateData(U32 width, U32 height, void* data, PixelFormat format = PixelFormat::RGBA) = 0;

		virtual std::shared_ptr<Texture> GetSubTexture(const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize = glm::vec2(1)) = 0;
		static std::shared_ptr<Texture> GetSubTexture(Texture& texture, const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize = glm::vec2(1));

		virtual std::vector<glm::vec2> GetSubTextureUV(const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize = glm::vec2(1)) = 0;
	};

}
