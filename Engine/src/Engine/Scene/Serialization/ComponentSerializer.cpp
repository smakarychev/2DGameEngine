#include "enginepch.h"
#include "ComponentSerializer.h"

#include "SerializationUtils.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneUtils.h"

namespace Engine
{
    ComponentSerializerBase::ComponentSerializerBase(const ComponentSignature& signature, Scene& scene)
        : m_ComponentSignature(signature),
          m_Scene(scene), m_Registry(m_Scene.GetRegistry())
    {
    }

    NameSerializer::NameSerializer(Scene& scene)
        : ComponentSerializer<Component::Name>(GetStaticSignature(), scene)
    {
    }

    void NameSerializer::SerializeComponent(const Component::Name& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Name" << YAML::Value << component.EntityName;
        emitter << YAML::EndMap;
    }

    void NameSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto name = node[m_ComponentSignature];
        auto& nameComp = m_Registry.AddOrGet<Component::Name>(e);
        nameComp.EntityName = name["Name"].as<std::string>();
    }

    LocalToWorldTransformSerializer::LocalToWorldTransformSerializer(Scene& scene)
        : ComponentSerializer<Component::LocalToWorldTransform2D>(GetStaticSignature(), scene)
    {
    }

    void LocalToWorldTransformSerializer::SerializeComponent(const Component::LocalToWorldTransform2D& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Position" << YAML::Value << component.Position;
        emitter << YAML::Key << "Rotation" << YAML::Value << component.Rotation.RotationVec;
        emitter << YAML::Key << "Scale" << YAML::Value << component.Scale;
        emitter << YAML::EndMap;
    }

    void LocalToWorldTransformSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto tf = node[m_ComponentSignature];
        auto& ltwtf = m_Registry.AddOrGet<Component::LocalToWorldTransform2D>(e);
        ltwtf.Position = tf["Position"].as<glm::vec2>();
        ltwtf.Rotation = tf["Rotation"].as<glm::vec2>();
        ltwtf.Scale = tf["Scale"].as<glm::vec2>();
    }

    LocalToParentTransformSerializer::LocalToParentTransformSerializer(Scene& scene)
        : ComponentSerializer<Component::LocalToParentTransform2D>(GetStaticSignature(), scene)
    {
    }

    void LocalToParentTransformSerializer::SerializeComponent(const Component::LocalToParentTransform2D& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Position" << YAML::Value << component.Position;
        emitter << YAML::Key << "Rotation" << YAML::Value << component.Rotation.RotationVec;
        emitter << YAML::Key << "Scale" << YAML::Value << component.Scale;
        emitter << YAML::EndMap;
    }

    void LocalToParentTransformSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto tf = node[m_ComponentSignature];
        auto& ltptf = m_Registry.AddOrGet<Component::LocalToParentTransform2D>(e);
        ltptf.Position = tf["Position"].as<glm::vec2>();
        ltptf.Rotation = tf["Rotation"].as<glm::vec2>();
        ltptf.Scale = tf["Scale"].as<glm::vec2>();
    }

    CameraSerializer::CameraSerializer(Scene& scene)
        : ComponentSerializer<Component::Camera>(GetStaticSignature(), scene)
    {
    }

    void CameraSerializer::SerializeComponent(const Component::Camera& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "CameraController" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Type" << YAML::Value <<
            SerializationUtils::GetCameraControllerTypeAsString(component);
        emitter << YAML::EndMap;

        emitter << YAML::Key << "IsPrimary" << YAML::Value << component.IsPrimary;

        emitter << YAML::EndMap;
    }

    void CameraSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto cam = node[m_ComponentSignature];
        CameraController::ControllerType type = SerializationUtils::GetCameraControllerTypeFromString(
                cam["CameraController"]["Type"].as<std::string>());
        auto& camComp = SceneUtils::AddDefault2DCamera(m_Scene, e, type);
        camComp.IsPrimary = cam["IsPrimary"].as<bool>();
    }

    ChildRelSerializer::ChildRelSerializer(Scene& scene)
        : ComponentSerializer<Component::ChildRel>(GetStaticSignature(), scene)
    {
    }

    void ChildRelSerializer::SerializeComponent(const Component::ChildRel& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "ChildrenCount" << YAML::Value << component.ChildrenCount;
        emitter << YAML::Key << "First" << YAML::Value << component.First;
        emitter << YAML::EndMap;
    }

    void ChildRelSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto chR = node[m_ComponentSignature];
        auto& chRComp = m_Registry.AddOrGet<Component::ChildRel>(e);
        chRComp.ChildrenCount = chR["ChildrenCount"].as<U32>();
        chRComp.First = chR["First"].as<Entity>();
    }

    void ChildRelSerializer::FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map)
    {
        if (m_Registry.Has<Component::ChildRel>(e))
        {
            auto& chRComp = m_Registry.Get<Component::ChildRel>(e);
            if (chRComp.First != NULL_ENTITY) map[chRComp.First].push_back(&chRComp.First);
        }
    }

    ParentRelSerializer::ParentRelSerializer(Scene& scene)
        : ComponentSerializer<Component::ParentRel>(GetStaticSignature(), scene)
    {
    }

    void ParentRelSerializer::SerializeComponent(const Component::ParentRel& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Depth" << YAML::Value << component.Depth;
        emitter << YAML::Key << "Parent" << YAML::Value << component.Parent;
        emitter << YAML::Key << "Next" << YAML::Value << component.Next;
        emitter << YAML::Key << "Prev" << YAML::Value << component.Prev;
        emitter << YAML::EndMap;
    }

    void ParentRelSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto pR = node[m_ComponentSignature];
        auto& pRComp = m_Registry.AddOrGet<Component::ParentRel>(e);
        pRComp.Depth = pR["Depth"].as<U32>();
        pRComp.Parent = pR["Parent"].as<Entity>();
        pRComp.Next = pR["Next"].as<Entity>();
        pRComp.Prev = pR["Prev"].as<Entity>();
    }

    void ParentRelSerializer::FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map)
    {
        if (m_Registry.Has<Component::ParentRel>(e))
        {
            auto& pRComp = m_Registry.Get<Component::ParentRel>(e);
            if (pRComp.Parent != NULL_ENTITY) map[pRComp.Parent].push_back(&pRComp.Parent);
            if (pRComp.Next != NULL_ENTITY) map[pRComp.Next].push_back(&pRComp.Next);
            if (pRComp.Prev != NULL_ENTITY) map[pRComp.Prev].push_back(&pRComp.Prev);
        }
    }

    RigidBody2DSerializer::RigidBody2DSerializer(Scene& scene)
        : ComponentSerializer<Component::RigidBody2D>(GetStaticSignature(), scene)
    {
    }

    void RigidBody2DSerializer::SerializeComponent(const Component::RigidBody2D& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Type" << YAML::Value << SerializationUtils::GetRigidBodyTypeAsString(component);
        emitter << YAML::Key << "Flags" << YAML::Value << YAML::BeginSeq;
        auto flagsAsStrings = SerializationUtils::GetRigidBodyFlagsAsStrings(component);
        for (auto& flag : flagsAsStrings)
        {
            emitter << YAML::Value << flag;
        }
        emitter << YAML::EndSeq;
        emitter << YAML::Key << "Mass" << YAML::Value << (component.PhysicsBody->GetInverseMass() == 0.0f ? -1.0f : 1.0f / component.PhysicsBody->GetInverseMass());
        emitter << YAML::EndMap;
    }

    void RigidBody2DSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto rB = node[m_ComponentSignature];
        auto& pRComp = m_Registry.AddOrGet<Component::RigidBody2D>(e);
        if (!pRComp.PhysicsBody.Get()) SceneUtils::AddDefaultPhysicalRigidBody2D(m_Scene, e);
        pRComp.Type = SerializationUtils::GetRigidBodyTypeFromString(rB["Type"].as<std::string>());
        Physics::RigidBodyDef2D::BodyFlags flags = SerializationUtils::GetBodyFlagsFromStrings(
            rB["Flags"].as<std::vector<std::string>>());
        F32 mass = rB["Mass"].as<F32>();
        if (mass == -1.0f) pRComp.PhysicsBody->SetInverseMass(0.0f);
        else pRComp.PhysicsBody->SetMass(mass);
        pRComp.Flags = flags;
    }

    void RigidBody2DSerializer::AddEmptyComponentTo(Entity e)
    {
        if (!m_Registry.Has<Component::RigidBody2D>(e))
        {
            auto& rb = m_Registry.Add<Component::RigidBody2D>(e);
            SceneUtils::AddDefaultPhysicalRigidBody2D(m_Scene, e);
        }
    }

    BoxCollider2DSerializer::BoxCollider2DSerializer(Scene& scene)
        : ComponentSerializer<Component::BoxCollider2D>(GetStaticSignature(), scene)
    {
    }

    void BoxCollider2DSerializer::SerializeComponent(const Component::BoxCollider2D& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;

        emitter << YAML::Key << "PhysicsMaterial" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Restitution" << YAML::Value << component.PhysicsMaterial.Restitution;
        emitter << YAML::Key << "Friction" << YAML::Value << component.PhysicsMaterial.Friction;
        emitter << YAML::Key << "Density" << YAML::Value << component.PhysicsMaterial.Density;
        emitter << YAML::EndMap;

        emitter << YAML::Key << "Filter" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Category" << YAML::Value << component.PhysicsCollider->GetFilter().CategoryBits;
        emitter << YAML::Key << "Mask" << YAML::Value << component.PhysicsCollider->GetFilter().MaskBits;
        emitter << YAML::Key << "Group" << YAML::Value << component.PhysicsCollider->GetFilter().GroupIndex;
        emitter << YAML::EndMap;
        
        emitter << YAML::Key << "Offset" << YAML::Value << component.Offset;
        emitter << YAML::Key << "HalfSize" << YAML::Value << component.HalfSize;
        emitter << YAML::Key << "IsSensor" << YAML::Value << component.IsSensor;
        emitter << YAML::EndMap;
    }

    void BoxCollider2DSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto box = node[m_ComponentSignature];
        auto& boxComp = m_Registry.AddOrGet<Component::BoxCollider2D>(e);
        if (!boxComp.PhysicsCollider.Get()) SceneUtils::AddDefaultBoxCollider2D(m_Scene, e);
        boxComp.PhysicsMaterial.Restitution = box["PhysicsMaterial"]["Restitution"].as<F32>();
        boxComp.PhysicsMaterial.Friction = box["PhysicsMaterial"]["Friction"].as<F32>();
        boxComp.PhysicsMaterial.Density = box["PhysicsMaterial"]["Density"].as<F32>();
        Physics::Filter filter;
        filter.CategoryBits = box["Filter"]["Category"].as<U16>();
        filter.MaskBits = box["Filter"]["Mask"].as<U16>();
        filter.GroupIndex = box["Filter"]["Group"].as<I32>();
        boxComp.PhysicsCollider->SetFilter(filter);
        boxComp.Offset = box["Offset"].as<glm::vec2>();
        boxComp.HalfSize = box["HalfSize"].as<glm::vec2>();
        boxComp.IsSensor = box["IsSensor"].as<bool>();
    }

    void BoxCollider2DSerializer::AddEmptyComponentTo(Entity e)
    {
        if (!m_Registry.Has<Component::BoxCollider2D>(e))
        {
            auto& col = m_Registry.Add<Component::BoxCollider2D>(e);
            SceneUtils::AddDefaultBoxCollider2D(m_Scene, e);
        }
    }

    SpriteRendererSerializer::SpriteRendererSerializer(Scene& scene)
        : ComponentSerializer<Component::SpriteRenderer>(GetStaticSignature(), scene)
    {
    }

    void SpriteRendererSerializer::SerializeComponent(const Component::SpriteRenderer& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Texture" << YAML::Value << (component.Texture
                                                                   ? component.Texture->GetData().Name
                                                                   : "NULL");

        emitter << YAML::Key << "UV" << YAML::Value << YAML::BeginSeq;
        for (auto& uv : component.UV) emitter << uv;
        emitter << YAML::EndSeq;

        emitter << YAML::Key << "Tint" << YAML::Value << component.Tint;
        emitter << YAML::Key << "Tiling" << YAML::Value << component.Tiling;

        emitter << YAML::Key << "SortingLayer" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Name" << YAML::Value << component.SortingLayer.Name;
        emitter << YAML::Key << "Id" << YAML::Value << component.SortingLayer.Id;
        emitter << YAML::Key << "Priority" << YAML::Value << static_cast<U32>(component.SortingLayer.Priority);
        emitter << YAML::EndMap;

        emitter << YAML::Key << "OrderInLayer" << YAML::Value << component.OrderInLayer;
        emitter << YAML::Key << "FlipX" << YAML::Value << component.FlipX;
        emitter << YAML::Key << "FlipY" << YAML::Value << component.FlipY;
        emitter << YAML::EndMap;
    }

    void SpriteRendererSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto sR = node[m_ComponentSignature];
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

    void SpriteRendererSerializer::AddEmptyComponentTo(Entity e)
    {
        if (!m_Registry.Has<Component::SpriteRenderer>(e))
        {
            auto& spr = m_Registry.Add<Component::SpriteRenderer>(e);
        }
    }

    SpriteAnimationSerializer::SpriteAnimationSerializer(Scene& scene)
        : ComponentSerializer<Component::Animation>(GetStaticSignature(), scene)
    {
    }

    void SpriteAnimationSerializer::SerializeComponent(const Component::Animation& component, YAML::Emitter& emitter)
    {
        auto& spriteAnim = *component.SpriteAnimation;
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Texture" << YAML::Value << (spriteAnim.m_SpriteSheet
                                                                   ? spriteAnim.m_SpriteSheet->GetData().Name
                                                                   : "NULL");
        emitter << YAML::Key << "StartPosition" << YAML::Value << spriteAnim.m_StartPosition;
        emitter << YAML::Key << "SpriteSize" << YAML::Value << spriteAnim.m_SpriteSize;
        emitter << YAML::Key << "FrameCount" << YAML::Value << spriteAnim.m_FrameCount;
        emitter << YAML::Key << "FPSSpeed" << YAML::Value << spriteAnim.m_FpsSpeed;
        emitter << YAML::Key << "CurrentFrame" << YAML::Value << spriteAnim.m_CurrentFrame;
        emitter << YAML::Key << "TotalDuration" << YAML::Value << spriteAnim.m_TotalDuration;
        emitter << YAML::Key << "MaxDuration" << YAML::Value << spriteAnim.m_MaxDuration;

        emitter << YAML::Key << "CurrentFrameUV" << YAML::Value << YAML::BeginSeq;
        for (auto& uv : spriteAnim.m_CurrentFrameUV) emitter << uv;
        emitter << YAML::EndSeq;

        emitter << YAML::EndMap;
    }

    void SpriteAnimationSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto anim = node[m_ComponentSignature];
        auto& spriteAnimComp = m_Registry.AddOrGet<Component::Animation>(e);

        Texture* spriteSheet = (anim["Texture"].as<std::string>() == "NULL"
                                    ? nullptr
                                    : Texture::LoadTextureFromFile(anim["Texture"].as<std::string>()).get());
        glm::uvec2 startPosition = anim["StartPosition"].as<glm::uvec2>();
        glm::uvec2 spriteSize = anim["SpriteSize"].as<glm::uvec2>();
        U32 frameCount = anim["FrameCount"].as<U32>();
        U32 fpsSpeed = anim["FPSSpeed"].as<U32>();
        U32 currentFrame = anim["CurrentFrame"].as<U32>();
        F32 totalDuration = anim["TotalDuration"].as<F32>();
        F32 maxDuration = anim["MaxDuration"].as<F32>();

        if (!spriteAnimComp.SpriteAnimation)
            spriteAnimComp.SpriteAnimation = CreateRef<SpriteAnimation>(
                spriteSheet, startPosition, spriteSize, frameCount, fpsSpeed, maxDuration);
        auto& spriteAnim = *spriteAnimComp.SpriteAnimation;
            
        for (U32 i = 0; i < 4; i++) spriteAnim.m_CurrentFrameUV[i] = anim["CurrentFrameUV"][i].as<glm::vec2>();
        spriteAnim.m_CurrentFrame = currentFrame;
        spriteAnim.m_TotalDuration = totalDuration;
    }
}
