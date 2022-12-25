#pragma once

#include "Engine/Core/Types.h"

#include "MathUtils.h"

#include <glm/glm.hpp>

namespace Engine
{
    using namespace Types;

    namespace Math
    {
        inline F32 Cross2D(const glm::vec2& a, const glm::vec2& b)
        {
            return a.x * b.y - a.y * b.x;
        }

        inline glm::vec2 Cross2D(const glm::vec2& vec, F32 val)
        {
            return {vec.y * val, -vec.x * val};
        }

        inline glm::vec2 Cross2D(F32 val, const glm::vec2& vec)
        {
            return {-vec.y * val, vec.x * val};
        }

        inline glm::vec2 ComplexMultiply(const glm::vec2& a, const glm::vec2& b)
        {
            return {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x};
        }
        
        inline glm::vec2 CombineRotation(const glm::vec2& rotVecA, const glm::vec2& rotVecB)
        {
            return ComplexMultiply(rotVecA, rotVecB);
        }

        inline glm::vec2 Rotate(const glm::vec2& vec, const glm::vec2& rotVec)
        {
            return ComplexMultiply(vec, rotVec);
        }
    }
}
