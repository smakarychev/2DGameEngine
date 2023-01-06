﻿#include "MarioComponentSerializers.h"

SensorsSerializer::SensorsSerializer(Scene& scene)
    : ComponentSerializer<Component::Sensors>(GetStaticSignature(), scene)
{
}

void SensorsSerializer::SerializeComponent(const Component::Sensors& component, YAML::Emitter& emitter)
{
    emitter << YAML::Key << m_ComponentSignature;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "Top" << YAML::Value << component.Top;
    emitter << YAML::Key << "Bottom" << YAML::Value << component.Bottom;
    emitter << YAML::Key << "Left" << YAML::Value << component.Left;
    emitter << YAML::Key << "Right" << YAML::Value << component.Right;
    emitter << YAML::EndMap;
}

void SensorsSerializer::DeserializeComponent(Entity e, YAML::Node& node)
{
    auto sens = node[m_ComponentSignature];
    auto& sensorsComponent = m_Registry.AddOrGet<Component::Sensors>(e);
    sensorsComponent.Top = sens["Top"].as<Entity>();
    sensorsComponent.Bottom = sens["Bottom"].as<Entity>();
    sensorsComponent.Left = sens["Left"].as<Entity>();
    sensorsComponent.Right = sens["Right"].as<Entity>();
}

void SensorsSerializer::FillEntityRelationsMap(Entity e, std::unordered_map<Entity, std::vector<Entity*>>& map)
{
    if (m_Registry.Has<Component::Sensors>(e))
    {
        auto& sensorsComponent = m_Registry.Get<Component::Sensors>(e);
        if (sensorsComponent.Top != NULL_ENTITY) map[sensorsComponent.Top].push_back(&sensorsComponent.Top);
        if (sensorsComponent.Bottom != NULL_ENTITY) map[sensorsComponent.Bottom].push_back(&sensorsComponent.Bottom);
        if (sensorsComponent.Left != NULL_ENTITY) map[sensorsComponent.Left].push_back(&sensorsComponent.Left);
        if (sensorsComponent.Right != NULL_ENTITY) map[sensorsComponent.Right].push_back(&sensorsComponent.Right);
    }
}