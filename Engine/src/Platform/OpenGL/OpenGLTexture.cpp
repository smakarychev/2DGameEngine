#include "enginepch.h"

#include "OpenGLTexture.h"

#include "Engine/Core/Log.h"

#include <glad/glad.h>

namespace Engine
{
	
	OpenGLTexture::OpenGLTexture(const TextureData& textureData) : m_Data(textureData)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Id);

		glTextureParameteri(m_Id, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_Id, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_Id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_Id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLenum internalFormat = 0, dataFormat = 0;
		if (textureData.Channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (textureData.Channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		glTextureStorage2D(m_Id, 1, internalFormat, textureData.Width, textureData.Height);
		glTextureSubImage2D(m_Id, 0, 0, 0, textureData.Width, textureData.Height, dataFormat, GL_UNSIGNED_BYTE, textureData.Data);

		glGenerateTextureMipmap(m_Id);
	}


	void OpenGLTexture::Bind(U32 slot)
	{
		glBindTextureUnit(slot, m_Id);
	}

	void OpenGLTexture::UpdateData(void* data, PixelFormat format)
	{
		glTextureSubImage2D(m_Id, 0, 0, 0, m_Data.Width, m_Data.Height, GetOpenGLPixelFormat(format), GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture::UpdateData(U32 width, U32 height, void* data, PixelFormat format)
	{
		m_Data.Width = width;
		m_Data.Height = height;
		glTextureSubImage2D(m_Id, 0, 0, 0, width, height, GetOpenGLPixelFormat(format), GL_UNSIGNED_BYTE, data);
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &m_Id);
	}

	U32 GetOpenGLPixelFormat(Texture::PixelFormat format)
	{
		switch (format)
		{
		case Texture::PixelFormat::Red:			    return GL_RED;
		case Texture::PixelFormat::RedInteger:	    return GL_RED_INTEGER;
		case Texture::PixelFormat::RG:		        return GL_RG;
		case Texture::PixelFormat::RGInteger:	    return GL_RG_INTEGER;
		case Texture::PixelFormat::RGB:		        return GL_RGB;
		case Texture::PixelFormat::RGBInteger:	    return GL_RGB_INTEGER;
		case Texture::PixelFormat::RGBA:			return GL_RGBA;
		case Texture::PixelFormat::RGBAInteger:	    return GL_RGBA_INTEGER;
		case Texture::PixelFormat::Depth:		    return GL_DEPTH;
		case Texture::PixelFormat::DepthStencil:	return GL_DEPTH_STENCIL;
		case Texture::PixelFormat::Alpha:		    return GL_ALPHA;
		}
		ENGINE_CORE_ERROR("Unknown type of pixel format");
		return 0;
	}

}
