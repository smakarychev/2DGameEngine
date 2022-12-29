#pragma once
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
    public:
        ScenePanels(Scene& scene);
        void OnEvent(Event& event);
        void OnUpdate();
        void OnImguiUpdate();
        void OnMainMenuDraw();

    private:
        bool OnMousePressed(MouseButtonPressedEvent& event);
        void FindActiveEntityOnViewPortPressed();
        void DrawHierarchyPanel();
        void DrawHierarchyOf(Entity entity);
        void MarkHierarchyOf(Entity entity, std::unordered_map<Entity, bool>& traversalMap);

        void DrawInspectorPanel();
        void DrawUIOfComponent(ComponentUIDescBase& uiDesc);
        void DrawDialogs();

        
        std::vector<Entity> FindTopLevelEntities();
        

        void SaveState();
        void CheckState();
        
    private:
        Scene& m_Scene;
        // Active entity ui things.
        bool m_FindActiveEntity{false};
        Entity m_ActiveEntity{};

        FileDialogInfo m_SaveDialog{};
        FileDialogInfo m_OpenDialog{};
        PrefabDialogInfo m_PrefabDialog{};
        
        std::unordered_map<Entity, bool> m_TraversalMap;
        std::vector<Ref<ComponentUIDescBase>> m_ComponentUIDescriptions;
        
        struct SavedState
        {
            glm::vec2 Position{};
        };
        SavedState m_SavedState{};
    };
}
