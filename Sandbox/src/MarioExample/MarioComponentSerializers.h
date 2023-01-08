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

struct MarioMenuItem
{
    enum class Type { Exit, Open };
    U32 Order{0};
    Type Type{Type::Exit};
    std::string Info{};
    std::string Title{};
};

class MarioMenuItemSerializer : public ComponentSerializer<MarioMenuItem>
{
public:
    COMPONENT_SERIALIZER_SIGNATURE(MarioMenuItem)
    MarioMenuItemSerializer(Scene& scene);
    void SerializeComponent(const MarioMenuItem& component, YAML::Emitter& emitter) override;
    void DeserializeComponent(Entity e, YAML::Node& node) override;
};