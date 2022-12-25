#include "enginepch.h"

#include "SceneGraph.h"

#include "Engine/ECS/View.h"

namespace Engine
{
    SceneGraph::SceneGraph(Registry& registry)
        : m_Registry(registry)
    {
        m_TransformHierarchy.resize(128);
    }

    void SceneGraph::UpdateTransforms()
    {
        for (auto& tlayer : m_TransformHierarchy) tlayer.clear();

        // Set initial transforms.
        for (auto e : View<Component::ChildRel>(m_Registry))
        {
            auto& localToWorld = m_Registry.Get<Component::LocalToWorldTransform2D>(e);
            U8 layer = 0;
            I32 parentIndex = -1;
            if (m_Registry.Has<Component::ParentRel>(e))
            {
                auto& parentRel = m_Registry.Get<Component::ParentRel>(e);
                ENGINE_CORE_ASSERT(parentRel.Depth < 127, "Hierarchy is too deep.")
                layer = static_cast<U8>(parentRel.Depth);
                parentIndex = static_cast<I32>(parentRel.Parent);
            }
            m_TransformHierarchy[layer].emplace_back(parentIndex, e, localToWorld);

            auto& childRel = m_Registry.Get<Component::ChildRel>(e);
            Entity child = childRel.First;
            for (U32 childI = 0; childI < childRel.ChildrenCount; childI++)
            {
                auto& childLocalToParent = m_Registry.Get<Component::LocalToParentTransform2D>(child);
                m_TransformHierarchy[layer + 1].emplace_back(m_TransformHierarchy[layer].size() - 1, child, childLocalToParent);
                child = m_Registry.Get<Component::ParentRel>(child).Next;
            }
        }
        // Apply parent transforms to it's child.
        for (U32 i = 1; i < m_TransformHierarchy.size(); i++)
        {
            auto& tLayer = m_TransformHierarchy[i];
            if (tLayer.size() == 0) break;
            for (auto& childTf : tLayer)
            {
                auto& parentLocalToWorld = m_TransformHierarchy[i - 1][childTf.ParentIndex];
                childTf.WorldTransform = childTf.WorldTransform.Concatenate(parentLocalToWorld.WorldTransform);
            }
        }
        for (U32 i = 1; i < m_TransformHierarchy.size(); i++)
        {
            auto& tLayer = m_TransformHierarchy[i];
            if (tLayer.size() == 0) break;
            for (auto& childTf : tLayer)
            {
                auto& localToWorld = m_Registry.Get<Component::LocalToWorldTransform2D>(childTf.EntityId);
                localToWorld = childTf.WorldTransform;
            }
        }
    }
}

