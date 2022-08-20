#include "enginepch.h"

#include "Font.h"

#include "Engine/Resource/ResourceManager.h"

namespace Engine
{
    Ref<Font> Font::ReadFontFromFile(const std::filesystem::path& path)
    {
        return FontLoader::LoadFontFromFile(path);
    }

    Font::Font(const std::string name, Ref<Texture> atlas) :
        m_FontName(name), m_Atlas(atlas)
    { }

}