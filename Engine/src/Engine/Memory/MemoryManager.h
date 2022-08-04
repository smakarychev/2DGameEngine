#pragma once

#include "BuddyAllocator.h"
#include "DequeAllocator.h"
#include "FreelistRedBlackTreeAllocator.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "Engine/Log.h"

#include <deque>

namespace Engine
{
	// TODO: Move to config.
	static U64 TINY_POOL_SIZE = 8_B;
	static U64 SMALL_POOL_SIZE = 32_B;
	static U64 MEDIUM_POOL_SIZE = 128_B;
	static U64 BIG_POOL_SIZE = 512_B;
	static U64 BUDDY_DEFAULT_SIZE = 128_MiB;

	struct MemoryInterval
	{
		U64 Begin;
		U64 End;
	};

	struct MarkedInterval
	{
		MemoryInterval Interval;
		U64 Mark;
	};

	class DeallocationDispatcher
	{
	public:
		DeallocationDispatcher(MarkedInterval& interval, void* address) : m_Interval(interval), m_Address(address)
		{ }

		template <U64 Mark, typename Fn>
		void Dispatch(Fn func)
		{
			if (m_Interval.Mark == Mark) func(m_Address);
		}
	private:
		MarkedInterval m_Interval;
		void* m_Address;
	};

	class MemoryManager
	{
	public:
		static void Init();
		static void* Alloc(U64 sizeBytes);
		static void Dealloc(void* memory);
		static void ProbeAll();
	private:
		static MarkedInterval* GetContainingInterval(void* address);

		// TODO: custom containters.
		static std::vector<MemoryInterval> CarveIntervals(const std::vector<MemoryInterval>& from, const std::vector<MemoryInterval>& what);
		static std::vector<MarkedInterval> MarkIntervals(const std::vector<MemoryInterval>& intervals, U64 mark);
		static std::vector<MarkedInterval> MergeIntervals(const std::vector<MarkedInterval>& intervals);
		template <typename Allocator>
		static std::vector<MemoryInterval> Probe(Allocator* alloc, const std::vector<MemoryInterval> memIntervals)
		{
			std::vector<MemoryInterval> result;
			std::vector<U64> memoryBounds = alloc->GetMemoryBounds();
			std::deque<U64> addresses;
			std::move(begin(memoryBounds), end(memoryBounds), back_inserter(addresses));

			while (addresses.size() > 0)
			{
				auto begin = addresses.front();
				addresses.pop_front();
				auto end = addresses.front();
				addresses.pop_front();
				result.emplace_back(begin, end);
				ENGINE_INFO("{} from {:x} to {:x} ({} bytes)", alloc->GetDebugName(), begin, end, end - begin);
			}
			return result;
		}

		// TODO: Tie allocators and marks.
		template <typename Allocator>
		static void PerformProbeTask(Allocator* alloc, U64 allocatorMark,
			std::vector<MemoryInterval>& searchIntervals,
			std::vector<MemoryInterval>& coveredIntervals,
			std::vector<MarkedInterval>& markedIntervals)
		{
			auto coveredLocal = Probe(alloc, searchIntervals);
			coveredIntervals.insert(coveredIntervals.end(), coveredLocal.begin(), coveredLocal.end());
			auto markedLocal = MarkIntervals(coveredLocal, allocatorMark);
			markedIntervals.insert(markedIntervals.end(), markedLocal.begin(), markedLocal.end());
			searchIntervals = CarveIntervals(searchIntervals, coveredIntervals);
		}
													  
	private:										  
		static FreelistRedBlackAllocator s_FreeTreeAllocator;
		static BuddyAllocator s_BuddyAllocator;
		static PoolAllocator s_TinyPool;
		static PoolAllocator s_SmallPool;
		static PoolAllocator s_MediumPool;
		static PoolAllocator s_BigPool;

		static U64 s_LowestAllocation;
		static U64 s_HighestAllocation;

		static std::vector<MarkedInterval> s_MarkedIntervals;

		static bool s_IsPendingProbe;
	};
}