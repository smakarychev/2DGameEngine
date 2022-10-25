#pragma once

#include "RenderCommand.h"

#include "Engine/Memory/MemoryUtils.h"
#include "Engine/Memory/StackAllocator.h"

namespace Engine
{
	class RenderQueue
	{
	public:
		RenderQueue();
		void BeginCommand(RenderCommandFn command);
		template <typename T>
		void PushParameter(const T& parameter);
		void EndCommand();
		void Execute();
		void Clear() { m_QueueAllocator.Clear(); }
	private:
		// TODO: implement sorting support.
		StackAllocator m_QueueAllocator{ 4_MiB };
		void* m_QueueBuffer = nullptr;
		void* m_LastCommand = nullptr;

		bool m_CommandDescStarted = false;
		void* m_CurrentCommandSizeAddress = nullptr;
		U32 m_CurrentCommandParameterSize = 0;
	};

	template<typename T>
	inline void RenderQueue::PushParameter(const T& parameter)
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == true, "No active command");
		// Copy paremeters to buffer, increment parameters size.
		U32 parameterSize = sizeof parameter;
		m_LastCommand = m_QueueAllocator.Alloc(parameterSize);
		new (m_LastCommand) T{ parameter };
		m_CurrentCommandParameterSize += parameterSize;
	}

}