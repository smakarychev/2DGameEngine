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

	std::shared_ptr<Texture> OpenGLTexture::GetSubTexture(const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize)
	{
		TextureData data;
		data.Channels = m_Data.Channels;
		data.Name = m_Data.Name + std::string("sub");
		data.Data = nullptr;
		data.Width = U32(tileSize.x  * subtexSize.x);
		data.Height = U32(tileSize.y * subtexSize.y);
		std::shared_ptr<Texture> subTexture = Texture::Create(data);

		glCopyImageSubData(
			m_Id, GL_TEXTURE_2D, 0, GLint(tileSize.x * subtexCoords.x), GLint(tileSize.y * subtexCoords.y), 0,
			subTexture->GetId(), GL_TEXTURE_2D, 0, 0, 0, 0,
			GLsizei(tileSize.x * subtexSize.x), GLsizei(tileSize.y * subtexSize.y), 1
		);
		return subTexture;
	}

	std::vector<glm::vec2> OpenGLTexture::GetSubTextureUV(const glm::vec2& tileSize, const glm::vec2& subtexCoords, const glm::vec2& subtexSize)
	{
		// Normalize tilesize to get uv.
		glm::vec2 normalizedTile = glm::vec2(tileSize.x / m_Data.Width, tileSize.y / m_Data.Height);
		glm::vec2 begin = glm::vec2(subtexCoords.x * normalizedTile.x, subtexCoords.y * normalizedTile.y);
		glm::vec2 end = begin + glm::vec2(subtexSize.x * normalizedTile.x, subtexSize.y * normalizedTile.y);
		return { {
			{ begin.x,	begin.y },
			{ end.x,	begin.y },
			{ end.x,	end.y	},
			{ begin.x,	end.y	}
		} };
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
