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
        void OnUpdate();
        void UpdateGraphOfEntity(Entity entity);
        void ReflectEntityTransformToPrefabTransform();
    private:
        void UpdateTransforms(const std::vector<Entity>& topLevelEntities);
        std::vector<Entity> FindTopLevelEntities();
        void MarkHierarchyOf(Entity entity, std::unordered_map<Entity, bool>& traversalMap);

        
    private:
        std::vector<std::vector<EntityWorldTransformInfo>> m_TransformHierarchy{};

        std::unordered_map<Entity, bool> m_DrawTraversalMap;
        
        Registry& m_Registry;
    };
}
