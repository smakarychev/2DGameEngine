#include "enginepch.h"
#include "Prefab.h"

#include "SceneSerializer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneUtils.h"

namespace Engine
{
    void PrefabUtils::CreatePrefabFromEntity(Entity entity, const std::string& prefabName, Scene& scene)
    {
        std::string fullName = "assets/prefabs/" + prefabName + ".prefab";
        U64 prefabId = PrefabUtils::GeneratePrefabId();
        ENGINE_CORE_TRACE("Creating prefab: {}, generated id: {}", fullName, prefabId);
        SceneSerializer serializer(scene);
        serializer.SetComponentSerializers(scene.GetSerializer().GetComponentSerializers());

        auto& registry = scene.GetRegistry();
        std::vector<Entity> prefabEntities;
        SceneUtils::TraverseTreeAndApply(entity, registry, [&](Entity e)
        {
            if (!registry.Has<Component::BelongsToPrefab>(e))
            {
                prefabEntities.push_back(e);
            }
        });
        serializer.SerializeGeneratedPrefab(prefabEntities, fullName, prefabId);
        // Add new `Prefab` entity - when serializing, only this entity will be serialized.
        auto&& [prefab, tf] = SceneUtils::AddDefaultEntity(scene, "prefab-" + prefabName);
        auto& prefabComp = registry.Add<Component::Prefab>(prefab);
        prefabComp.Id = prefabId;
        prefabComp.Name = fullName;
        SceneUtils::AddChild(scene, prefab, entity);
        // Add `BelongsToPrefab` component to every entity in prefab.
        for (auto e : prefabEntities)
        {
            auto& belongsToPrefab = registry.Add<Component::BelongsToPrefab>(e);
            belongsToPrefab.PrefabId = prefabId;
        }
    }

    PrefabSerializer::PrefabSerializer(Scene& scene)
        : ComponentSerializer(GetStaticSingature(), scene)
    {
    }

    void PrefabSerializer::SerializeComponent(const Component::Prefab& component, YAML::Emitter& emitter)
    {
        emitter << YAML::Key << m_ComponentSignature;
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Name" << YAML::Value << component.Name;
        emitter << YAML::Key << "Id" << YAML::Value << component.Id;
        emitter << YAML::EndMap;
    }

    void PrefabSerializer::DeserializeComponent(Entity e, YAML::Node& node)
    {
        auto prefab = node[m_ComponentSignature];
        auto& prefabComponent = m_Registry.AddOrGet<Component::Prefab>(e);
        prefabComponent.Name = prefab["Name"].as<std::string>();
        prefabComponent.Id = prefab["Id"].as<U64>();
    }

}
