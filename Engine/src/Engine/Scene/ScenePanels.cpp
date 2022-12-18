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
        FrameBuffer* frameBuffer = m_Scene.GetFrameBuffer();
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
            bool opened = ImGui::TreeNodeEx((void*)(U64)(U32)e, flags, tag.TagName.c_str());
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
                registry.Add<Component::Transform2D>(e);
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
        bool opened = ImGui::TreeNodeEx((void*)(U64)(U32)entity, flags, tag.TagName.c_str());
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

    template <typename T, typename RenderFN>
    static void DrawComponent(const ScenePanels::ComponentUIDescription<T, RenderFN> desc, Scene& scene, Entity entity)
    {
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        auto& registry = scene.GetRegistry();
        if (registry.Has<T>(entity))
        {
            auto& comp = registry.Get<T>(entity);
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)ComponentFamily::TYPE<T>, flags, desc.Name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvailable.x);
            if (ImGui::Button("+"))
            {
                if (desc.IsRemovable) ImGui::OpenPopup("ComponentSettings");
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
                desc.RenderFn(comp);
                ImGui::TreePop();
            }

            if (removeComponent) registry.Remove<T>(entity);
        }
    }

   
    void ScenePanels::DrawInspectorPanel()
    {
        auto& registry = m_Scene.GetRegistry();
        ImGui::Begin("Inspector");
        if (m_ActiveEntity.Id != registry.GetEntityManager().GetNullEntityFlag())
        {
            ImGui::PushID(static_cast<I32>(m_ActiveEntity));

            auto transformCompUI = CreateComponentUIDescription<Component::Transform2D>(
                "Transform2D",
                false,
                [](Component::Transform2D& transform)
                {
                    F32 rotationDegrees = glm::degrees(std::acos(transform.Rotation[0]));
                    if (transform.Rotation[1] < 0.0f) rotationDegrees *= -1.0f;
                    ImGui::DragFloat2("Position", &transform.Position[0], 0.05f, -10.0f, 10.0f);
                    ImGui::DragFloat("Rotation", &rotationDegrees, 0.05f, -360.0f, 360.0f);
                    ImGui::DragFloat2("Scale", &transform.Scale[0], 0.05f, -10.0f, 10.0f);
                    const F32 rotationRadians = glm::radians(rotationDegrees);
                    transform.Rotation = glm::vec2{glm::cos(rotationRadians), glm::sin(rotationRadians)};
                }
            );
            auto tagCompUI = CreateComponentUIDescription<Component::Tag>(
                "Tag",
                false,
                [&](Component::Tag& tag)
                {
                    char newTagName[512];
                    strcpy(newTagName, tag.TagName.c_str());
                    ImGui::InputText("Tag", newTagName, 512);
                    std::string oldName = tag.TagName;
                    tag.TagName = std::string(newTagName);
                    if (oldName != tag.TagName)
                    {
                        registry.PopFromMap(m_ActiveEntity, oldName);
                        registry.PushToMap(m_ActiveEntity, tag.TagName);    
                    }
                }
            );
            auto boxCollider2DCompUI = CreateComponentUIDescription<Component::BoxCollider2D>(
                "BoxCollider2D",
                true,
                [](Component::BoxCollider2D& boxCollider2D)
                {
                    ImGui::DragFloat2("Position", &boxCollider2D.Offset[0], 0.05f, -10.0f, 10.0f);
                    ImGui::DragFloat2("Scale", &boxCollider2D.HalfSize[0], 0.05f, 0.0f, 10.0f);
                }
            );
            auto rigidBody2DCompUI = CreateComponentUIDescription<Component::RigidBody2D>(
                "RigidBody2D",
                true,
                [](Component::RigidBody2D& rigidBody)
                {
                }
            );
            
            DrawComponent<Component::Transform2D>(transformCompUI, m_Scene, m_ActiveEntity);
            DrawComponent<Component::Tag>(tagCompUI, m_Scene, m_ActiveEntity);
            DrawComponent<Component::BoxCollider2D>(boxCollider2DCompUI, m_Scene, m_ActiveEntity);
            DrawComponent<Component::RigidBody2D>(rigidBody2DCompUI, m_Scene, m_ActiveEntity);

            if (registry.Has<Component::RigidBody2D>(m_ActiveEntity))
            {
                SceneUtils::SynchronizePhysics(m_Scene, m_ActiveEntity);
            }
            
            ImGui::PopID();
        }
        ImGui::End();
    }
}
