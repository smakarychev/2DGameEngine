#pragma once
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
        template <typename T, typename UIFunc>
        struct ComponentUIDescription
        {
            using ComponentType = T;
            std::string Name = "Default";
            bool IsRemovable = true;
            UIFunc RenderFn;
        };
        template <typename T, typename UIFunc>
        ComponentUIDescription<T, UIFunc> CreateComponentUIDescription(const std::string& name,  bool isRemovable, UIFunc renderFn)
        {
            return ComponentUIDescription<T, UIFunc>{.Name = name, .IsRemovable = isRemovable, .RenderFn = renderFn};
        }
    public:
        ScenePanels(Scene& scene);
        void OnEvent(Event& event);
        void OnUpdate();
        void OnImguiUpdate();

    private:
        bool OnMousePressed(MouseButtonPressedEvent& event);
        void FindActiveEntityOnViewPortPressed();
        void DrawHierarchyPanel();
        void DrawInspectorPanel();

        void DrawHierarchyOf(Entity entity);
        void MarkHierarchyOf(Entity entity);
        
        std::vector<Entity> FindTopLevelHierarchy();
        
    private:
        Scene& m_Scene;
        // Active entity ui things.
        bool m_FindActiveEntity = false;
        Entity m_ActiveEntity{};

        std::unordered_map<Entity, bool> m_TraversalMap;
        
    };
}
