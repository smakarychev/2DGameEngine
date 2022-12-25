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
        ScenePanels(Scene& scene);
        void OnEvent(Event& event);
        void OnUpdate();
        void OnImguiUpdate();

    private:
        bool OnMousePressed(MouseButtonPressedEvent& event);
        void FindActiveEntityOnViewPortPressed();
        void DrawHierarchyPanel();
        void DrawHierarchyOf(Entity entity);
        void MarkHierarchyOf(Entity entity);

        void DrawInspectorPanel();
        void DrawUIOfComponent(ComponentUIDescBase& uiDesc);
        
        std::vector<Entity> FindTopLevelHierarchy();

        void SaveState();
        void CheckState();
        
    private:
        Scene& m_Scene;
        // Active entity ui things.
        bool m_FindActiveEntity = false;
        Entity m_ActiveEntity{};

        std::unordered_map<Entity, bool> m_TraversalMap;
        std::vector<Ref<ComponentUIDescBase>> m_ComponentUIDescriptions;
        
        struct SavedState
        {
            glm::vec2 Position{};
        };
        SavedState m_SavedState{};
    };
}
