#include "enginepch.h"
#include "Polygon.h"

Engine::Polygon::Polygon(const std::vector<glm::vec2>& convexHull, bool genUV)
    : m_Vertices(convexHull)
{
    ENGINE_CORE_ASSERT(convexHull.size() >= 3, "Polygon is degenerate.")

    // Form indices.
    m_Indices.reserve((convexHull.size() - 2) * 3);
    for (U32 i = 1; i < convexHull.size() - 1; i++)
    {
        m_Indices.push_back(0);
        m_Indices.push_back(i + 1);
        m_Indices.push_back(i);
    }

    if (genUV)
    {
        m_UV.reserve(m_Vertices.size());
        F32 radius = 0.0f;
        for (auto& vertex : convexHull)
        {
            F32 curRad = glm::max(glm::abs(vertex.x), glm::abs(vertex.y));
            if (curRad > radius) radius = curRad;
        }
        for (auto& vertex : convexHull)
        {
            glm::vec2 distanceVec = vertex;
            distanceVec /= radius;
            glm::vec2 uv = (distanceVec + 1.0f) * 0.5f;
            m_UV.push_back(uv);
        }
    }
}
