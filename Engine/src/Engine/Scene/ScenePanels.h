﻿#pragma once
#include "ComponentUI.h"
#include "Engine/ECS/EntityId.h"

#include "Engine/Imgui/ImguiCommon.h"

namespace Engine
{
    class Event;
    class MouseButtonPressedEvent;
    class Scene;

    class ScenePanels
    {
    public:
        struct FileDialogInfo
        {
            bool IsActive{false};
            std::string FileName{};
        };
        struct PrefabDialogInfo : FileDialogInfo
        {
            Entity Entity;
        };
        struct AssetsPanelInfo
        {
            std::filesystem::path RootAssetsPath = "assets";
            std::filesystem::path CurrentPath = RootAssetsPath;
            F32 IconsWidth = 80.0f;
            F32 IconsPadding = 20.0f;
            struct Icons
            {
                Ref<Texture> FileIcon{Texture::LoadTextureFromFile("assets/textures/editor/file.png")};
                Ref<Texture> DirectoryIcon{Texture::LoadTextureFromFile("assets/textures/editor/directory.png")};
                Ref<Texture> BackIcon{Texture::LoadTextureFromFile("assets/textures/editor/back.png")};
            };
            Icons Icons;
        };
        struct SceneControlsInfo
        {
            F32 IconsWidth = 25.0f;
            struct Icons
            {
                Ref<Texture> PauseIcon{Texture::LoadTextureFromFile("assets/textures/editor/pause.png")};
                Ref<Texture> PlayIcon{Texture::LoadTextureFromFile("assets/textures/editor/play.png")};
            };
            Icons Icons;
        };
    public:
        ScenePanels(Scene& scene);
        void OnEvent(Event& event);
        void OnUpdate();
        void OnImguiUpdate();
        void OnMainMenuDraw();

        template <typename T>
        void AddComponentUiDesc();
        
        void ResetActiveEntity() { m_ActiveEntity = NULL_ENTITY; }
        Entity GetActiveEntity() const { return m_ActiveEntity; }
    private:
        bool OnMousePressed(MouseButtonPressedEvent& event);
        void FindActiveEntityOnViewPortPressed();
        void DrawHierarchyPanel();
        void DrawHierarchyOf(Entity entity);
        void MarkHierarchyOf(Entity entity, std::unordered_map<Entity, bool>& traversalMap);

        void DrawInspectorPanel();
        void DrawUIOfComponent(ComponentUIDescBase& uiDesc);
        void DrawAddComponentOption();

        void DrawDialogs();

        void DrawAssetsPanel();

        void DrawPlayPauseButtons();
        
        std::vector<Entity> FindTopLevelEntities();
        

        void SaveState();
        void CheckState();
        
    private:
        Scene& m_Scene;
        // Active entity ui things.
        bool m_FindActiveEntity{false};
        Entity m_ActiveEntity{NULL_ENTITY};
        Entity m_ToDeleteEntity{NULL_ENTITY};
        
        FileDialogInfo m_SaveDialog{};
        FileDialogInfo m_OpenDialog{};
        PrefabDialogInfo m_PrefabDialog{};
        
        AssetsPanelInfo m_AssetsPanelInfo{};
        SceneControlsInfo m_SceneControlsInfo{};
        
        std::vector<Ref<ComponentUIDescBase>> m_ComponentUIDescriptions;
        
        struct SavedState
        {
            glm::vec2 Position{};
        };
        SavedState m_SavedState{};
    };

    template <typename T>
    void ScenePanels::AddComponentUiDesc()
    {
        auto newUIDesc = CreateRef<T>(m_Scene);
        auto it = std::ranges::find_if(m_ComponentUIDescriptions, [&](auto& el)
        {
            return el->GetSignature() == newUIDesc->GetSignature();
        });
        if (it != m_ComponentUIDescriptions.end()) return;
        m_ComponentUIDescriptions.push_back(newUIDesc);
    }
}
