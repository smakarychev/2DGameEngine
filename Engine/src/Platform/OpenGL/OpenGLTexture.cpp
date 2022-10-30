#include "enginepch.h"

#include "OpenGLTexture.h"

#include "Engine/Core/Log.h"

#include "Engine/Memory/MemoryManager.h"
#include "Engine/Rendering/RendererAPI.h"

#include <glad/glad.h>

namespace Engine
{
	
	OpenGLTexture::OpenGLTexture(const TextureData& textureData) : m_Data(textureData)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_Id);

		SetWrapSMode(textureData.WrapS);
		SetWrapTMode(textureData.WrapT);
		SetMinificationFilter(textureData.Minification);
		SetMagnificationFilter(textureData.Magnification);

		auto&& [internalFormat, dataFormat] = GetInternalFormatFormatPair(textureData.PixelFormat);

		glTextureStorage2D(m_Id, 1, internalFormat, (GLsizei)textureData.Width, (GLsizei)textureData.Height);
		glTextureSubImage2D(m_Id, 0, 0, 0, (GLsizei)textureData.Width, (GLsizei)textureData.Height, dataFormat, GL_UNSIGNED_BYTE, textureData.Data);

		if (textureData.GenerateMipmaps)
		{
			glGenerateTextureMipmap(m_Id);
		}
	}


	void OpenGLTexture::Bind(U32 slot)
	{
		glBindTextureUnit(slot, m_Id);
	}

	void OpenGLTexture::UpdateData(void* data, PixelFormat format)
	{
		glTextureSubImage2D(m_Id, 0, 0, 0, m_Data.Width, m_Data.Height, GetFormat(format), GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture::UpdateData(U32 width, U32 height, void* data, PixelFormat format)
	{
		m_Data.Width = width;
		m_Data.Height = height;
		glTextureSubImage2D(m_Id, 0, 0, 0, width, height, GetFormat(format), GL_UNSIGNED_BYTE, data);
	}

	PixelData OpenGLTexture::ReadPixel(U32 x, U32 y, RendererAPI::DataType dataType)
	{
		PixelData pixelData;
		glReadPixels(GLint(x), GLint(y), 1, 1, GetFormat(m_Data.PixelFormat), RendererAPI::GetNativeDataType(dataType), &pixelData.Data);
		return pixelData;
	}

	Ref<Texture> OpenGLTexture::GetSubTexture(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize)
	{
		return GetSubTexturePixels(tileSize * subtexCoords, tileSize * subtexSize);
	}

	Ref<Texture> OpenGLTexture::GetSubTexturePixels(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx)
	{
		TextureData data;
		data.PixelFormat = m_Data.PixelFormat;
		data.Name = m_Data.Name + std::string("sub");
		data.Data = nullptr;
		data.Width = subtexSizePx.x;
		data.Height = subtexSizePx.y;
		Ref<Texture> subTexture = Texture::Create(data);

		glCopyImageSubData(
			m_Id, GL_TEXTURE_2D, 0, GLint(subtexCoordsPx.x), GLint(subtexCoordsPx.y), 0,
			subTexture->GetId(), GL_TEXTURE_2D, 0, 0, 0, 0,
			GLsizei(subtexSizePx.x), GLsizei(subtexSizePx.y), 1
		);
		return subTexture;
	}

	std::array<glm::vec2, 4> OpenGLTexture::GetSubTextureUV(const glm::uvec2& tileSize, const glm::uvec2& subtexCoords, const glm::uvec2& subtexSize)
	{
		// Normalize tilesize to get uv.
		glm::vec2 normalizedTile = glm::vec2{ (F32)tileSize.x / m_Data.Width, (F32)tileSize.y / m_Data.Height };
		glm::vec2 begin = glm::vec2{ subtexCoords.x * normalizedTile.x, subtexCoords.y * normalizedTile.y };
		glm::vec2 end = begin + glm::vec2{ subtexSize.x * normalizedTile.x, subtexSize.y * normalizedTile.y };
		return { {
			{ begin.x,	begin.y },
			{ end.x,	begin.y },
			{ end.x,	end.y	},
			{ begin.x,	end.y	}
		} };
	}

	std::array<glm::vec2, 4> OpenGLTexture::GetSubTexturePixelsUV(const glm::uvec2& subtexCoordsPx, const glm::uvec2& subtexSizePx)
	{
		glm::vec2 begin = subtexCoordsPx;
		glm::vec2 end = begin + glm::vec2{ subtexSizePx };
		// Normalize to get uv.
		begin /= glm::vec2{ m_Data.Width, m_Data.Height };
		end   /= glm::vec2{ m_Data.Width, m_Data.Height };
		return { {
			{ begin.x,	begin.y },
			{ end.x,	begin.y },
			{ end.x,	end.y	},
			{ begin.x,	end.y	}
		} };
	}

	void OpenGLTexture::SetMagnificationFilter(Filter filter)
	{
		switch (filter)
		{
		case Engine::Texture::Filter::Nearest:			glTextureParameteri(m_Id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case Engine::Texture::Filter::Linear:			glTextureParameteri(m_Id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Engine::Texture::Filter::MipmapNearest:	glTextureParameteri(m_Id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;
		case Engine::Texture::Filter::MipmapLinear:		glTextureParameteri(m_Id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;
		}
	}

	void OpenGLTexture::SetMinificationFilter(Filter filter)
	{
		switch (filter)
		{
		case Engine::Texture::Filter::Nearest:			glTextureParameteri(m_Id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			break;																			 
		case Engine::Texture::Filter::Linear:			glTextureParameteri(m_Id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			break;																			 
		case Engine::Texture::Filter::MipmapNearest:	glTextureParameteri(m_Id, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;																			 
		case Engine::Texture::Filter::MipmapLinear:		glTextureParameteri(m_Id, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;																			 
		}
	}

	void OpenGLTexture::SetWrapSMode(WrapMode mode)
	{
		switch (mode)
		{
		case Engine::Texture::WrapMode::None:
			break;
		case Engine::Texture::WrapMode::Clamp:			glTextureParameteri(m_Id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			break;
		case Engine::Texture::WrapMode::Repeat:			glTextureParameteri(m_Id, GL_TEXTURE_WRAP_S, GL_REPEAT);
			break;
		}
	}
	void OpenGLTexture::SetWrapTMode(WrapMode mode)
	{
		switch (mode)
		{
		case Engine::Texture::WrapMode::None:
			break;
		case Engine::Texture::WrapMode::Clamp:			glTextureParameteri(m_Id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		case Engine::Texture::WrapMode::Repeat:			glTextureParameteri(m_Id, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		}
	}

	std::pair<GLenum, GLenum> OpenGLTexture::GetInternalFormatFormatPair(Texture::PixelFormat format)
	{
		switch (format)
		{
		case Engine::Texture::PixelFormat::RedInteger:		return std::make_pair(GL_R32I, GetFormat(format));
		case Engine::Texture::PixelFormat::RGB:				return std::make_pair(GL_RGB8, GetFormat(format));
		case Engine::Texture::PixelFormat::RGBA:			return std::make_pair(GL_RGBA8, GetFormat(format));
		case Engine::Texture::PixelFormat::DepthStencil:	return std::make_pair(GL_DEPTH32F_STENCIL8, GetFormat(format));
		case Engine::Texture::PixelFormat::None:
			ENGINE_CORE_FATAL("Pixel format for texture is unset!");
			break;
		default:
			ENGINE_CORE_FATAL("Pixel format in unimplemented!");
			break;
		}
		return std::pair<GLenum, GLenum>();
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &m_Id);
	}

	U32 OpenGLTexture::GetFormat(Texture::PixelFormat format)
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
