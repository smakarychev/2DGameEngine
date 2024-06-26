#include "enginepch.h"

#include "Font.h"

#include "Engine/Resource/ResourceManager.h"

namespace Engine
{
    Ref<Font> Font::ReadFontFromFile(const std::filesystem::path& path, Texture::TextureData data)
    {
        return FontLoader::LoadFontFromFile(path, data);
    }

    Font::Font(const std::string name, Ref<Texture> atlas, F32 size) :
        m_FontName(name), m_Atlas(std::move(atlas)), m_BaseFontSize(size)
    { }

}