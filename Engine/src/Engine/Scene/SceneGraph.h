#pragma once
#include "Engine/ECS/Components.h"
#include "Engine/ECS/EntityId.h"

namespace Engine
{
    class SceneGraph
    {
    public:
        struct EntityWorldTransformInfo
        {
            I32 ParentIndex{-1};
            Entity EntityId;
            Component::LocalToWorldTransform2D WorldTransform;
        };
    public:
        SceneGraph(Registry& registry);
        void UpdateTransforms();
    private:
        std::vector<std::vector<EntityWorldTransformInfo>> m_TransformHierarchy{};
        Registry& m_Registry;
    };
}
