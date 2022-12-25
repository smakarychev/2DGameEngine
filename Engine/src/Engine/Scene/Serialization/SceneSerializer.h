#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
    class Scene;

    struct SceneDesc
    {
        
    };
    
    class SceneSerializer
    {
        SceneSerializer();
        void Serialize(Scene& scene, const std::string& filePath);
        
        SceneDesc Deserialize(const std::string& filePath);
    private:
    };
}
