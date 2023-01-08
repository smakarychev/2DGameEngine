#include "enginepch.h"
#include "SceneSerializer.h"

#include "ComponentSerializer.h"
#include "Prefab.h"
#include "Engine/Core/Input.h"
#include "Engine/ECS/View.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneUtils.h"

namespace Engine
{
    SceneSerializer::SceneSerializer(Scene& scene)
        : m_Scene(scene)
    {
        m_ComponentSerializers.push_back(CreateRef<NameSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<LocalToWorldTransformSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<LocalToParentTransformSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<CameraSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<ChildRelSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<ParentRelSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<RigidBody2DSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<BoxCollider2DSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<SpriteRendererSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<SpriteAnimationSerializer>(m_Scene));
        m_ComponentSerializers.push_back(CreateRef<PrefabSerializer>(m_Scene));
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter emitter;
        auto& registry = m_Scene.GetRegistry();
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Scene" << YAML::Value << "Default";

        // Prefabs.
        emitter << YAML::Key << "Prefabs" << YAML::Value << YAML::BeginSeq;
        for (auto e : View<Component::Prefab>(registry))
        {
            if (!registry.Has<Component::BelongsToPrefab>(e)) SerializePrefab(e, emitter);
        }
        emitter << YAML::EndSeq;

        // Entities.
        emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        for (auto e : View<>(registry))
        {
            if (!registry.Has<Component::Prefab>(e) && !registry.Has<Component::BelongsToPrefab>(e))
                SerializeEntity(
                    e, emitter);
        }
        emitter << YAML::EndSeq;

        emitter << YAML::EndMap;
        Write(filepath, emitter);
    }

    void SceneSerializer::SerializeEntity(Entity entity, YAML::Emitter& emitter)
    {
        //ENGINE_CORE_TRACE("Serializing entity {} ({})", m_Scene.GetRegistry().Get<Component::Name>(entity).EntityName, entity.Id);
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Entity" << YAML::Value << entity.Id;
        for (auto& componentSerializer : m_ComponentSerializers)
        {
            componentSerializer->SerializeComponentOf(entity, emitter);
        }
        emitter << YAML::EndMap;
    }

    void SceneSerializer::SerializeGeneratedPrefab(const std::vector<Entity>& entities, const std::string& prefabName,
                                                   U64 prefabId)
    {
        YAML::Emitter emitter;
        auto& registry = m_Scene.GetRegistry();
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Prefab" << YAML::Value << YAML::BeginMap;
        emitter << YAML::Key << "Name" << YAML::Value << prefabName;
        emitter << YAML::Key << "Id" << YAML::Value << prefabId;
        emitter << YAML::EndMap;

        // Prefabs.
        emitter << YAML::Key << "Prefabs" << YAML::Value << YAML::BeginSeq;
        for (auto e : entities)
        {
            if (registry.Has<Component::Prefab>(e)) SerializePrefab(e, emitter);
        }
        emitter << YAML::EndSeq;

        // Entities.
        emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        for (auto e : entities)
        {
            if (!registry.Has<Component::Prefab>(e) && !registry.Has<Component::BelongsToPrefab>(e)) SerializeEntity(e, emitter);
        }
        emitter << YAML::EndSeq;

        emitter << YAML::EndMap;
        Write(prefabName, emitter);
    }

    void SceneSerializer::SerializePrefab(Entity prefab, YAML::Emitter& emitter)
    {
        auto& prefabComp = m_Scene.GetRegistry().Get<Component::Prefab>(prefab);
        //ENGINE_CORE_TRACE("Serializing prefab {} ({})", prefabComp.Name, prefabComp.Id);
        emitter << YAML::BeginMap;
        emitter << YAML::Key << "Prefab" << YAML::Value << prefab.Id;
        for (auto& componentSerializer : m_ComponentSerializers)
        {
            componentSerializer->SerializeComponentOf(prefab, emitter);
        }
        emitter << YAML::EndMap;
    }

    void SceneSerializer::OnImguiPayloadAccept(const ImGuiPayload* payload, const PayloadAdditionalInfo& pai)
    {
        auto& registry = m_Scene.GetRegistry();
        std::string payloadFile = std::string((char*)payload->Data);
        SceneUtils::AssetPayloadType assetType = SceneUtils::GetAssetPayloadTypeFromString(payloadFile);
        switch (assetType)
        {
        case SceneUtils::AssetPayloadType::Scene:
        {
            m_Scene.Open(std::filesystem::path(payloadFile).stem().string());
            break;        
        }
        
        case SceneUtils::AssetPayloadType::Prefab:
        {
            Entity prefab = AddPrefabToScene(payloadFile);
            registry.Get<Component::LocalToWorldTransform2D>(prefab).Position = pai.MousePos;
            m_Scene.OnSceneGlobalUpdate(prefab);
            break;
        }
            
        case SceneUtils::AssetPayloadType::Image:
        {
            Entity entityUnderMouse = pai.EntityUnderMouse;
            if (entityUnderMouse != NULL_ENTITY)
            {
                if (registry.Has<Component::SpriteRenderer>(entityUnderMouse))
                {
                    registry.Get<Component::SpriteRenderer>(entityUnderMouse).Texture = Texture::LoadTextureFromFile(payloadFile).get();
                }
            }
            break;
        }
        case SceneUtils::AssetPayloadType::Font:
        case SceneUtils::AssetPayloadType::Unknown:
            break;
        }
    }

