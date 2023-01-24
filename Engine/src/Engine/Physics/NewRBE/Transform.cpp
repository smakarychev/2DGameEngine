#include "enginepch.h"

#include "Transform.h"

#include "Engine/ECS/Components.h"

namespace Engine::WIP::Physics
{
    Transform2D::Transform2D(Component::LocalToWorldTransform2D& ltwt)
        : Position(ltwt.Position), Rotation(ltwt.Rotation)
    {
    }

    Transform2D::Transform2D(Component::LocalToParentTransform2D& ltpt)
        : Position(ltpt.Position), Rotation(ltpt.Rotation)
    {
    }

    glm::vec2 Transform2D::Transform(const glm::vec2& point) const
    {
        return TransformPoint2D(point, Position, Rotation);
    }

    glm::vec2 Transform2D::TransformDirection(const glm::vec2& dir) const
    {
        return TransformDir2D(dir, Rotation);
    }

    glm::vec2 Transform2D::InverseTransform(const glm::vec2& point) const
    {
        return InverseTransformPoint2D(point, Position, Rotation);
    }

    glm::vec2 Transform2D::InverseTransformDirection(const glm::vec2& dir) const
    {
        return InverseTransformDir2D(dir, Rotation);
    }
}
