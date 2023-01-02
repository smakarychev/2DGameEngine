#pragma once

#include "Engine.h"

using namespace Engine::Types;
using namespace Engine;

class SensorsSerializer : public ComponentSerializer<Component::Sensors>
{
public:
    COMPONENT_SERIALIZER_SIGNATURE(Sensors)
    SensorsSerializer(Scene& scene);
    void SerializeComponent(const Component::Sensors& component, YAML::Emitter& emitter) override;
    void DeserializeComponent(Entity e, YAML::Node& node) override;
    void FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map) override;
};

class MarioInputSerializer : public ComponentSerializer<Component::MarioInput>
{
public:
    COMPONENT_SERIALIZER_SIGNATURE(MarioInput)
    MarioInputSerializer(Scene& scene);
    void SerializeComponent(const Component::MarioInput& component, YAML::Emitter& emitter) override;
    void DeserializeComponent(Entity e, YAML::Node& node) override;
};

class MarioStateSerializer : public ComponentSerializer<Component::MarioState>
{
public:
    COMPONENT_SERIALIZER_SIGNATURE(MarioState)
    MarioStateSerializer(Scene& scene);
    void SerializeComponent(const Component::MarioState& component, YAML::Emitter& emitter) override;
    void DeserializeComponent(Entity e, YAML::Node& node) override;
};

class CollisionCallbackSerializer : public ComponentSerializer<Component::CollisionCallback>
{
public:
    COMPONENT_SERIALIZER_SIGNATURE(CollisionCallback)
    CollisionCallbackSerializer(Scene& scene);
    void SerializeComponent(const Component::CollisionCallback& component, YAML::Emitter& emitter) override;
    void DeserializeComponent(Entity e, YAML::Node& node) override;
};