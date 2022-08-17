#include "enginepch.h"
#include "RegularPolygon.h"

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryManager.h"
#include <glm/gtc/constants.hpp>

namespace Engine
{
	RegularPolygon::RegularPolygon(U32 angles, bool genUV) : m_Angles(angles)
	{
		ENGINE_CORE_ASSERT(angles >= 3, "Regular polygon must have at least 3 angles.");

		// Generate points.
		m_Vertices.resize(angles);
		for (U32 i = 0; i < angles; i++)
		{
			F32 phase = 2.0f * glm::pi<float>() * i / angles;
			m_Vertices[i] = glm::vec2(glm::sin(phase), glm::cos(phase));
		}

		if (genUV)
		{
			// Generate UVs.
			m_UV.resize(m_Vertices.size());
			for (U32 i = 0; i < angles; i++)
			{
				F32 phase = 2.0f * glm::pi<float>() * i / angles;
				m_UV[i] = glm::vec2(1.0f + glm::sin(phase), 1.0f + glm::cos(phase)) / 2.0f;
			}
		}
		
		// Generate indices.
		m_Indices.reserve((angles - 2) * 3);
		for (U32 i = 1; i < angles - 1; i++)
		{
			m_Indices.push_back(0);
			m_Indices.push_back(i + 1);
			m_Indices.push_back(i);
		}
	}

	void RegularPolygon::GenerateUVs(const std::vector<glm::vec2> uv)
	{
		m_UV.resize(m_Vertices.size());
		for (U32 i = 0; i < m_Angles; i++)
		{
			m_UV[i] = (m_Vertices[i] * glm::vec2((uv[1].x - uv[0].x), (uv[2].y - uv[1].y)) +
				glm::vec2(uv[1].x + uv[0].x, uv[2].y + uv[1].y)) / 2.0f;
		}
	}
}

