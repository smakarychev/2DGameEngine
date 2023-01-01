#pragma once
#include "ComponentSerializer.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/Random.h"

namespace Engine
{
    using namespace Types;

    namespace Component
    {
        struct Prefab
        {
            std::string Name{"Default"};
            U64 Id{};
        };

        struct BelongsToPrefab
        {
            U64 PrefabId{};
        };
    }

    namespace PrefabUtils
    {
        inline U64 GeneratePrefabId() { return Random::UInt64(); }
        void CreatePrefabFromEntity(Entity entity, const std::string& prefabName, Scene& scene);
        Entity CreatePrefab(const std::string& prefabName, U64 prefabId, const std::vector<Entity>& entities,
                            Scene& scene);
        Entity CreatePrefab(const std::string& prefabName, U64 prefabId, Entity topLevelEntity,
                            const std::vector<Entity>& entities, Scene& scene);
    }

    class PrefabSerializer : public ComponentSerializer<Component::Prefab>
    {
    public:
        COMPONENT_SERIALIZER_SIGNATURE("PrefabDetails")
        PrefabSerializer(Scene& scene);
        void SerializeComponent(const Component::Prefab& component, YAML::Emitter& emitter) override;
        void DeserializeComponent(Entity e, YAML::Node& node) override;
    };
}
