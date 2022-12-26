#pragma once


#include "Engine/Core/Types.h"
#include "Engine/Core/Core.h"

#include "RendererAPI.h"

#include <filesystem>
#include <glm/glm.hpp>
#include <utility>

namespace Engine
{
	using namespace Types;

	struct PixelData
	{
		F64 Data;
		operator U8() { return  *reinterpret_cast<U8*>(&Data); }
		operator I8() { return  *reinterpret_cast<I8*>(&Data); }
		operator U16() { return *reinterpret_cast<U16*>(&Data); }
		operator I16() { return *reinterpret_cast<I16*>(&Data); }
		operator U32() { return *reinterpret_cast<U32*>(&Data); }
		operator I32() { return *reinterpret_cast<I32*>(&Data); }
		operator F32() { return *reinterpret_cast<F32*>(&Data); }
		operator F64() { return *reinterpret_cast<F64*>(&Data); }
	};

	class Texture
	{
	public:
		enum class Filter
		{
			None, Nearest, Linear, MipmapNearest, MipmapLinear
		};

		enum class WrapMode
		{
			None, Clamp, Repeat
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

		struct TextureData
		{
			std::string Name = "Default";
			U8* Data = nullptr;
			U32 Width = 0, Height = 0;
			PixelFormat PixelFormat = PixelFormat::RGBA;
			Filter Minification = Filter::Nearest, Magnification = Filter::MipmapLinear;
			WrapMode WrapS = WrapMode::Repeat; WrapMode WrapT = WrapMode::Repeat;
			bool GenerateMipmaps = true;
		};

	public:
		virtual ~Texture() {}
		static Ref<Texture> LoadTextureFromFile(const std::filesystem::path& path);
		static Ref<Texture> Create(const TextureData& textureData);

		virtual U32 GetId() const = 0;

		virtual void Bind(U32 slot = 0) = 0;
		virtual void UpdateData(void* data, PixelFormat format = PixelFormat::RGBA) = 0;
		virtual void UpdateData(U32 width, U32 height, void* data, PixelFormat format = PixelFormat::RGBA) = 0;
		virtual PixelData ReadPixel(U32 x, U32 y, RendererAPI::DataType dataType) = 0;

		virtual void SetMinificationFilter(Filter filter) = 0;
		virtual void SetMagnificationFilter(Filter filter) = 0;
		virtual void SetWrapSMode(WrapMode mode) = 0;
		virtual void SetWrapTMode(WrapMode mode) = 0;

		virtual const TextureData& GetData() const = 0;

		virtual Ref<Texture> GetSubTexture(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize = glm::uvec2(1)) = 0;
		static  Ref<Texture> GetSubTexture(Texture& texture, const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize = glm::uvec2(1));
		virtual Ref<Texture> GetSubTexturePixels(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx) = 0;
		static  Ref<Texture> GetSubTexturePixels(Texture& texture, const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx);

		virtual std::array<glm::vec2, 4> GetSubTextureUV(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize = glm::uvec2(1)) = 0;
		virtual std::array<glm::vec2, 4> GetSubTexturePixelsUV(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx) = 0;
	};

}
