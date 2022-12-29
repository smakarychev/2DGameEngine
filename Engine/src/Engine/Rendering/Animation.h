#pragma once

#include "Texture.h"

namespace Engine
{
    class SpriteAnimation
    {
        // TODO: possibly temp.
        friend class SpriteAnimationSerializer;
    public:
        SpriteAnimation(Texture* sprites, const glm::uvec2& startPoint, const glm::uvec2& spriteSize, U32 frameCount,
                        U32 fpsSpeed, F32 maxDuration = 0);
        void Update(F32 dt);
        const std::array<glm::vec2, 4>& GetCurrentFrameUV() const { return m_CurrentFrameUV; }
        bool HasEnded() const { return m_MaxDuration != 0.0f && m_TotalDuration >= m_MaxDuration; }
    private:
        Texture* m_SpriteSheet = nullptr;
        glm::uvec2 m_StartPosition = glm::uvec2{0, 0};
        glm::uvec2 m_SpriteSize = glm::uvec2{16, 16};
        // Number of frames (sprites) in animation.
        U32 m_FrameCount = 1;
        U32 m_CurrentFrame = 0;
        F32 m_FrameDuration = 1.0f;
        F32 m_TotalDuration = 0.0f;
        F32 m_TimeAccumulator = 0.0f;
        F32 m_MaxDuration   = 0.0f;
        U32 m_FpsSpeed = 0;
        std::array<glm::vec2, 4> m_CurrentFrameUV{};
    };
}
