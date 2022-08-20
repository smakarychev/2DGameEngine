#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    using namespace Types;
    class Holder
    {
    public:
        static Ref<Texture> tex;
    };
    
    class Font
    {
    public:
        struct CharacterInfo
        {
            std::vector<glm::vec2> UV = std::vector<glm::vec2>(4, glm::vec2{});
            glm::vec2 Size;
            glm::vec2 Bearing;
            F32 Advance;
        };
    public:
        static Ref<Font> ReadFontFromFile(const std::filesystem::path& path);
        Font(const std::string name, Ref<Texture> atlas);

        Texture& GetAtlas() const { return *m_Atlas; }
        std::vector<CharacterInfo>& GetCharacters() { return m_Characters; }
        void SetCharacters(const std::vector<CharacterInfo>& chars) { m_Characters = chars; }

    private:
        std::vector<CharacterInfo> m_Characters;
        std::string m_FontName = "Default";
        Ref<Texture> m_Atlas;
    };
}