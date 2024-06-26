﻿#include "enginepch.h"
#include "ScenePanels.h"

#include "Scene.h"
#include "SceneUtils.h"
#include "Engine/Core/Input.h"
#include "Engine/ECS/View.h"
#include "Serialization/Prefab.h"

namespace Engine
{
    ScenePanels::ScenePanels(Scene& scene)
        : m_Scene(scene)
    {
        m_ActiveEntity = NULL_ENTITY;
        m_ComponentUIDescriptions.push_back(CreateRef<NameUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<LocalToWorldTransformUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<LocalToParentTransformUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<BoxCollider2DUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<RigidBody2DUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<SpriteRendererUIDesc>(m_Scene));
        m_ComponentUIDescriptions.push_back(CreateRef<CameraUIDesc>(m_Scene));
    }

    void ScenePanels::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_FN(ScenePanels::OnMousePressed));
    }

    void ScenePanels::OnUpdate()
    {
        if (m_ToDeleteEntity != NULL_ENTITY)
        {
            SceneUtils::DeleteEntity(m_Scene, m_ToDeleteEntity);
            ResetActiveEntity();
            m_ToDeleteEntity = NULL_ENTITY;
        }
        FindActiveEntityOnViewPortPressed();
    }

    void ScenePanels::OnImguiUpdate()
    {
        DrawHierarchyPanel();
        DrawInspectorPanel();
    }

    void ScenePanels::OnMainMenuDraw()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save", nullptr, false)) m_SaveDialog.IsActive = true;
                if (ImGui::MenuItem("Open", nullptr, false)) m_OpenDialog.IsActive = true;
                ImGui::EndMenu();
            }
            DrawPlayPauseButtons();
            ImGui::EndMainMenuBar();
        }
        // TODO: find a better place for it.
        DrawDialogs();
        DrawAssetsPanel();
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
        if (!SceneUtils::HasEntityUnderMouse(mousePos, frameBuffer)) return;
        Entity entityUnderMouse = SceneUtils::GetEntityUnderMouse(mousePos, frameBuffer);
        m_ActiveEntity = entityUnderMouse;
        // If entity belongs to prefab, set it's prefab as active entity instead.
        auto& registry = m_Scene.GetRegistry();
        if (registry.Has<Component::BelongsToPrefab>(m_ActiveEntity)) 
        {
            m_ActiveEntity = SceneUtils::FindParentingPrefab(m_ActiveEntity, registry);
        }
    }

    void ScenePanels::DrawHierarchyPanel()
    {
        auto& registry = m_Scene.GetRegistry();

        ImGui::Begin("Scene");
        std::vector<Entity> topLevelEntities = FindTopLevelEntities();
        for (auto e : topLevelEntities)
        {
            DrawHierarchyOf(e);
        }

        // Drag and drop to change hierarchy.
        ImGui::Dummy(ImGui::GetContentRegionAvail());
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("entityHierarchy"))
            {
                Entity dragged = *static_cast<Entity*>(payload->Data);
                if (registry.Has<Component::ParentRel>(dragged)) SceneUtils::RemoveChild(m_Scene, dragged);
            }
            ImGui::EndDragDropTarget();
        }

        // Right-click to create an entity.
        if (ImGui::BeginPopupContextWindow(nullptr, 1, false))
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
        bool isPrefab = registry.Has<Component::Prefab>(entity);
        auto& name = registry.Get<Component::Name>(entity);
        
        ImGuiTreeNodeFlags flags = ((m_ActiveEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
            ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth | ((hasChildren && !isPrefab) ? ImGuiTreeNodeFlags_DefaultOpen : 0);
        bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<U64>(entity.Id)), flags,
                                        (name.EntityName + " (" + std::to_string(entity.Id) + ")").c_str());

        // Right click on entity to open it's menu.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Create prefab from entity and it's children"))
            {
                m_PrefabDialog.IsActive = true;
                m_PrefabDialog.Entity = entity;
            }
            if (!registry.Has<Component::BelongsToPrefab>(entity))
            {
                if (ImGui::MenuItem("Delete entity"))
                {
                    m_ToDeleteEntity = entity;
                }
            }
            ImGui::EndPopup();
        }
        
        // Drag and drop to change hierarchy.
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("entityHierarchy", &entity, sizeof(Entity));
            ImGui::Text((name.EntityName + " (" + std::to_string(entity.Id) + ")").c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("entityHierarchy"))
            {
                Entity dragged = *static_cast<Entity*>(payload->Data);
                SceneUtils::AddChild(m_Scene, entity, dragged);
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemClicked())
        {
            m_ActiveEntity = entity;
        }
        
        if (opened)
        {
            if (hasChildren)
            {
                SceneUtils::TraverseChildren(entity, registry, [&](Entity e)
                {
                    DrawHierarchyOf(e);
                });
            }
            ImGui::TreePop();
        }
    }

    void ScenePanels::MarkHierarchyOf(Entity entity, std::unordered_map<Entity, bool>& traversalMap)
    {
        auto& registry = m_Scene.GetRegistry();
        if (traversalMap[entity]) return;
        traversalMap[entity] = true;
        if (registry.Has<Component::ChildRel>(entity))
        {
            SceneUtils::TraverseChildren(entity, registry, [&](Entity e) 
            {
                MarkHierarchyOf(e, traversalMap);
            });
        }
    }

    std::vector<Entity> ScenePanels::FindTopLevelEntities()
    {
        std::vector<Entity> result;
        std::unordered_map<Entity, bool> traversal;
        auto& registry = m_Scene.GetRegistry();

        for (auto e : View<>(registry))
        {
            if (traversal[e]) continue;
            MarkHierarchyOf(e, traversal);
            if (registry.Has<Component::ParentRel>(e)) continue;
            result.push_back(e);
        }

        return result;
    }

    void ScenePanels::DrawInspectorPanel()
    {
        auto& registry = m_Scene.GetRegistry();
        ImGui::Begin("Inspector");
        if (m_ActiveEntity != NULL_ENTITY)
        {
            ImGui::PushID(static_cast<I32>(m_ActiveEntity));
            SaveState();
            for (auto& uiDesc : m_ComponentUIDescriptions)
            {
                if (uiDesc->ShouldDraw(m_ActiveEntity)) DrawUIOfComponent(*uiDesc);
            }

            CheckState();
            
            SceneUtils::TraverseTree(m_ActiveEntity, registry, [&](Entity e)
            {
                if (registry.Has<Component::BoxCollider2D>(e))
                {
                    SceneUtils::SynchronizePhysics(m_Scene, e, SceneUtils::PhysicsSynchroSetting::ColliderOnly);
                }
                if (registry.Has<Component::RigidBody2D>(e))
                {
                    SceneUtils::SynchronizePhysics(m_Scene, e, SceneUtils::PhysicsSynchroSetting::RBOnly);
                }
            });

            ImGui::PopID();
        }
        DrawAddComponentOption();
        ImGui::End();
    }

    void ScenePanels::DrawUIOfComponent(ComponentUIDescBase& uiDesc)
    {
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)uiDesc.GetComponentID(), flags, uiDesc.GetSignature());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x);
        if (ImGui::Button("+"))
        {
            if (uiDesc.IsComponentRemovable()) ImGui::OpenPopup("ComponentSettings");
        }

        bool removeComponent = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
            if (ImGui::MenuItem("Remove component")) removeComponent = true;
            ImGui::EndPopup();
        }

        if (open)
        {
            uiDesc.OnUIDraw(m_ActiveEntity);
            ImGui::TreePop();
        }

        if (removeComponent) uiDesc.RemoveComponent(m_ActiveEntity);
    }

    void ScenePanels::DrawAddComponentOption()
    {
        if (m_ActiveEntity == NULL_ENTITY) return;
        if (ImGuiCommon::BeginCombo("Components", ""))
        {
            for (const auto& componentSerializer : m_Scene.GetSerializer().GetComponentSerializers())
            {
                if (!componentSerializer->SupportsCreationInEditor()) continue;
                if (ImGui::Selectable(componentSerializer->GetSignature(), false))
                {
                    componentSerializer->AddEmptyComponentTo(m_ActiveEntity);
                }
            }
            ImGuiCommon::EndCombo();
        }
    }

    void ScenePanels::DrawDialogs()
    {
        if (m_SaveDialog.IsActive)
        {
            ImGui::Begin("Save scene");
            ImGuiCommon::DrawTextField("Scene name", m_SaveDialog.FileName);
            if (ImGui::Button("Save"))
            {
                if (m_SaveDialog.FileName.empty())
                {
                    ENGINE_CORE_ERROR("File name has to be non empty.");
                    ImGui::End();
                    return;
                }
                m_Scene.Save(m_SaveDialog.FileName);
                m_SaveDialog.FileName = {};
                m_SaveDialog.IsActive = false;
            }
            ImGui::End();
        }
        if (m_OpenDialog.IsActive)
        {
            ImGui::Begin("Open scene");
            ImGuiCommon::DrawTextField("Scene name", m_OpenDialog.FileName);
            if (ImGui::Button("Open"))
            {
                if (m_OpenDialog.FileName.empty())
                {
                    ENGINE_CORE_ERROR("File name has to be non empty.");
                    ImGui::End();
                    return;
                }
                m_Scene.Open(m_OpenDialog.FileName);
                m_OpenDialog.FileName = {};
                m_OpenDialog.IsActive = false;
            }
            ImGui::End();
        }
        if (m_PrefabDialog.IsActive)
        {
            ImGui::Begin("Save prefab");
            ImGuiCommon::DrawTextField("Prefab name", m_PrefabDialog.FileName);
            if (ImGui::Button("Save"))
            {
                if (m_PrefabDialog.FileName.empty())
                {
                    ENGINE_CORE_ERROR("Prefab name has to be non empty.");
                    ImGui::End();
                    return;
                }
                PrefabUtils::CreatePrefabFromEntity(m_PrefabDialog.Entity, m_PrefabDialog.FileName, m_Scene);
                m_PrefabDialog.FileName = {};
                m_PrefabDialog.IsActive = false;
            }
            ImGui::End();
        }
    }

    void ScenePanels::DrawAssetsPanel()
    {
        ImGui::Begin("Assets");
        F32 availableWidth = ImGui::GetContentRegionAvail().x;
        F32 totalIconSize = m_AssetsPanelInfo.IconsWidth + m_AssetsPanelInfo.IconsPadding;
        U32 totalColumns = Math::Max(
            static_cast<U32>(availableWidth / totalIconSize), 1u);

        ImGui::Columns(static_cast<I32>(totalColumns), "AssetsColumns", false);

        if (m_AssetsPanelInfo.CurrentPath != m_AssetsPanelInfo.RootAssetsPath)
        {
            Ref<Texture> icon = m_AssetsPanelInfo.Icons.BackIcon;
            if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(icon->GetId()), {totalIconSize, totalIconSize},
                                   {0, 1}, {1, 0}))
            {
                m_AssetsPanelInfo.CurrentPath = m_AssetsPanelInfo.CurrentPath.parent_path();
            }
            ImGui::NextColumn();
        }
        for (const auto& dirEntry : std::filesystem::directory_iterator(m_AssetsPanelInfo.CurrentPath))
        {
            if (!dirEntry.is_directory()) continue;
            std::string pathString = dirEntry.path().stem().string();
            ImGui::PushID(pathString.c_str());

            Ref<Texture> icon = m_AssetsPanelInfo.Icons.DirectoryIcon;
            if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(static_cast<U64>(icon->GetId())),
                                   {totalIconSize, totalIconSize},
                                   {0, 1}, {1, 0}))
            {
                m_AssetsPanelInfo.CurrentPath /= dirEntry.path().filename();
            }

            ImGui::TextWrapped(pathString.c_str());
            ImGui::NextColumn();
            ImGui::PopID();
        }
        for (const auto& dirEntry : std::filesystem::directory_iterator(m_AssetsPanelInfo.CurrentPath))
        {
            if (dirEntry.is_directory()) continue;
            std::string pathString = dirEntry.path().stem().string();
            ImGui::PushID(pathString.c_str());

            Ref<Texture> icon = m_AssetsPanelInfo.Icons.FileIcon;
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<U64>(icon->GetId())),
                    {totalIconSize, totalIconSize},
                    {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                std::string relativePathString = dirEntry.path().string();
                ImGui::SetDragDropPayload("AssetsPayload", relativePathString.c_str(),
                                          (relativePathString.size() + 1) * sizeof(relativePathString[0]));
                ImGui::Text(relativePathString.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::TextWrapped(pathString.c_str());
            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);


        ImGui::End();
    }

    void ScenePanels::DrawPlayPauseButtons()
    {
        if (!m_Scene.IsReady()) return; 
        ImGui::Begin("##PlayAndPause", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
        
        ImGuiStyle& style = ImGui::GetStyle();
        F32 size = m_SceneControlsInfo.IconsWidth + style.FramePadding.x * 2.0f;
        F32 availWidth = ImGui::GetContentRegionAvail().x;
        F32 off = (availWidth - size) * 0.5f;
        if (off > 0.0f) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
        size = m_SceneControlsInfo.IconsWidth;
        
        auto playIcon = m_SceneControlsInfo.Icons.PlayIcon;
        auto pauseIcon = m_SceneControlsInfo.Icons.PauseIcon;
        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(playIcon->GetId()), {size, size},
                         {0, 1}, {1, 0}))
        {
            m_Scene.OnScenePlay();
        }
        ImGui::SameLine();
        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(pauseIcon->GetId()), {size, size},
                         {0, 1}, {1, 0}))
        {
            m_Scene.OnSceneStop();
        }
        ImGui::End();
    }

    void ScenePanels::SaveState()
    {
        auto& registy = m_Scene.GetRegistry();
        Component::LocalToWorldTransform2D tf = registy.Has<Component::LocalToParentTransform2D>(m_ActiveEntity)
                                                    ? Component::LocalToWorldTransform2D(
                                                        registy.Get<
                                                            Component::LocalToParentTransform2D>(m_ActiveEntity))
                                                    : registy.Get<Component::LocalToWorldTransform2D>(m_ActiveEntity);
        m_SavedState.Position = tf.Position;
    }

    void ScenePanels::CheckState()
    {
        auto& registy = m_Scene.GetRegistry();
        bool hasParent = registy.Has<Component::LocalToParentTransform2D>(m_ActiveEntity);
        Component::LocalToWorldTransform2D tf = hasParent
                                                    ? Component::LocalToWorldTransform2D(
                                                        registy.Get<
                                                            Component::LocalToParentTransform2D>(m_ActiveEntity))
                                                    : registy.Get<Component::LocalToWorldTransform2D>(m_ActiveEntity);
        if (tf.Position != m_SavedState.Position)
        {
            SceneUtils::TraverseTree(m_ActiveEntity, registy, [&](Entity e)
            {
                if (registy.Has<Component::RigidBody2D>(e))
                {
                    auto& rb = registy.Get<Component::RigidBody2D>(e);
                    auto& prb = rb.PhysicsBody;
                    prb->SetAngularVelocity(0.0f);
                    prb->SetLinearVelocity(glm::vec2{0.0f});
                    prb->ResetForce();
                    prb->ResetTorque();
                }
            });
        }
        if (hasParent)
        {
            m_Scene.GetSceneGraph().UpdateGraphOfEntity(registy.Get<Component::ParentRel>(m_ActiveEntity).Parent);    
        }
        else
        {
            m_Scene.GetSceneGraph().UpdateGraphOfEntity(m_ActiveEntity); 
        }
    }
}
