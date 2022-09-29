#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    using namespace Types;
    
    class Font
    {
    public:
        struct CharacterInfo
        {
            std::vector<glm::vec2> UV = std::vector<glm::vec2>(4, glm::vec2{});
            glm::vec2 Size = glm::vec2{ 0.0f };
            glm::vec2 Bearing = glm::vec2{0.0f};
            F32 Advance = 0.0f;
        };
    public:
        static Ref<Font> ReadFontFromFile(const std::filesystem::path& path);
        Font(const std::string name, Ref<Texture> atlas, F32 fontSize);

        Texture& GetAtlas() const { return *m_Atlas; }
        std::vector<CharacterInfo>& GetCharacters() { return m_Characters; }
        void SetCharacters(const std::vector<CharacterInfo>& chars) { m_Characters = chars; }
        void SetLineHeight(F32 lineHeight) { m_LineHeight = lineHeight; }
        F32 GetLineHeight() const { return m_LineHeight; }

        void SetGeometryScale(F32 scale) { m_GeometryScale = scale; }
        F32 GetGeometryScale() const { return m_GeometryScale; }

        F32 GetBaseFontSize() { return m_BaseFontSize; }

    private:
        std::vector<CharacterInfo> m_Characters;
        std::string m_FontName = "Default";
        Ref<Texture> m_Atlas;
        F32 m_LineHeight = 0.0f;
        F32 m_GeometryScale = 0.0f;
        F32 m_BaseFontSize = 0.0f;
    };
}
