#pragma once

#include "RenderCommand.h"
#include "SortingKey.h"

#include "Engine/Memory/MemoryUtils.h"
#include "Engine/Memory/StackAllocator.h"

// TODO: Requires major rework!

namespace Engine
{
	class RenderQueue
	{
	public:
		RenderQueue();
		void BeginCommand(RenderCommandFn command);
		template <typename T>
		void PushParameter(const T& parameter);
		void AddSortingKey(const SortingKey& key);
		void EndCommand();
		void Sort();
		void Execute();
		void Clear(); 
	private:
		struct SortingKeyCommandPair
		{
			U32 Key = 0;
			RenderCommandFn* CommandPtr = nullptr;
		};
	private:
		StackAllocator m_SortingAllocator{ 8_MiB };
		void* m_SortingBuffer = nullptr;
		void* m_LastSortingPair = nullptr;

		StackAllocator m_QueueAllocator{ 8_MiB };
		void* m_QueueBuffer = nullptr;
		void* m_LastCommand = nullptr;

		bool m_CommandDescStarted = false;
		U32 m_CommandsCount = 0;
	};

	template<typename T>
	inline void RenderQueue::PushParameter(const T& parameter)
	{
		ENGINE_CORE_ASSERT(m_CommandDescStarted == true, "No active command");
		// Copy paremeters to buffer, increment parameters size.
		m_LastCommand = m_QueueAllocator.Alloc(sizeof parameter);
		if (m_LastCommand == nullptr)
		{
			Execute();
			Clear();
			m_LastCommand = m_QueueAllocator.Alloc(sizeof parameter);
		}
		new (m_LastCommand) T{ parameter };
	}

}