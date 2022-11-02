#pragma once

#include "Engine/Core/Types.h"

#include <string>

namespace Engine
{
    using namespace Types;

    // Action to be performed by Scene class,
    // adds a level of indirection between user's input and
    // action that shall correspond to it, allows for replays,
    // and non-human generated input.
    class Action
    {
    public:
        enum class Status { Begin, End };

    public:
        Action(Status status = Status::End, std::string debugName = "DefaultCmd")
            : m_Status(status), m_DebugName(std::move(debugName))
        {}
        virtual ~Action() = default;

        void SetStatus(Status status) { m_Status = status; }
        
        virtual void Execute() = 0;
    protected:
        Status m_Status = Status::Begin;
        std::string m_DebugName = "DefaultCmd";
    };
}
