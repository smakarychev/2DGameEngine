#pragma once
#include "ComponentSerializer.h"
#include "Engine/Core/Core.h"

namespace Engine
{
    class ComponentSerializerBase;
}

namespace Engine
{
    class Scene;
    
    class SceneSerializer
    {
    public:
        SceneSerializer(Scene& scene);
        void Serialize(const std::string& filePath);
        void Deserialize(const std::string& filePath);
    private:
        std::vector<Ref<ComponentSerializerBase>> m_ComponentSerializers;
        Scene& m_Scene;
        YAML::Emitter m_Emitter;
        YAML::Node m_Nodes;
    };
}
