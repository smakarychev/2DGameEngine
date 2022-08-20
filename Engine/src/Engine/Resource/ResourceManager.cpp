#include "enginepch.h"

#include "ResourceManager.h"
#include "Engine/Memory/MemoryManager.h"

#include "Engine/Core/Log.h"

#include <stb_image/stb_image.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace Engine
{
	std::string ResourceManager::ReadFile(const std::filesystem::path& path)
	{
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (in)
		{
			return(std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
		}
		ENGINE_CORE_ERROR("ResourceManager: error reading file {}", path.string());
		return "";
	}

	void ResourceManager::ShutDown()
	{
		ShaderLoader::ShutDown();
		TextureLoader::ShutDown();
		FontLoader::ShutDown();
	}

	std::unordered_map<std::string, Ref<Shader>> ShaderLoader::s_LoadedShaders;

	std::shared_ptr<Shader> ShaderLoader::LoadShaderFromFile(const std::filesystem::path& path)
	{
		std::string pathString = path.string();
		auto it = s_LoadedShaders.find(pathString);
		if (it != s_LoadedShaders.end()) return it->second;

		std::string shaderFileContent = ResourceManager::ReadFile(path);

		std::string shaderName = path.filename().string();

		Ref<Shader> shader = Shader::CreateShaderFromSource(shaderName, shaderFileContent);
		s_LoadedShaders.emplace(pathString, shader);
		return shader;
	}
	void ShaderLoader::ShutDown()
	{
		s_LoadedShaders.clear();
	}

	std::unordered_map<std::string, Ref<Texture>> TextureLoader::s_LoadedTextures;

	std::shared_ptr<Texture> TextureLoader::LoadTextureFromFile(const std::filesystem::path& path)
	{
		// TODO: maybe move in somewhere else (it is one assignment).
		stbi_set_flip_vertically_on_load(true);

		std::string pathString = path.string();
		auto it = s_LoadedTextures.find(pathString);
		if (it != s_LoadedTextures.end()) return it->second;

		std::string textureName = path.filename().string();

		Texture::TextureData textureData;
		I32 width, height, channels;
		
		textureData.Data = stbi_load(pathString.c_str(), &width, &height, &channels, 0);
		if (textureData.Data == nullptr) ENGINE_CORE_ERROR("Failed to load texture: {}", pathString);
		textureData.Width = U32(width);
		textureData.Height = U32(height);
		textureData.Channels = U32(channels);
		textureData.Name = textureName;

		Ref<Texture> texture = Texture::Create(textureData);
		stbi_image_free(textureData.Data);

		s_LoadedTextures.emplace(pathString, texture);
		return texture;
	}
	
	void TextureLoader::ShutDown()
	{
		s_LoadedTextures.clear();
	}

	std::unordered_map<std::string, Ref<Font>> FontLoader::s_LoadedFonts;

	void ExtractCharacters(Font& font, const std::vector<msdf_atlas::GlyphGeometry>& glyphs);

	Ref<Font> FontLoader::LoadFontFromFile(const std::filesystem::path& path)
	{
		using namespace msdf_atlas;
		std::string pathString = path.string();
		auto it = s_LoadedFonts.find(pathString);
		if (it != s_LoadedFonts.end()) return it->second;

		std::string fontName = path.filename().string();

		if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) 
		{
			if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, pathString.c_str())) 
			{		
				std::vector<GlyphGeometry> glyphs;
				FontGeometry fontGeometry(&glyphs);
				fontGeometry.loadCharset(font, 2.0, Charset::ASCII);

				const F64 maxCornerAngle = 3.0;
				for (GlyphGeometry& glyph : glyphs)
					glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

				TightAtlasPacker packer;
				packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::POWER_OF_TWO_RECTANGLE);
				packer.setScale(48.0);
				packer.setPixelRange(2.0);
				packer.setMiterLimit(1.0);
				packer.pack(glyphs.data(), glyphs.size());
				I32 width = 0, height = 0;
				packer.getDimensions(width, height);
				ImmediateAtlasGenerator<F32, 3, msdfGenerator, BitmapAtlasStorage<byte, 3>> generator(width, height);
				GeneratorAttributes attributes;
				generator.setAttributes(attributes);
				generator.setThreadCount(4);
				generator.generate(glyphs.data(), glyphs.size());
				
				msdfgen::BitmapConstRef<msdfgen::byte, 3> bitmap = generator.atlasStorage();
				Texture::TextureData data;
				data.Channels = 3;
				data.Width = bitmap.width; data.Height = bitmap.height;
				data.Data = const_cast<U8*>(bitmap.pixels);
				Ref<Texture> fontAtlas = Texture::Create(data);
				fontAtlas->SetMinificationFilter(Texture::Filter::Linear);
				fontAtlas->SetMagnificationFilter(Texture::Filter::Linear);

				Ref<Font> newFont = CreateRef<Font>(fontName, fontAtlas);
				ExtractCharacters(*newFont, glyphs);
				s_LoadedFonts.emplace(pathString, newFont);

				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}

		return s_LoadedFonts[pathString];
	}
	
	// TODO: make a method of FontLoader?
	void ExtractCharacters(Font& font, const std::vector<msdf_atlas::GlyphGeometry>& glyphs)
	{
		std::vector<Font::CharacterInfo> chars(glyphs.back().getCodepoint() + 1);
		U32 atlasWidth = font.GetAtlas().GetData().Width;
		U32 atlasHeight = font.GetAtlas().GetData().Height;
		for (auto& glyph : glyphs)
		{
			F64 l, b, r, t;
			I32 x, y, w, h;
			
			glyph.getQuadAtlasBounds(l, b, r, t);
			glm::vec2 bl { F32(l) / atlasWidth, F32(b) / atlasHeight };
			glm::vec2 tr { F32(r) / atlasWidth, F32(t) / atlasHeight };

			glyph.getQuadPlaneBounds(l, b, r, t);
			Font::CharacterInfo character
			{
				{ { {bl.x, bl.y }, {tr.x, bl.y}, {tr.x, tr.y}, {bl.x, tr.y} } },
				glm::vec2{r - l, t - b },
				glm::vec2{l / 2.0f + r / 2.0f, b / 2.0f + t / 2.0f},
				glyph.getAdvance()
			};
			U32 codePoint = glyph.getCodepoint();
			chars[codePoint] = character;
		}
		font.SetCharacters(chars);
	}

	void FontLoader::ShutDown()
	{
		s_LoadedFonts.clear();
	}
}