#pragma once

#include <glm/glm.hpp>

#include "Engine/Common/Geometry2D.h"
#include "Engine/Core/Types.h"

namespace Engine
{
    namespace Component
    {
        struct LocalToParentTransform2D;
        struct LocalToWorldTransform2D;
    }
}

namespace Engine::WIP::Physics::Newest
{
    using namespace Types;

    struct Transform2D
    {
        glm::vec2 Position{};
        Rotation Rotation{};

        Transform2D() = default;
        Transform2D(Component::LocalToWorldTransform2D& ltwt);
        Transform2D(Component::LocalToParentTransform2D& ltpt);
        
        glm::vec2 Transform(const glm::vec2& point) const;
        glm::vec2 TransformDirection(const glm::vec2& dir) const;
        glm::vec2 InverseTransform(const glm::vec2& point) const;
        glm::vec2 InverseTransformDirection(const glm::vec2& dir) const;
    };
    
    
}
