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
        ComponentSerializerBase(const ComponentSignature& signature, YAML::Emitter& emitter, Scene& scene);
        virtual ~ComponentSerializerBase() = default;
        virtual void SerializeComponentOf(Entity e) = 0;
        virtual void DeserializeComponentOf(Entity e, YAML::Node& node) = 0;

    protected:
        ComponentSignature m_ComponentSignature{};
        YAML::Emitter& m_Emitter;
        Scene& m_Scene;
        Registry& m_Registry;
    };

    template <typename T>
    class ComponentSerializer : public ComponentSerializerBase
    {
    public:
        ComponentSerializer(const ComponentSignature& signature, YAML::Emitter& emitter, Scene& scene);
        void SerializeComponentOf(Entity e) override;
        void DeserializeComponentOf(Entity e, YAML::Node& node) override;

    protected:
        virtual void SerializeComponent(const T& component) = 0;
        virtual void DeserializeComponent(Entity e, YAML::Node& node) = 0;
    };


    template <typename T>
    ComponentSerializer<T>::ComponentSerializer(const ComponentSignature& signature, YAML::Emitter& emitter,
                                                 Scene& scene)
        : ComponentSerializerBase(signature, emitter, scene)
    {
    }

    template <typename T>
    void ComponentSerializer<T>::SerializeComponentOf(Entity e)
    {
        if (m_Registry.Has<T>(e)) SerializeComponent(m_Registry.Get<T>(e));
    }

    template <typename T>
    void ComponentSerializer<T>::DeserializeComponentOf(Entity e, YAML::Node& node)
    {
        return DeserializeComponent(e, node);
    }

    class TagSerializer : public ComponentSerializer<Component::Tag>
    {
    public:
        TagSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::Tag& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class LocalToWorldTransformSerializer : public ComponentSerializer<Component::LocalToWorldTransform2D>
    {
    public:
        LocalToWorldTransformSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::LocalToWorldTransform2D& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class LocalToParentTransformSerializer : public ComponentSerializer<Component::LocalToParentTransform2D>
    {
    public:
        LocalToParentTransformSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::LocalToParentTransform2D& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class ChildRelSerializer : public ComponentSerializer<Component::ChildRel>
    {
    public:
        ChildRelSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::ChildRel& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class ParentRelSerializer : public ComponentSerializer<Component::ParentRel>
    {
    public:
        ParentRelSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::ParentRel& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class RigidBody2DSerializer : public ComponentSerializer<Component::RigidBody2D>
    {
    public:
        RigidBody2DSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::RigidBody2D& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class BoxCollider2DSerializer : public ComponentSerializer<Component::BoxCollider2D>
    {
    public:
        BoxCollider2DSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::BoxCollider2D& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };

    class SpriteRendererSerializer : public ComponentSerializer<Component::SpriteRenderer>
    {
    public:
        SpriteRendererSerializer(YAML::Emitter& emitter,  Scene& scene);
        void SerializeComponent(const Component::SpriteRenderer& component) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };
    
}
