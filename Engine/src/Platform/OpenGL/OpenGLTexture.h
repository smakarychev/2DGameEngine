#pragma once

#include "Engine/Rendering/Texture.h"

namespace Engine
{
	U32 GetOpenGLPixelFormat(Texture::PixelFormat format);

	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture(const TextureData& textureData);
		~OpenGLTexture();

		U32 GetId() const override { return m_Id; }

		void Bind(U32 slot) override;
		void UpdateData(void* data, PixelFormat format) override;
		void UpdateData(U32 width, U32 height, void* data, PixelFormat format) override;

	private:
		U32 m_Id;
		TextureData m_Data;
	};
}
