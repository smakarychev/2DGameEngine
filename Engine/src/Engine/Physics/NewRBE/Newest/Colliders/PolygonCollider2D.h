#pragma once

#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    class PolygonCollider2D : public Collider2D
    {
        FRIEND_MEMORY_FN
    public:
        struct Box2D
        {
            glm::vec2 Center{0.0f};
            glm::vec2 HalfSize{0.5f};
        };
    public:
        PolygonCollider2D();
        PolygonCollider2D(const std::vector<glm::vec2>& vertices);
        Box2D SetAsBox(const glm::vec2& halfSize, const glm::vec2& center = glm::vec2{0.0f, 0.0f}, const Rotation& rotation = 0.0f);
        Box2D GetAsBox();
        DefaultBounds2D GenerateBounds(const Transform2D& transform) const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
        F32 GetRadius() const { return m_Radius; }
        const std::vector<glm::vec2>& GetVertices() const { return m_Vertices; }
        const std::vector<glm::vec2>& GetNormals() const { return m_Normals; }
    private:
        PolygonCollider2D(const std::vector<glm::vec2>& vertices, const std::vector<glm::vec2>& normals, const glm::vec2& center);
        void BuildPolygonFromPoints(const std::vector<glm::vec2>& vertices);
        void SetAsErrorBox();
        glm::vec2 ComputeCentroid();
    private:
        std::vector<glm::vec2> m_Vertices;
        std::vector<glm::vec2> m_Normals;
        glm::vec2 m_Center{};
        static constexpr F32 m_Radius = 2.0f * 0.005f;
    };
}
