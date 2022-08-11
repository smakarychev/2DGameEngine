#pragma once

#include <filesystem>

#include "Engine/Core/Types.h"

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
	};

}
