#include "enginepch.h"
#include "Animation.h"

namespace Engine
{
	SpriteAnimation::SpriteAnimation(Texture* sprites, const glm::uvec2& startPoint, const glm::uvec2& spriteSize, U32 frameCount, U32 fpsSpeed, F32 maxDuration)
		: m_SpriteSheet(sprites), m_StartPosition(startPoint), m_SpriteSize(spriteSize), m_FrameCount(frameCount),
		  m_FrameDuration(fpsSpeed != 0.0f ? 1.0f / static_cast<F32>(fpsSpeed) : 1.0f), m_MaxDuration(maxDuration)
	{
		std::array<glm::vec2, 2> uvRect{};
		uvRect[0] = { m_StartPosition.x + m_SpriteSize.x * m_CurrentFrame, m_StartPosition.y };
		uvRect[1] = { m_StartPosition.x + m_SpriteSize.x * (m_CurrentFrame + 1), m_StartPosition.y + m_SpriteSize.y };
		uvRect[0] /= glm::vec2{ m_SpriteSheet->GetData().Width, m_SpriteSheet->GetData().Height };
		uvRect[1] /= glm::vec2{ m_SpriteSheet->GetData().Width, m_SpriteSheet->GetData().Height };
		m_CurrentFrameUV = {
			glm::vec2{ uvRect[0].x, uvRect[0].y },
			glm::vec2{ uvRect[1].x, uvRect[0].y },
			glm::vec2{ uvRect[1].x, uvRect[1].y },
			glm::vec2{ uvRect[0].x, uvRect[1].y }
		};
	}
	
	void SpriteAnimation::Update(F32 dt)
	{
		// If animation is not looped.
		m_TotalDuration += dt;
		m_TimeAccumulator += dt;
		// If frame is displayed long enough.
		if (m_TimeAccumulator < m_FrameDuration) return;
		while (m_TimeAccumulator > m_FrameDuration)
		{
			m_CurrentFrame++;
			m_TimeAccumulator -= m_FrameDuration;
		}
		m_CurrentFrame = m_CurrentFrame % m_FrameCount;
		std::array<glm::vec2, 2> uvRect{};
		uvRect[0] = { m_StartPosition.x + m_SpriteSize.x * m_CurrentFrame, m_StartPosition.y };
		uvRect[1] = { m_StartPosition.x + m_SpriteSize.x * (m_CurrentFrame + 1), m_StartPosition.y + m_SpriteSize.y };
		uvRect[0] /= glm::vec2{ m_SpriteSheet->GetData().Width, m_SpriteSheet->GetData().Height };
		uvRect[1] /= glm::vec2{ m_SpriteSheet->GetData().Width, m_SpriteSheet->GetData().Height };
		m_CurrentFrameUV = {
			glm::vec2{ uvRect[0].x, uvRect[0].y },
			glm::vec2{ uvRect[1].x, uvRect[0].y },
			glm::vec2{ uvRect[1].x, uvRect[1].y },
			glm::vec2{ uvRect[0].x, uvRect[1].y }
		};
	}
}


