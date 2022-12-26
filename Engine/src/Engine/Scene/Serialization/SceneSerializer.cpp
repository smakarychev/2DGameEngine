#include "enginepch.h"
#include "SceneSerializer.h"

#include "ComponentSerializer.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    SceneSerializer::SceneSerializer(Scene& scene)
        : m_Scene(scene)
    {
        m_ComponentSerializers.push_back(CreateRef<TagSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<LocalToWorldTransformSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<LocalToParentTransformSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<ChildRelSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<ParentRelSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<RigidBody2DSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<BoxCollider2DSerializer>(m_Emitter, m_Scene));
        m_ComponentSerializers.push_back(CreateRef<SpriteRendererSerializer>(m_Emitter, m_Scene));
    }

    void SceneSerializer::Serialize(const std::string& filePath)
    {
        auto& registry = m_Scene.GetRegistry();
        m_Emitter << YAML::BeginMap;
        m_Emitter << YAML::Key << "Scene" << YAML::Value << "Default";
        m_Emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        for (auto e : View<>(registry))
        {
            m_Emitter << YAML::BeginMap;
            m_Emitter << YAML::Key << "Entity" << YAML::Value << e.Id;
            for (auto& componentSerializer : m_ComponentSerializers)
            {
                componentSerializer->SerializeComponentOf(e);
            }
            m_Emitter << YAML::EndMap;
        }
        m_Emitter << YAML::EndSeq;
        m_Emitter << YAML::EndMap;
        std::ofstream scene(filePath);
        scene << m_Emitter.c_str();
    }

    void SceneSerializer::Deserialize(const std::string& filePath)
    {
        //TODO: keep id map to resolve entity-entity relations.
        
        m_Nodes = YAML::LoadFile(filePath);
        auto entities = m_Nodes["Entities"];
        for (auto e : entities)
        {
            std::string entityName = e["Tag"]["Tag"].as<std::string>();
            auto realE = m_Scene.GetRegistry().CreateEntity(entityName);
            for (auto& componentSerializer : m_ComponentSerializers)
            {
                componentSerializer->DeserializeComponentOf(realE, e);
            }
        }
    }
}
