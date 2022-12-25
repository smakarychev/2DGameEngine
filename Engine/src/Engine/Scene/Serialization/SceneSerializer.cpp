#include "enginepch.h"
#include "SceneSerializer.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Memory/MemoryManager.h"

namespace Engine
{
    SceneSerializer::SceneSerializer()
    {
    }

    void SceneSerializer::Serialize(Scene& scene, const std::string& filePath)
    {
    }

    SceneDesc SceneSerializer::Deserialize(const std::string& filePath)
    {
        return SceneDesc();
    }
}
