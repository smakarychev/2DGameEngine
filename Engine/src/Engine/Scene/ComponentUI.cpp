#include "enginepch.h"
#include "ComponentUI.h"

#include "Scene.h"
#include "Engine/ECS/Registry.h"
#include "Engine/Imgui/ImguiCommon.h"
#include "imgui/imgui.h"

namespace Engine
{
    ComponentUIDescBase::ComponentUIDescBase(const ComponentSignature& signature, bool isRemovable, Scene& scene)
        : m_Signature(signature), m_IsRemovable(isRemovable), m_Scene(scene), m_Registry(scene.GetRegistry()) 
    {
    }

    ComponentUIDescBase::~ComponentUIDescBase() = default;

    LocalToWorldTransformUIDesc::LocalToWorldTransformUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), false, scene)
    {
    }

    void LocalToWorldTransformUIDesc::OnUIDraw(Entity e, Component::LocalToWorldTransform2D& component)
    {
        if (m_Registry.Has<Component::LocalToParentTransform2D>(e)) return;
        F32 rotationDegrees = glm::degrees(std::atan2(component.Rotation[1], component.Rotation[0]));
        ImGuiCommon::DrawFloat2("Position", component.Position, 0.05f, -IMGUI_LIMITLESS, IMGUI_LIMITLESS);
        ImGuiCommon::DrawFloat("Rotation", rotationDegrees, 0.05f, -360.0f, 360.0f);
        ImGuiCommon::DrawFloat2("Scale", component.Scale, 0.05f, 0.0f, IMGUI_LIMITLESS);
        const F32 rotationRadians = glm::radians(rotationDegrees);
        component.Rotation = glm::vec2{glm::cos(rotationRadians), glm::sin(rotationRadians)};
    }

    bool LocalToWorldTransformUIDesc::ShouldDraw(Entity e)
    {
        return ComponentUIDesc<Component::LocalToWorldTransform2D>::ShouldDraw(e) &&
            !m_Registry.Has<Component::LocalToParentTransform2D>(e);
    }

    LocalToParentTransformUIDesc::LocalToParentTransformUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), false, scene)
    {
    }

    void LocalToParentTransformUIDesc::OnUIDraw(Entity e, Component::LocalToParentTransform2D& component)
    {
        F32 rotationDegrees = glm::degrees(std::atan2(component.Rotation[1], component.Rotation[0]));
        ImGuiCommon::DrawFloat2("Position", component.Position, 0.05f, -IMGUI_LIMITLESS, IMGUI_LIMITLESS);
        ImGuiCommon::DrawFloat("Rotation", rotationDegrees, 0.05f, -360.0f, 360.0f);
        ImGuiCommon::DrawFloat2("Scale", component.Scale, 0.05f, 0.0f, IMGUI_LIMITLESS);
        const F32 rotationRadians = glm::radians(rotationDegrees);
        component.Rotation = glm::vec2{glm::cos(rotationRadians), glm::sin(rotationRadians)};
    }

    NameUIDesc::NameUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), false, scene)
    {
    }

    void NameUIDesc::OnUIDraw(Entity e, Component::Name& component)
    {
        std::string oldName = component.EntityName;
        ImGuiCommon::DrawTextField("Name", component.EntityName);
    }

    BoxCollider2DUIDesc::BoxCollider2DUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), true, scene)
    {
    }

    void BoxCollider2DUIDesc::OnUIDraw(Entity e, Component::BoxCollider2D& component)
    {
        ImGuiCommon::DrawFloat2("Position", component.Offset, 0.05f, -IMGUI_LIMITLESS, IMGUI_LIMITLESS);
        ImGuiCommon::DrawFloat2("Scale", component.HalfSize, 0.05f, 0.0f, IMGUI_LIMITLESS);
        ImGui::Checkbox("Is sensor", &component.IsSensor);
        const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
        bool opened = ImGui::TreeNodeEx(&component, flags, "Physics material");
        if (opened)
        {
            ImGuiCommon::DrawFloat("Restitution", component.PhysicsMaterial.Restitution, 0.05f, 0.0f, 1.0f);
            ImGuiCommon::DrawFloat("Friction", component.PhysicsMaterial.Friction, 0.05f, 0.0f, 1.0f);
            ImGui::TreePop();
        }
    }

    RigidBody2DUIDesc::RigidBody2DUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), true, scene)
    {
    }

    void RigidBody2DUIDesc::OnUIDraw(Entity e, Component::RigidBody2D& component)
    {
        const std::array<std::string, 2> types = { "Static", "Dynamic" };
        std::string currentType =
            component.Type == Physics::RigidBodyType2D::Dynamic ? types[1] : types[0];
        F32 mass = component.PhysicsBody->GetMass();
        ImGuiCommon::DrawFloat("Mass", mass, 0.05f, 0.0f, IMGUI_LIMITLESS);
        component.PhysicsBody->SetMass(mass);
        if (ImGuiCommon::BeginCombo("Type", currentType.c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (const auto& type : types)
            {
                bool isSelected = currentType == type;
                if (ImGui::Selectable(type.c_str(), isSelected)) currentType = type;
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGuiCommon::EndCombo();
        }
        if (currentType == "Static") component.Type = Physics::RigidBodyType2D::Static;
        else component.Type = Physics::RigidBodyType2D::Dynamic;
        // TODO: flags
    }

    SpriteRendererUIDesc::SpriteRendererUIDesc(Scene& scene)
        : ComponentUIDesc(GetStaticSignature(), true, scene)
    {
    }

    void SpriteRendererUIDesc::OnUIDraw(Entity e, Component::SpriteRenderer& component)
    {
        ImGuiCommon::DrawColor("Tint", component.Tint);
        ImGuiCommon::DrawFloat2("Tiling", component.Tiling, 0.05f, 0.0f, IMGUI_LIMITLESS);
    }
}
