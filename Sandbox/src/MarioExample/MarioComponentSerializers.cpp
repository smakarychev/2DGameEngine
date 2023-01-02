#include "MarioComponentSerializers.h"

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

MarioInputSerializer::MarioInputSerializer(Scene& scene)
    : ComponentSerializer<Component::MarioInput>(GetStaticSignature(), scene)
{
}

void MarioInputSerializer::SerializeComponent(const Component::MarioInput& component, YAML::Emitter& emitter)
{
    emitter << YAML::Key << m_ComponentSignature;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "CanJump" << YAML::Value << component.CanJump;
    emitter << YAML::Key << "Jump" << YAML::Value << component.Jump;
    emitter << YAML::Key << "Left" << YAML::Value << component.Left;
    emitter << YAML::Key << "Right" << YAML::Value << component.Right;
    emitter << YAML::Key << "None" << YAML::Value << component.None;
    emitter << YAML::EndMap;
}

void MarioInputSerializer::DeserializeComponent(Entity e, YAML::Node& node)
{
    auto input = node[m_ComponentSignature];
    auto& inputComponent = m_Registry.AddOrGet<Component::MarioInput>(e);
    inputComponent.CanJump = input["CanJump"].as<bool>();
    inputComponent.Jump = input["Jump"].as<bool>();
    inputComponent.Left = input["Left"].as<bool>();
    inputComponent.Right = input["Right"].as<bool>();
    inputComponent.None = input["None"].as<bool>();
}

MarioStateSerializer::MarioStateSerializer(Scene& scene)
    : ComponentSerializer<Component::MarioState>(GetStaticSignature(), scene)
{
}

void MarioStateSerializer::SerializeComponent(const Component::MarioState& component, YAML::Emitter& emitter)
{
    emitter << YAML::Key << m_ComponentSignature;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "IsInMidAir" << YAML::Value << component.IsInMidAir;
    emitter << YAML::Key << "IsInFreeFall" << YAML::Value << component.IsInFreeFall;
    emitter << YAML::Key << "IsMovingLeft" << YAML::Value << component.IsMovingLeft;
    emitter << YAML::Key << "IsMovingRight" << YAML::Value << component.IsMovingRight;
    emitter << YAML::EndMap;
}

void MarioStateSerializer::DeserializeComponent(Entity e, YAML::Node& node)
{
    auto state = node[m_ComponentSignature];
    auto& stateComponent = m_Registry.AddOrGet<Component::MarioState>(e);
    stateComponent.IsInMidAir = state["IsInMidAir"].as<bool>();
    stateComponent.IsInFreeFall = state["IsInFreeFall"].as<bool>();
    stateComponent.IsMovingLeft = state["IsMovingLeft"].as<bool>();
    stateComponent.IsMovingRight = state["IsMovingRight"].as<bool>();
}

CollisionCallbackSerializer::CollisionCallbackSerializer(Scene& scene)
    : ComponentSerializer<Component::CollisionCallback>(GetStaticSignature(), scene)
{
}

void CollisionCallbackSerializer::SerializeComponent(const Component::CollisionCallback& component, YAML::Emitter& emitter)
{
    emitter << YAML::Key << m_ComponentSignature;
    emitter << YAML::BeginMap;
    emitter << YAML::Key << "SensorCallbackIndex" << YAML::Value << component.SensorCallbackIndex;
    emitter << YAML::EndMap;
}

void CollisionCallbackSerializer::DeserializeComponent(Entity e, YAML::Node& node)
{
    auto sCallback = node[m_ComponentSignature];
    auto& sCallbackComponent = m_Registry.AddOrGet<Component::CollisionCallback>(e);
    sCallbackComponent.SensorCallbackIndex = sCallback["SensorCallbackIndex"].as<I32>();
}
