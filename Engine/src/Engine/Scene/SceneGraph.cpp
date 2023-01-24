#include "enginepch.h"

#include "SceneGraph.h"

#include "SceneUtils.h"
#include "Engine/ECS/View.h"
#include "Serialization/Prefab.h"

namespace Engine
{
    SceneGraph::SceneGraph(Registry& m_Registry)
        : m_Registry(m_Registry)
    {
        m_TransformHierarchy.resize(128);
    }

    void SceneGraph::OnUpdate()
    {
        const std::vector<Entity>& topLevelEntities = FindTopLevelEntities();
        UpdateTransforms(topLevelEntities);
    }

    void SceneGraph::UpdateGraphOfEntity(Entity entity)
    {
        UpdateTransforms({entity});
    }

    void SceneGraph::UpdateTransforms(const std::vector<Entity>& topLevelEntities)
    {
        for (auto& tlayer : m_TransformHierarchy) tlayer.clear();

        // First add the top level entities, since they do not depend on any other entities.
        for (auto e : topLevelEntities)
        {
            m_TransformHierarchy[0].emplace_back(-1, e, m_Registry.Get<Component::LocalToWorldTransform2D>(e));
        }
        // Then iterate all the child entities, and set `ParentIndex`,
        // to later combine child's transform with parent's transform.
        for (U32 layer = 0; layer < 256; layer++)
        {
            if (m_TransformHierarchy[layer].empty()) break;
            for (I32 parentI = 0; parentI < static_cast<I32>(m_TransformHierarchy[layer].size()); parentI++)
            {
                auto& record = m_TransformHierarchy[layer][parentI];
                if (!m_Registry.Has<Component::ChildRel>(record.EntityId)) continue;
                SceneUtils::TraverseChildren(record.EntityId, m_Registry, [&](Entity e)
                {
                    auto& childLocalToParent = m_Registry.Get<Component::LocalToParentTransform2D>(e);
                    m_TransformHierarchy[layer + 1].emplace_back(parentI, e, childLocalToParent);
                });
            }
        }

        // Apply parent transforms to it's child.
        for (U32 i = 1; i < m_TransformHierarchy.size(); i++)
        {
            auto& tLayer = m_TransformHierarchy[i];
            if (tLayer.empty()) break;
            for (auto& childTf : tLayer)
            {
                auto& parentLocalToWorld = m_TransformHierarchy[i - 1][childTf.ParentIndex];
                childTf.WorldTransform = childTf.WorldTransform.Concatenate(parentLocalToWorld.WorldTransform);
            }
        }
        for (U32 i = 1; i < m_TransformHierarchy.size(); i++)
        {
            auto& tLayer = m_TransformHierarchy[i];
            if (tLayer.empty()) break;
            for (auto& childTf : tLayer)
            {
                auto& localToWorld = m_Registry.Get<Component::LocalToWorldTransform2D>(childTf.EntityId);
                localToWorld = childTf.WorldTransform;
            }
        }
    }

    std::vector<Entity> SceneGraph::FindTopLevelEntities()
    {
        std::vector<Entity> result;
        std::unordered_map<Entity, bool> traversal;
        for (auto e : View<Component::ChildRel>(m_Registry))
        {
            if (traversal[e]) continue;
            // Mark all of it's children.
            MarkHierarchyOf(e, traversal);

            if (!m_Registry.Has<Component::ParentRel>(e))
            {
                result.push_back(e);
                continue;
            }

            Entity topOfTree = SceneUtils::FindTopOfTree(e, m_Registry);
            MarkHierarchyOf(topOfTree, traversal);
            result.push_back(topOfTree);
        }
        return result;
    }

    void SceneGraph::MarkHierarchyOf(Entity entity, std::unordered_map<Entity, bool>& traversalMap)
    {
        if (traversalMap[entity]) return;
        traversalMap[entity] = true;
        if (m_Registry.Has<Component::ChildRel>(entity))
        {
            SceneUtils::TraverseChildren(entity, m_Registry, [&](Entity e)
            {
               MarkHierarchyOf(e, traversalMap); 
            });
        }
    }

    void SceneGraph::ReflectEntityTransformToPrefabTransform()
    {
        for (auto prefab : View<Component::Prefab>(m_Registry))
        {
            bool hasParent = m_Registry.Has<Component::ParentRel>(prefab);
            auto& eTf = m_Registry.Get<Component::LocalToWorldTransform2D>(m_Registry.Get<Component::ChildRel>(prefab).First);
            auto& eTfLocal = m_Registry.Get<Component::LocalToParentTransform2D>(m_Registry.Get<Component::ChildRel>(prefab).First);

            if (hasParent)
            {
                m_Registry.Get<Component::LocalToParentTransform2D>(prefab).Position = eTfLocal.Position;
            }
            else
            {
                m_Registry.Get<Component::LocalToWorldTransform2D>(prefab).Position = eTf.Position;
            }
            eTfLocal.Position = glm::vec2{0.0f, 0.0f};
        }
    }
}
