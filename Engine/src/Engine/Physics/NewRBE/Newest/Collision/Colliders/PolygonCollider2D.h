#pragma once

#include "Engine/Core/Core.h"
#include "Collider2D.h"

namespace Engine::WIP::Physics::Newest
{
    struct PolygonColliderDesc2D : ColliderDesc2D
    {
        // CookedFull when vertices, normals and center is provided.
        enum class DataType { Empty, Raw, CookedVertices, CookedVerticesNormals, CookedFull };
        PolygonColliderDesc2D() : ColliderDesc2D(Collider2DType::Polygon) {}
        DataType DataType{DataType::Empty};
        std::vector<glm::vec2> Vertices{};
        std::vector<glm::vec2> Normals{};
        glm::vec2 Center{};
    };
    
    class PolygonCollider2D : public Collider2D
    {
        friend class PhysicsFactory;
    public:
        struct Box2D
        {
            glm::vec2 Center{0.0f};
            glm::vec2 HalfSize{0.5f};
        };
    public:
        PolygonCollider2D(const PolygonColliderDesc2D& colDesc);
        Box2D SetAsBox(const glm::vec2& halfSize, const glm::vec2& center = glm::vec2{0.0f, 0.0f}, const Rotation& rotation = 0.0f);
        Box2D GetAsBox() const;
        AABB2D GenerateBounds(const Transform2D& transform) const override;
        AABB2D GenerateLocalBounds() const override;
        MassInfo2D CalculateMass() const override;
        glm::vec2 GetCenterOfMass() const override;
        F32 GetRadius() const { return m_Radius; }
        const std::vector<glm::vec2>& GetVertices() const { return m_Vertices; }
        const std::vector<glm::vec2>& GetNormals() const { return m_Normals; }
    private:
        void BuildPolygonFromPoints(const std::vector<glm::vec2>& vertices);
        void BuildNormals();
        void SetAsErrorBox();
        glm::vec2 ComputeCentroid() const;
    private:
        std::vector<glm::vec2> m_Vertices;
        std::vector<glm::vec2> m_Normals;
        glm::vec2 m_Center{};
        static constexpr F32 m_Radius = 2.0f * 0.005f;
    };
}
