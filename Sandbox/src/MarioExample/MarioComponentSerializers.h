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

