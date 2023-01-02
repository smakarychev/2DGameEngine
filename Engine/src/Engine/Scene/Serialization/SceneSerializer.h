#pragma once
#include "ComponentSerializer.h"
#include "Engine/Core/Core.h"

struct ImGuiPayload;

namespace Engine
{
    class ComponentSerializerBase;
}

namespace Engine
{
    class Scene;

    class SceneSerializer
    {
        using ComponentSignature = ComponentSerializerBase::ComponentSignature;
    public:
        struct PayloadAdditionalInfo
        {
            glm::vec2 MousePos{0.0f, 0.0f};
            Entity EntityUnderMouse{NULL_ENTITY};
        };
        struct CopyPasteInfo
        {
            bool HasSaved{false};
            std::string SaveFileName{"assets/prefabs/temp.prefab"};
        };
    public:
        SceneSerializer(Scene& scene);
        void Serialize(const std::string& filepath);
        std::vector<Entity> Deserialize(const std::string& filePath);
        void SerializeEntity(Entity entity, YAML::Emitter& emitter);
        Entity DeserializeEntity(YAML::Node& entity, std::unordered_map<Entity, Entity>& deserializedToTrue,
                                 std::unordered_map<Entity, std::vector<Entity*>>& entityRelations,
                                 const std::string& entityTag = "Entity");
        Entity DeserializeEntityExcludeComponents(YAML::Node& entity,
                                                  std::unordered_map<Entity, Entity>& deserializedToTrue,
                                                  std::unordered_map<Entity, std::vector<Entity*>>& entityRelations,
                                                  const std::vector<ComponentSignature>&
                                                  excludingSignatures,
                                                  const std::string& entityTag = "Entity");

        void SerializeGeneratedPrefab(const std::vector<Entity>& entities, const std::string& prefabName, U64 prefabId);
        void SerializePrefab(Entity prefab, YAML::Emitter& emitter);

        void OnImguiPayloadAccept(const ImGuiPayload* payload, const PayloadAdditionalInfo& pai);

        void Write(const std::string& filepath, YAML::Emitter& emitter);

        template <typename T>
        void AddComponentSerializer();

        void SetComponentSerializers(const std::vector<Ref<ComponentSerializerBase>>& compSerializers);
        const std::vector<Ref<ComponentSerializerBase>>& GetComponentSerializers() const;

        void OnCopyPaste();

    private:
        std::vector<Entity> DeserializeEntities(YAML::Node& entities,
                                                std::unordered_map<Entity, Entity>& deserializedToTrue,
                                                std::unordered_map<Entity, std::vector<Entity*>>& entityRelations);
        std::vector<Entity> DeserializePrefabs(YAML::Node& prefabs,
                                               std::unordered_map<Entity, Entity>& deserializedToTrue,
                                               std::unordered_map<Entity, std::vector<Entity*>>& entityRelations);
        Entity AddPrefabToScene(const std::string& prefabPath);
        void FixRelations(std::unordered_map<Entity, Entity>& deserializedToTrue,
                          std::unordered_map<Entity, std::vector<Entity*>>& entityRelations);

        bool ShouldNotDeserialize(ComponentSerializerBase& compSerializer,
                                  const std::vector<ComponentSignature>& excludingSignatures);

    private:
        CopyPasteInfo m_CopyPasteInfo{};
        std::vector<Ref<ComponentSerializerBase>> m_ComponentSerializers;
        Scene& m_Scene;
    };

    template <typename T>
    void SceneSerializer::AddComponentSerializer()
    {
        auto newSerializer = CreateRef<T>(m_Scene);
        auto it = std::ranges::find_if(m_ComponentSerializers, [&](auto& el)
        {
            return el->GetSignature() == newSerializer->GetSignature();
        });
        if (it != m_ComponentSerializers.end()) return;
        m_ComponentSerializers.push_back(newSerializer);
    }
}
