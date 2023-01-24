#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Rendering/Buffer.h"
#include "Engine/Primitives/Shape.h"

#include <glm/glm.hpp>
namespace Engine
{
    using namespace Types;
    class Polygon
    {
    public:
        Polygon() = default;
        Polygon(const std::vector<glm::vec2>& convexHull, bool genUV = true);
        
    public:
        const std::vector<glm::vec2>& GetVertices() const { return m_Vertices; }
        const std::vector<glm::vec2>& GetUVs() const { return m_UV; }
        const std::vector<U32>& GetIndices() const { return m_Indices; }
        U32 GetNumberOfVertices() const { return static_cast<U32>(m_Vertices.size()); }
        U32 GetNumberOfIndices() const { return static_cast<U32>(m_Indices.size());}
        UShapePrimitive GetPrimitiveType() const { return m_PrimitiveType; }
    protected:
        std::vector<glm::vec2> m_Vertices;
        std::vector<glm::vec2> m_UV;
        std::vector<U32> m_Indices;
        UShapePrimitive m_PrimitiveType = UShapePrimitive::Triangles;
    };
}
