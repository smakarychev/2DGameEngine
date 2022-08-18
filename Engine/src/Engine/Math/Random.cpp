#include "enginepch.h"
#include "Random.h"

namespace Engine
{
    std::random_device Random::m_Device;
    std::mt19937 Random::m_Mt(Random::m_Device());
    std::uniform_real_distribution<> Random::m_UniformNormalizedReal(0.0f, 1.0f);

    F32 Random::Float()
    {
        return static_cast<F32>(m_UniformNormalizedReal(m_Mt));
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

    I32 Random::Int()
    {
        std::uniform_int_distribution<I32> distrib(std::numeric_limits<I32>::min(), std::numeric_limits<I32>::max());
        return distrib(m_Mt);
    }

    U32 Random::UInt()
    {
        std::uniform_int_distribution<U32> distrib(std::numeric_limits<U32>::min(), std::numeric_limits<U32>::max());
        return distrib(m_Mt);
    }

    I32 Random::Int(I32 left, I32 right)
    {
        std::uniform_int_distribution<I32> distrib(left, right);
        return distrib(m_Mt);
    }

    U32 Random::UInt(U32 left, U32 right)
    {
        std::uniform_int_distribution<U32> distrib(left, right);
        return distrib(m_Mt);
    }

}


