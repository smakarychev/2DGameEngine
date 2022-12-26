#include "enginepch.h"
#include "ComponentSerializer.h"
#include "SerializationUtils.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneUtils.h"

namespace Engine
{
    ComponentSerializerBase::ComponentSerializerBase(const ComponentSignature& signature, YAML::Emitter& emitter,
                                                     Scene& scene)
        : m_ComponentSignature(signature), m_Emitter(emitter),
          m_Scene(scene), m_Registry(m_Scene.GetRegistry())
    {
    }

    TagSerializer::TagSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::Tag>("Tag", emitter, scene)
    {
    }

    void TagSerializer::SerializeComponent(const Component::Tag& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Tag" << YAML::Value << component.TagName;
        m_Emitter << YAML::EndMap;
    }

    void TagSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto tag = node[m_ComponentSignature];
        if (tag)
        {
            auto& tagComp = m_Registry.AddOrGet<Component::Tag>(e);
            tagComp.TagName = tag["Tag"].as<std::string>();
        }
    }

    LocalToWorldTransformSerializer::LocalToWorldTransformSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::LocalToWorldTransform2D>("LocalToWorldTransform", emitter, scene)
    {
    }

    void LocalToWorldTransformSerializer::SerializeComponent(const Component::LocalToWorldTransform2D& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Position" << YAML::Value << component.Position;
        m_Emitter << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
        m_Emitter << YAML::Key << "Scale" << YAML::Value << component.Scale;
        m_Emitter << YAML::EndMap;
    }

    void LocalToWorldTransformSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto tf = node[m_ComponentSignature];
        if (tf)
        {
            auto& ltwtf = m_Registry.AddOrGet<Component::LocalToWorldTransform2D>(e);
            ltwtf.Position = tf["Position"].as<glm::vec2>();
            ltwtf.Rotation = tf["Rotation"].as<glm::vec2>();
            ltwtf.Scale = tf["Scale"].as<glm::vec2>();
        }
    }

    LocalToParentTransformSerializer::LocalToParentTransformSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::LocalToParentTransform2D>("LocalToParentTransform", emitter, scene)
    {
    }

    void LocalToParentTransformSerializer::SerializeComponent(const Component::LocalToParentTransform2D& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Position" << YAML::Value << component.Position;
        m_Emitter << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
        m_Emitter << YAML::Key << "Scale" << YAML::Value << component.Scale;
        m_Emitter << YAML::EndMap;
    }

    void LocalToParentTransformSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto tf = node[m_ComponentSignature];
        if (tf)
        {
            auto& ltptf = m_Registry.AddOrGet<Component::LocalToParentTransform2D>(e);
            ltptf.Position = tf["Position"].as<glm::vec2>();
            ltptf.Rotation = tf["Rotation"].as<glm::vec2>();
            ltptf.Scale = tf["Scale"].as<glm::vec2>();
        }
    }

    ChildRelSerializer::ChildRelSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::ChildRel>("ChildRel", emitter, scene)
    {
    }

    void ChildRelSerializer::SerializeComponent(const Component::ChildRel& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "ChildrenCount" << YAML::Value << component.ChildrenCount;
        m_Emitter << YAML::Key << "First" << YAML::Value << component.First;
        m_Emitter << YAML::EndMap;
    }

    void ChildRelSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto chR = node[m_ComponentSignature];
        if (chR)
        {
            auto& chRComp = m_Registry.AddOrGet<Component::ChildRel>(e);
            chRComp.ChildrenCount = chR["ChildrenCount"].as<U32>();
            chRComp.First = chR["First"].as<Entity>();
        }
    }

    ParentRelSerializer::ParentRelSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::ParentRel>("ParentRel", emitter, scene)
    {
    }

    void ParentRelSerializer::SerializeComponent(const Component::ParentRel& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Depth" << YAML::Value << component.Depth;
        m_Emitter << YAML::Key << "Parent" << YAML::Value << component.Parent;
        m_Emitter << YAML::Key << "Next" << YAML::Value << component.Next;
        m_Emitter << YAML::Key << "Prev" << YAML::Value << component.Prev;
        m_Emitter << YAML::EndMap;
    }

    void ParentRelSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto pR = node[m_ComponentSignature];
        if (pR)
        {
            auto& pRComp = m_Registry.AddOrGet<Component::ParentRel>(e);
            pRComp.Depth = pR["Depth"].as<U32>();
            pRComp.Parent = pR["Parent"].as<Entity>();
            pRComp.Next = pR["Next"].as<Entity>();
            pRComp.Prev = pR["Prev"].as<Entity>();
        }
    }

    RigidBody2DSerializer::RigidBody2DSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::RigidBody2D>("RigidBody2D", emitter, scene)
    {
    }

    void RigidBody2DSerializer::SerializeComponent(const Component::RigidBody2D& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Type" << YAML::Value << (component.Type == Physics::RigidBodyType2D::Dynamic
                                                                ? "Dynamic"
                                                                : "Static");
        m_Emitter << YAML::Key << "Flags" << YAML::Value << YAML::BeginSeq;
        auto flagsAsStrings = GetRigidBodyFlagsAsStrings(component);
        for (auto& flag : flagsAsStrings)
        {
            m_Emitter << YAML::Value << flag;
        }
        m_Emitter << YAML::EndSeq;
        m_Emitter << YAML::EndMap;
    }

    void RigidBody2DSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto rB = node[m_ComponentSignature];
        if (rB)
        {
            auto& pRComp = m_Registry.AddOrGet<Component::RigidBody2D>(e);
            if (!pRComp.PhysicsBody.Get()) SceneUtils::AddDefaultPhysicalRigidBody2D(m_Scene, e);
            pRComp.Type = rB["Type"].as<std::string>() == "Dynamic"
                              ? Physics::RigidBodyType2D::Dynamic
                              : Physics::RigidBodyType2D::Static;
            Physics::RigidBodyDef2D::BodyFlags flags = GetBodyFlagsFromStrings(
                rB["Flags"].as<std::vector<std::string>>());
            pRComp.Flags = flags;
        }
    }

    BoxCollider2DSerializer::BoxCollider2DSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::BoxCollider2D>("BoxCollider2D", emitter, scene)
    {
    }

    void BoxCollider2DSerializer::SerializeComponent(const Component::BoxCollider2D& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;

        m_Emitter << YAML::Key << "PhysicsMaterial" << YAML::Value;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Restitution" << YAML::Value << component.PhysicsMaterial.Restitution;
        m_Emitter << YAML::Key << "Friction" << YAML::Value << component.PhysicsMaterial.Friction;
        m_Emitter << YAML::Key << "Density" << YAML::Value << component.PhysicsMaterial.Density;
        m_Emitter << YAML::EndMap;

        m_Emitter << YAML::Key << "Offset" << YAML::Value << component.Offset;
        m_Emitter << YAML::Key << "HalfSize" << YAML::Value << component.HalfSize;
        m_Emitter << YAML::Key << "IsSensor" << YAML::Value << component.IsSensor;
        m_Emitter << YAML::EndMap;
    }

    void BoxCollider2DSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto box = node[m_ComponentSignature];
        if (box)
        {
            auto& boxComp = m_Registry.AddOrGet<Component::BoxCollider2D>(e);
            if (!boxComp.PhysicsCollider.Get()) SceneUtils::AddDefaultBoxCollider2D(m_Scene, e);
            boxComp.PhysicsMaterial.Restitution = box["PhysicsMaterial"]["Restitution"].as<F32>();
            boxComp.PhysicsMaterial.Friction = box["PhysicsMaterial"]["Friction"].as<F32>();
            boxComp.PhysicsMaterial.Density = box["PhysicsMaterial"]["Density"].as<F32>();
            boxComp.Offset = box["Offset"].as<glm::vec2>();
            boxComp.HalfSize = box["HalfSize"].as<glm::vec2>();
            boxComp.IsSensor = box["IsSensor"].as<bool>();
        }
    }

    SpriteRendererSerializer::SpriteRendererSerializer(YAML::Emitter& emitter, Scene& scene)
        : ComponentSerializer<Component::SpriteRenderer>("SpriteRenderer", emitter, scene)
    {
    }

    void SpriteRendererSerializer::SerializeComponent(const Component::SpriteRenderer& component)
    {
        m_Emitter << YAML::Key << m_ComponentSignature;
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Texture" << YAML::Value << (component.Texture
                                                                   ? component.Texture->GetData().Name
                                                                   : "NULL");

        m_Emitter << YAML::Key << "UV" << YAML::Value << YAML::BeginSeq;
        for (auto& uv : component.UV) m_Emitter << uv;
        m_Emitter << YAML::EndSeq;

        m_Emitter << YAML::Key << "Tint" << YAML::Value << component.Tint;
        m_Emitter << YAML::Key << "Tiling" << YAML::Value << component.Tiling;

        m_Emitter << YAML::Key << "SortingLayer" << YAML::Value << YAML::BeginMap;
        m_Emitter << YAML::Key << "Name" << YAML::Value << component.SortingLayer.Name;
        m_Emitter << YAML::Key << "Id" << YAML::Value << component.SortingLayer.Id;
        m_Emitter << YAML::Key << "Priority" << YAML::Value << static_cast<U32>(component.SortingLayer.Priority);
        m_Emitter << YAML::EndMap;

        m_Emitter << YAML::Key << "OrderInLayer" << YAML::Value << component.OrderInLayer;
        m_Emitter << YAML::Key << "FlipX" << YAML::Value << component.FlipX;
        m_Emitter << YAML::Key << "FlipY" << YAML::Value << component.FlipY;
        m_Emitter << YAML::EndMap;
    }

    void SpriteRendererSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto sR = node[m_ComponentSignature];
        if (sR)
        {
            auto& sRComp = m_Registry.AddOrGet<Component::SpriteRenderer>(e);
            sRComp.Texture = (sR["Texture"].as<std::string>() == "NULL"
                                  ? nullptr
                                  : Texture::LoadTextureFromFile(sR["Texture"].as<std::string>()).get());

            for (U32 i = 0; i < 4; i++) sRComp.UV[i] = sR["UV"][i].as<glm::vec2>();

            sRComp.Tint = sR["Tint"].as<glm::vec4>();
            sRComp.Tiling = sR["Tiling"].as<glm::vec2>();
            sRComp.SortingLayer.Name = sR["SortingLayer"]["Name"].as<std::string>();
            sRComp.SortingLayer.Id = sR["SortingLayer"]["Id"].as<U32>();
            sRComp.SortingLayer.Priority = static_cast<U8>(sR["SortingLayer"]["Priority"].as<U32>());
            sRComp.OrderInLayer = sR["OrderInLayer"].as<I16>();
            sRComp.FlipX = sR["FlipX"].as<bool>();
            sRComp.FlipY = sR["FlipY"].as<bool>();
        }
    }
}
