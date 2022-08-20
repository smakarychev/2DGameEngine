#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Rendering/Font.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

#include <filesystem>

namespace Engine
{
	using namespace Types;

	class ResourceManager
	{
	public:
		static std::string ReadFile(const std::filesystem::path& path);
		static void ShutDown();
	};

	class ShaderLoader
	{
	public:
		static Ref<Shader> LoadShaderFromFile(const std::filesystem::path& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, Ref<Shader>> s_LoadedShaders;
	};

	class TextureLoader
	{
	public:
		static Ref<Texture> LoadTextureFromFile(const std::filesystem::path& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, Ref<Texture>> s_LoadedTextures;
	};

	class FontLoader
	{
	public:
		static Ref<Font> LoadFontFromFile(const std::filesystem::path& path);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, Ref<Font>> s_LoadedFonts;
	};
}