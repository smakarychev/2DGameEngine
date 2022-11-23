#pragma once

#include "Engine/Rendering/Texture.h"

namespace Engine
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const TextureData& textureData);
		~OpenGLTexture();

		U32 GetId() const override { return m_Id; }

		void Bind(U32 slot) override;
		void UpdateData(void* data, PixelFormat format) override;
		void UpdateData(U32 width, U32 height, void* data, PixelFormat format) override;
		PixelData ReadPixel(U32 x, U32 y, RendererAPI::DataType dataType) override;

		const TextureData& GetData() const override { return m_Data; }

		Ref<Texture> GetSubTexture(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize) override;
		Ref<Texture> GetSubTexturePixels(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx) override;
		std::array<glm::vec2, 4> GetSubTextureUV(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize) override;
		std::array<glm::vec2, 4> GetSubTexturePixelsUV(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx) override;

		void SetMinificationFilter(Filter filter) override;
		void SetMagnificationFilter(Filter filter) override;
		void SetWrapSMode(WrapMode mode) override;
		void SetWrapTMode(WrapMode mode) override;
		static U32 GetFormat(Texture::PixelFormat format);
		static U32 GetPixelDataType(Texture::PixelFormat format);
	private:
		// Returns internalformat - format pair.
		std::pair<U32, U32> GetInternalFormatFormatPair(Texture::PixelFormat format);
	private:
		U32 m_Id{};
		TextureData m_Data;
	};
}
