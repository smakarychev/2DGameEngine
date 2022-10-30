#include "enginepch.h"

#include "RenderQueue.h"

namespace Engine
{
	RenderQueue::RenderQueue() 
		: m_QueueBuffer(m_QueueAllocator.Alloc(0)),
		m_SortingBuffer(m_SortingAllocator.Alloc(0))
	{
	}

	void RenderQueue::BeginCommand(RenderCommandFn command)
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == false, "Unfinished command");
		// Allocate space for command pointer + 4 bytes for parameter size.
		m_LastCommand = m_QueueAllocator.Alloc(sizeof RenderCommandFn);
		MemoryUtils::Copy(m_LastCommand, &command, sizeof RenderCommandFn);
		
		m_LastSortingPair =  new (m_SortingAllocator.Alloc<SortingKeyCommandPair>()) SortingKeyCommandPair();
		((SortingKeyCommandPair*)m_LastSortingPair)->CommandPtr = (RenderCommandFn*)m_LastCommand;

		m_CommandDescStarted = true;
		m_CommandsCount++;
	}

	void RenderQueue::AddSortingKey(const SortingKey& key)
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == true, "No active command");
		((SortingKeyCommandPair*)m_LastSortingPair)->Key = key;
	}

	void RenderQueue::EndCommand()
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == true, "No active command");
		m_CommandDescStarted = false;
		if (U64((U8*)m_LastCommand - (U8*)m_QueueBuffer) < 256_B)
		{
			Execute();
			Clear();
		}
	}

	void RenderQueue::Sort()
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == false, "Unfinished command");
		std::qsort(m_SortingBuffer, m_CommandsCount, sizeof(SortingKeyCommandPair), [](const auto* a, const auto* b) { 
			const auto cmp =  ((SortingKeyCommandPair *)a)->Key <=> ((SortingKeyCommandPair*)b)->Key;
			if (cmp < 0) return -1;
			if (cmp > 0) return 1;
			return 0;
		});
	}

	void RenderQueue::Execute()
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == false, "Unfinished command");
		SortingKeyCommandPair* keyCommand = (SortingKeyCommandPair *)m_SortingBuffer;
		U32 commandNum = 0;
		while (commandNum < m_CommandsCount)
		{
			RenderCommandFn command = *(RenderCommandFn*)(keyCommand->CommandPtr);
			void* parameters = (U8*)keyCommand->CommandPtr + sizeof(RenderCommandFn);
			command(parameters);
			keyCommand++;
			commandNum++;
		}
	}
	
	void RenderQueue::Clear()
	{
		m_QueueAllocator.Clear();
		m_SortingAllocator.Clear();
		m_CommandsCount = 0;
	}
}

