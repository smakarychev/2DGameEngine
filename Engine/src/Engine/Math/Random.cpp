#include "enginepch.h"
#include "Random.h"

namespace Engine
{
    std::random_device Random::m_Device;
    std::mt19937 Random::m_Mt(Random::m_Device());
    std::uniform_real_distribution<> Random::m_UniformNormalizedReal(0.0f, 1.0f);

    F32 Random::Float()
    {
        return m_UniformNormalizedReal(m_Mt);
    }

    F32 Random::Float(F32 left, F32 right)
    {
        return Float() * (right - left) + left;
    }

    glm::vec2 Random::Float2()
    {
        return glm::vec2(Float(), Float());
    }

    glm::vec2 Random::Float2(F32 left, F32 right)
    {
        return glm::vec2(Float(left, right), Float(left, right));
    }
    
    glm::vec3 Random::Float3()
    {
        return glm::vec3(Float(), Float(), Float());
    }
    
    glm::vec3 Random::Float3(F32 left, F32 right)
    {
        return glm::vec3(Float(left, right), Float(left, right), Float(left, right));
    }
    
    glm::vec4 Random::Float4()
    {
        return glm::vec4(Float(), Float(), Float(), Float());
    }
    
    glm::vec4 Random::Float4(F32 left, F32 right)
    {
        return glm::vec4(Float(left, right), Float(left, right), Float(left, right), Float(left, right));
    }
}