    std::vector<Entity> SceneSerializer::Deserialize(const std::string& filePath)
    {
        YAML::Node nodes = YAML::LoadFile(filePath);

        std::unordered_map<Entity, Entity> deserializedToTrueEntityMap;
        std::unordered_map<Entity, std::vector<Entity*>> entityRelationsMap;

        auto prefabs = nodes["Prefabs"];
        auto addedFromPrefabs = DeserializePrefabs(prefabs, deserializedToTrueEntityMap, entityRelationsMap);
        auto entities = nodes["Entities"];
        auto addedFromEntities = DeserializeEntities(entities, deserializedToTrueEntityMap, entityRelationsMap);

        FixRelations(deserializedToTrueEntityMap, entityRelationsMap);

        std::vector<Entity> addedEntities = addedFromPrefabs;
        addedEntities.reserve(addedEntities.size() + addedFromEntities.size());
        addedEntities.insert(addedEntities.end(), addedFromEntities.begin(), addedFromEntities.end());
        return addedEntities;
    }

    std::vector<Entity> SceneSerializer::DeserializeEntities(YAML::Node& entities,
                                                             std::unordered_map<Entity, Entity>& deserializedToTrue,
                                                             std::unordered_map<Entity, std::vector<Entity*>>&
                                                             entityRelations)
    {
        std::vector<Entity> addedEntities;
        for (auto e : entities)
        {
            addedEntities.push_back(DeserializeEntity(e, deserializedToTrue, entityRelations));
        }
        return addedEntities;
    }

    std::vector<Entity> SceneSerializer::DeserializePrefabs(YAML::Node& prefabs,
                                                            std::unordered_map<Entity, Entity>& deserializedToTrue,
                                                            std::unordered_map<Entity, std::vector<Entity*>>&
                                                            entityRelations)
    {
        auto& registry = m_Scene.GetRegistry();
        std::vector<Entity> addedEntities;
        // Each prefab is essentially a scene.
        for (auto prefab : prefabs)
        {
            auto addedFromPrefab = Deserialize(prefab["PrefabDetails"]["Name"].as<std::string>());
            addedEntities.reserve(addedEntities.size() + addedFromPrefab.size() + 1);
            addedEntities.insert(addedEntities.end(), addedFromPrefab.begin(), addedFromPrefab.end());
            // Now just deserialize prefab `entity`, and add top level entity from prefab `scene` as it's child.
            Entity prefabEntity = DeserializeEntityExcludeComponents(prefab,
                deserializedToTrue,
                entityRelations,
                {ChildRelSerializer::GetStaticSignature()},
                "Prefab");
            // Entities, deserialized earlier, have 1 common parent (otherwise prefab is ill-formed).
            Entity topEntity = SceneUtils::FindTopOfTree(addedFromPrefab.front(), registry);
            SceneUtils::AddChild(m_Scene, prefabEntity, topEntity, false, SceneUtils::LocalTransformPolicy::SameAsWorld);
            for (auto e : addedFromPrefab)
            {
                if (!registry.Has<Component::BelongsToPrefab>(e))
                {
                    auto& belongsToPrefab = registry.Add<Component::BelongsToPrefab>(e);
                    belongsToPrefab.PrefabId = registry.Get<Component::Prefab>(prefabEntity).Id;
                }
            }
            addedEntities.push_back(prefabEntity);
        }
        return addedEntities;
    }

    Entity SceneSerializer::AddPrefabToScene(const std::string& prefabPath)
    {
        auto& registry = m_Scene.GetRegistry();
        auto prefabEntities = Deserialize(prefabPath);
        YAML::Node nodes = YAML::LoadFile(prefabPath);
        std::string prefabName = nodes["Prefab"]["Name"].as<std::string>();
        U64 prefabId = nodes["Prefab"]["Id"].as<U64>();
        std::vector<Entity> addToPrefab;
        addToPrefab.reserve(prefabEntities.size());
        for (auto e : prefabEntities)
        {
            if (!registry.Has<Component::BelongsToPrefab>(e)) addToPrefab.push_back(e);
        }
        return PrefabUtils::CreatePrefab(prefabName, prefabId, addToPrefab, m_Scene);
    }

