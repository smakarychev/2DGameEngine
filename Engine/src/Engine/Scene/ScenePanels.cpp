#include "enginepch.h"
#include "ScenePanels.h"

#include "Scene.h"
#include "SceneUtils.h"
#include "Engine/Core/Input.h"
#include "Engine/ECS/View.h"

namespace Engine
{
    ScenePanels::ScenePanels(Scene& scene)
        : m_Scene(scene)
    {
        m_ActiveEntity = NULL_ENTITY;
        auto& registry = m_Scene.GetRegistry();
        m_ComponentUIDescriptions.push_back(CreateRef<TagUIDesc>(registry));
        m_ComponentUIDescriptions.push_back(CreateRef<LocalToWorldTransformUIDesc>(registry));
        m_ComponentUIDescriptions.push_back(CreateRef<LocalToParentTransformUIDesc>(registry));
        m_ComponentUIDescriptions.push_back(CreateRef<BoxCollider2DUIDesc>(registry));
        m_ComponentUIDescriptions.push_back(CreateRef<RigidBody2DUIDesc>(registry));
        m_ComponentUIDescriptions.push_back(CreateRef<SpriteRendererUIDesc>(registry));
    }

    void ScenePanels::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(ScenePanels::OnMousePressed));
    }

    void ScenePanels::OnUpdate()
    {
        FindActiveEntityOnViewPortPressed();
    }

    void ScenePanels::OnImguiUpdate()
    {
        DrawHierarchyPanel();
        DrawInspectorPanel();
    }

    bool ScenePanels::OnMousePressed(MouseButtonPressedEvent& event)
    {
        m_FindActiveEntity = true;
        return false;
    }

    void ScenePanels::FindActiveEntityOnViewPortPressed()
    {
        if (!m_FindActiveEntity) return;
        m_FindActiveEntity = false;
        FrameBuffer* frameBuffer = m_Scene.GetMainFrameBuffer();
        ENGINE_ASSERT(frameBuffer != nullptr, "Framebuffer is unset")

        // Read id from framebuffer texture (should only be executed in "Editor" mode.

        // First check that mouse is in main viewport, because imgui windows outside have
        // entity id of 0, instead of "clear" id of -1.
        glm::vec2 mousePos = Input::MousePosition();
        if (mousePos.x < 0.0f || mousePos.x > ImguiState::MainViewportSize.x ||
            mousePos.y < 0.0f || mousePos.y > ImguiState::MainViewportSize.y)
        {
            return;
        }
        // Read id texture, and get entityId from it.
        I32 entityId = frameBuffer->ReadPixel(1, static_cast<U32>(mousePos.x), static_cast<U32>(mousePos.y),
                                                RendererAPI::DataType::Int);
        m_ActiveEntity = entityId;
    }

    void ScenePanels::DrawHierarchyPanel()
    {
        m_TraversalMap.clear();
        std::vector<Entity> topLevelHierarchy = FindTopLevelHierarchy();
        auto& registry = m_Scene.GetRegistry();
        ImGui::Begin("Scene");

        for (auto e : topLevelHierarchy)
        {
            DrawHierarchyOf(e);
            MarkHierarchyOf(e);
        }
        
        for (auto e : View<>(registry))
        {
            if (m_TraversalMap[e] == true) continue;
            m_TraversalMap[e] = true;
            
            auto& tag = registry.Get<Component::Tag>(e);
            ImGuiTreeNodeFlags flags = ((m_ActiveEntity == e) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void*)(U64)(U32)e, flags, (tag.TagName + " (" + std::to_string(e.Id) + ")").c_str());
            if (ImGui::IsItemClicked())
            {
                m_ActiveEntity = e;
            }
            if (opened)
            {
                ImGui::TreePop();
            }
        }
        // Right-click to create an entity.
        if (ImGui::BeginPopupContextWindow(0, 1, false))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                Entity e = registry.CreateEntity("Empty Entity");
                registry.Add<Component::LocalToWorldTransform2D>(e);
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    void ScenePanels::DrawHierarchyOf(Entity entity)
    {
        auto& registry = m_Scene.GetRegistry();

        bool hasChildren = registry.Has<Component::ChildRel>(entity);
        
        auto& tag = registry.Get<Component::Tag>(entity);
        ImGuiTreeNodeFlags flags = ((m_ActiveEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth | (hasChildren ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        bool opened = ImGui::TreeNodeEx((void*)(U64)(U32)entity, flags, (tag.TagName + " (" + std::to_string(entity.Id) + ")").c_str());
        if (ImGui::IsItemClicked())
        {
            m_ActiveEntity = entity;
        }
        if (opened)
        {
            if (hasChildren)
            {
                auto& childRel = registry.Get<Component::ChildRel>(entity);
                auto curr = childRel.First;
                while (curr != NULL_ENTITY)
                {
                    DrawHierarchyOf(curr);
                    auto& parentRel = registry.Get<Component::ParentRel>(curr);
                    curr = parentRel.Next;
                }
            }
            ImGui::TreePop();
        }
    }

    void ScenePanels::MarkHierarchyOf(Entity entity)
    {
        auto& registry = m_Scene.GetRegistry();
        m_TraversalMap[entity] = true;
        if (registry.Has<Component::ChildRel>(entity))
        {
            auto& childRel = registry.Get<Component::ChildRel>(entity);
            auto curr = childRel.First;
            while (curr != NULL_ENTITY)
            {
                MarkHierarchyOf(curr);
                auto& parentRel = registry.Get<Component::ParentRel>(curr);
                curr = parentRel.Next;
            }
        }
    }
    
    std::vector<Entity> ScenePanels::FindTopLevelHierarchy()
    {
        std::vector<Entity> result;
        std::unordered_map<Entity, bool> traversal;
        auto& registry = m_Scene.GetRegistry();
        for (auto e : View<Component::ParentRel>(registry))
        {
            if (traversal[e]) continue;
            auto& parentRel = registry.Get<Component::ParentRel>(e);
            Entity curr = parentRel.Parent;
            while(registry.Has<Component::ParentRel>(curr))
            {
                traversal[curr] = true;
                curr = registry.Get<Component::ParentRel>(curr).Parent;
            }
            result.push_back(curr);
        }
        return result;
    }

    void ScenePanels::DrawInspectorPanel()
    {
        auto& registry = m_Scene.GetRegistry();
        ImGui::Begin("Inspector");
        if (m_ActiveEntity.Id != registry.GetEntityManager().GetNullEntityFlag())
        {
            ImGui::PushID(static_cast<I32>(m_ActiveEntity));
            SaveState();
            for (auto& uiDesc : m_ComponentUIDescriptions)
            {
                if (uiDesc->ShouldDraw(m_ActiveEntity)) DrawUIOfComponent(*uiDesc);
            }

            if (registry.Has<Component::BoxCollider2D>(m_ActiveEntity))
            {
                SceneUtils::SynchronizePhysics(m_Scene, m_ActiveEntity, SceneUtils::PhysicsSynchroSetting::ColliderOnly);
            }
            if (registry.Has<Component::RigidBody2D>(m_ActiveEntity))
            {
                SceneUtils::SynchronizePhysics(m_Scene, m_ActiveEntity, SceneUtils::PhysicsSynchroSetting::RBOnly);
            }
            CheckState();
            ImGui::PopID();
        }
        ImGui::End();
    }

    void ScenePanels::DrawUIOfComponent(ComponentUIDescBase& uiDesc)
    {
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        auto& registry = uiDesc.GetAttachedRegistry();
    
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)uiDesc.GetComponentID(), flags, uiDesc.GetComponentName().c_str());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x);
        if (ImGui::Button("+"))
        {
            if (uiDesc.IsComponentRemovable()) ImGui::OpenPopup("ComponentSettings");
        }

        bool removeComponent = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
            if (ImGui::MenuItem("Remove component"))
                removeComponent = true;
            ImGui::EndPopup();
        }

        if (open)
        {
            uiDesc.OnUIDraw(m_ActiveEntity);
            ImGui::TreePop();
        }

        if (removeComponent) uiDesc.RemoveComponent(m_ActiveEntity);
    }

    void ScenePanels::SaveState()
    {
        auto& registy = m_Scene.GetRegistry();
        Component::LocalToWorldTransform2D tf = registy.Has<Component::LocalToParentTransform2D>(m_ActiveEntity) ?
            Component::LocalToWorldTransform2D(registy.Get<Component::LocalToParentTransform2D>(m_ActiveEntity)) :
            registy.Get<Component::LocalToWorldTransform2D>(m_ActiveEntity);
        m_SavedState.Position = tf.Position;
    }

    void ScenePanels::CheckState()
    {
        auto& registy = m_Scene.GetRegistry();
        Component::LocalToWorldTransform2D tf = registy.Has<Component::LocalToParentTransform2D>(m_ActiveEntity) ?
            Component::LocalToWorldTransform2D(registy.Get<Component::LocalToParentTransform2D>(m_ActiveEntity)) :
            registy.Get<Component::LocalToWorldTransform2D>(m_ActiveEntity);
        if (tf.Position != m_SavedState.Position)
        {
            if (registy.Has<Component::RigidBody2D>(m_ActiveEntity))
            {
                auto& rb = registy.Get<Component::RigidBody2D>(m_ActiveEntity);
                auto& prb = rb.PhysicsBody;
                prb->SetAngularVelocity(0.0f);
                prb->SetLinearVelocity(glm::vec2{0.0f});
                prb->ResetForce();
                prb->ResetTorque();
            }
        }
    }
    
}
