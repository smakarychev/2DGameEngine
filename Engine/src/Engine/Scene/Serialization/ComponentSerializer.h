#pragma once

#include "Engine/ECS/EntityId.h"
#include "Engine/ECS/Registry.h"
#include "SerializationUtils.h"
#include "yaml-cpp/yaml.h"

namespace Engine
{
    class Scene;

    // TODO: replace with real clang-libtooling reflection in the nearest future.
    class ComponentSerializerBase
    {
    public:
        using ComponentSignature = std::string;

    public:
        ComponentSerializerBase(const ComponentSignature& signature, Scene& scene);
        virtual ~ComponentSerializerBase() = default;
        virtual void SerializeComponentOf(Entity e, YAML::Emitter& emitter) = 0;
        virtual void DeserializeComponentOf(Entity e, YAML::Node& node) = 0;

        virtual void FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map)
        {
        }

        const ComponentSignature& GetSignature() const { return m_ComponentSignature; }

    protected:
        ComponentSignature m_ComponentSignature{};
        Scene& m_Scene;
        Registry& m_Registry;
    };

#define COMPONENT_SERIALIZER_SIGNATURE(x) static constexpr Engine::ComponentSerializerBase::ComponentSignature GetStaticSingature() { return x; }

    template <typename T>
    class ComponentSerializer : public ComponentSerializerBase
    {
    public:
        ComponentSerializer(const ComponentSignature& signature, Scene& scene);
        void SerializeComponentOf(Entity e, YAML::Emitter& emitter) override;
        void DeserializeComponentOf(Entity e, YAML::Node& node) override;

    protected:
        virtual void SerializeComponent(const T& component, YAML::Emitter& emitter) = 0;
        virtual void DeserializeComponent(Entity e, YAML::Node& node) = 0;
    };


    template <typename T>
    ComponentSerializer<T>::ComponentSerializer(const ComponentSignature& signature, Scene& scene)
        : ComponentSerializerBase(signature, scene)
    {
    }

    template <typename T>
    void ComponentSerializer<T>::SerializeComponentOf(Entity e, YAML::Emitter& emitter)
    {
        if (m_Registry.Has<T>(e))
        {
            ENGINE_CORE_TRACE("\t\t\tSerializing {} component", m_ComponentSignature);
            SerializeComponent(m_Registry.Get<T>(e), emitter);
        }
    }

    template <typename T>
    void ComponentSerializer<T>::DeserializeComponentOf(Entity e, YAML::Node& node)
    {
        if (node[m_ComponentSignature])
        {
            ENGINE_CORE_TRACE("\t\t\tDeserializing {} component", m_ComponentSignature);
            DeserializeComponent(e, node);
        }
    }

    class NameSerializer : public ComponentSerializer<Component::Name>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("Name")
        NameSerializer(Scene& scene);
        void SerializeComponent(const Component::Name& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class TagSerializer : public ComponentSerializer<Component::Tag>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("Tag")
        TagSerializer(Scene& scene);
        void SerializeComponent(const Component::Tag& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class LocalToWorldTransformSerializer : public ComponentSerializer<Component::LocalToWorldTransform2D>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("LocalToWorldTransform")
        LocalToWorldTransformSerializer(Scene& scene);
        void SerializeComponent(const Component::LocalToWorldTransform2D& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class LocalToParentTransformSerializer : public ComponentSerializer<Component::LocalToParentTransform2D>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("LocalToParentTransform")
        LocalToParentTransformSerializer(Scene& scene);
        void SerializeComponent(const Component::LocalToParentTransform2D& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class CameraSerializer : public ComponentSerializer<Component::Camera>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("Camera")
        CameraSerializer(Scene& scene);
        void SerializeComponent(const Component::Camera& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class ChildRelSerializer : public ComponentSerializer<Component::ChildRel>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("ChildRel")
        ChildRelSerializer(Scene& scene);
        void SerializeComponent(const Component::ChildRel& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
        void FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map) override;
    };

    class ParentRelSerializer : public ComponentSerializer<Component::ParentRel>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("ParentRel")
        ParentRelSerializer(Scene& scene);
        void SerializeComponent(const Component::ParentRel& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
        void FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map) override;
    };

    class RigidBody2DSerializer : public ComponentSerializer<Component::RigidBody2D>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("RigidBody2D")
        RigidBody2DSerializer(Scene& scene);
        void SerializeComponent(const Component::RigidBody2D& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class BoxCollider2DSerializer : public ComponentSerializer<Component::BoxCollider2D>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("BoxCollider2D")
        BoxCollider2DSerializer(Scene& scene);
        void SerializeComponent(const Component::BoxCollider2D& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class SpriteRendererSerializer : public ComponentSerializer<Component::SpriteRenderer>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("SpriteRenderer")
        SpriteRendererSerializer(Scene& scene);
        void SerializeComponent(const Component::SpriteRenderer& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class SpriteAnimationSerializer : public ComponentSerializer<Component::Animation>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("Animation")
        SpriteAnimationSerializer(Scene& scene);
        void SerializeComponent(const Component::Animation& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };
}
