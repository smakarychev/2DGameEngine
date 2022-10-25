#include "enginepch.h"

#include "RenderQueue.h"

namespace Engine
{
	RenderQueue::RenderQueue() 
		: m_QueueBuffer(m_QueueAllocator.Alloc(0))
	{
	}

	void RenderQueue::BeginCommand(RenderCommandFn command)
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == false, "Unfinished command");
		// Allocate space for command pointer + 4 bytes for parameter size.
		m_LastCommand = m_QueueAllocator.Alloc(sizeof RenderCommandFn);
		MemoryUtils::Copy(m_LastCommand, &command, sizeof RenderCommandFn);
		m_CurrentCommandSizeAddress = m_QueueAllocator.Alloc(sizeof m_CurrentCommandParameterSize);
		
		m_CommandDescStarted = true;
		m_CurrentCommandParameterSize = 0;
	}

	void RenderQueue::EndCommand()
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == true, "No active command");
		*(U32*)m_CurrentCommandSizeAddress = m_CurrentCommandParameterSize;
		m_CommandDescStarted = false;
	}

	void RenderQueue::Execute()
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == false, "Unfinished command");
		void* currentCommand = m_QueueBuffer;
		while (currentCommand <= m_LastCommand)
		{
			RenderCommandFn command = *(RenderCommandFn*)(currentCommand);
			void* sizeOffset = ((U8*)currentCommand + sizeof(RenderCommandFn));
			U32 parameterSize = *(U32*)sizeOffset;
			command((U8*)sizeOffset + sizeof(U32));
			currentCommand = (U8*)sizeOffset + sizeof(U32) + parameterSize;
		}
	}
}