    Entity SceneSerializer::DeserializeEntity(YAML::Node& entity,
                                              std::unordered_map<Entity, Entity>& deserializedToTrue,
                                              std::unordered_map<Entity, std::vector<Entity*>>& entityRelations,
                                              const std::string& entityTag)
    {
        std::string entityName = entity["Name"]["Name"].as<std::string>();
        auto realE = m_Scene.GetRegistry().CreateEntity(entityName);

        Entity deserealizedEntityId = entity[entityTag].as<Entity>();
        deserializedToTrue[deserealizedEntityId] = realE;
        //ENGINE_CORE_TRACE("Deserializing entity {} ({}, real id: {})", entityName, deserealizedEntityId.Id, realE);
        for (auto& componentSerializer : m_ComponentSerializers)
        {
            componentSerializer->DeserializeComponentOf(realE, entity);
            componentSerializer->FillEntityRelationsMap(realE, entityRelations);
        }
        return realE;
    }

    Entity SceneSerializer::DeserializeEntityExcludeComponents(YAML::Node& entity,
                                                               std::unordered_map<Entity, Entity>& deserializedToTrue,
                                                               std::unordered_map<Entity, std::vector<Entity*>>&
                                                               entityRelations,
                                                               const std::vector<ComponentSignature>&
                                                               excludingSignatures, const std::string& entityTag)
    {
        std::string entityName = entity["Name"]["Name"].as<std::string>();
        auto realE = m_Scene.GetRegistry().CreateEntity(entityName);

        Entity deserealizedEntityId = entity[entityTag].as<Entity>();
        deserializedToTrue[deserealizedEntityId] = realE;
        //ENGINE_CORE_TRACE("Deserializing entity {} ({}, real id: {})", entityName, deserealizedEntityId.Id, realE);
        for (auto& componentSerializer : m_ComponentSerializers)
        {
            if (ShouldNotDeserialize(*componentSerializer, excludingSignatures)) continue;
            componentSerializer->DeserializeComponentOf(realE, entity);
            componentSerializer->FillEntityRelationsMap(realE, entityRelations);
        }
        return realE;
    }

    void SceneSerializer::FixRelations(std::unordered_map<Entity, Entity>& deserializedToTrue,
                                       std::unordered_map<Entity, std::vector<Entity*>>& entityRelations)
    {
        // Fixing entities (which behave like pointers).
        for (auto&& [desEntity, entityLocations] : entityRelations)
        {
            for (auto* loc : entityLocations)
            {
                *loc = deserializedToTrue[desEntity];
            }
        }
    }

    bool SceneSerializer::ShouldNotDeserialize(ComponentSerializerBase& compSerializer,
                                               const std::vector<ComponentSignature>& excludingSignatures)
    {
        auto it = std::ranges::find(excludingSignatures, compSerializer.GetSignature());
        return it != excludingSignatures.end();
    }

    void SceneSerializer::Write(const std::string& filepath, YAML::Emitter& emitter)
    {
        std::ofstream scene(filepath);
        scene << emitter.c_str();
    }

    void SceneSerializer::SetComponentSerializers(const std::vector<Ref<ComponentSerializerBase>>& compSerializers)
    {
        m_ComponentSerializers = compSerializers;
    }

    const std::vector<Ref<ComponentSerializerBase>>& SceneSerializer::GetComponentSerializers() const
    {
        return m_ComponentSerializers;
    }

    void SceneSerializer::OnCopyPaste()
    {
        auto& registry = m_Scene.GetRegistry();
        if (Input::GetKey(Key::LeftControl))
        {
            bool copy = Input::GetKeyDown(Key::C);
            bool paste = Input::GetKeyDown(Key::V);
            Entity active = m_Scene.GetScenePanels().GetActiveEntity();
            if (copy)
            {
                if (!registry.Has<Component::Prefab>(active)) return;
                std::vector<Entity> entitiesToSerialize;
                SceneUtils::TraverseTreeAndApply(active, registry, [&](Entity e)
                {
                   entitiesToSerialize.push_back(e); 
                });
                SerializeGeneratedPrefab(entitiesToSerialize, m_CopyPasteInfo.SaveFileName, PrefabUtils::GeneratePrefabId());
                m_CopyPasteInfo.HasSaved = true;
            }
            else if (paste)
            {
                if (!m_CopyPasteInfo.HasSaved) return;
                // The only thing saved `prefab` has is reference to some other prefab, we just need to add that prefab to scene.
                YAML::Node nodes = YAML::LoadFile(m_CopyPasteInfo.SaveFileName);
                std::string prefabPath = nodes["Prefabs"][0]["PrefabDetails"]["Name"].as<std::string>();
                Entity prefab = AddPrefabToScene(prefabPath);
                glm::vec2 mousePos = SceneUtils::GetMousePosition(m_Scene);
                mousePos = Math::Align(mousePos, {1.0f,1.0f});
                registry.Get<Component::LocalToWorldTransform2D>(prefab).Position = mousePos;
                m_Scene.OnSceneGlobalUpdate(prefab);
            }
        }
    }
}
