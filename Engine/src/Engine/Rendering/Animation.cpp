#include "enginepch.h"
#include "Animation.h"

namespace Engine
{
	SpriteAnimation::SpriteAnimation(Texture* sprites, const glm::uvec2& startPoint, const glm::uvec2& spriteSize, U32 frameCount, U32 fpsSpeed)
		: m_SpriteSheet(sprites), m_StartPosition(startPoint), m_SpriteSize(spriteSize), m_FrameCount(frameCount), m_FrameDuration(1.0f / fpsSpeed)
	{
		std::array<glm::vec2, 2> uvRect;
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
		m_TotalDuration += dt;
		// If frame is displayed long enought.
		if (m_TotalDuration < m_FrameDuration) return;
		while (m_TotalDuration > m_FrameDuration)
		{
			m_CurrentFrame++;
			m_TotalDuration -= m_FrameDuration;
		}
		m_CurrentFrame = m_CurrentFrame % m_FrameCount;
		std::array<glm::vec2, 2> uvRect;
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


