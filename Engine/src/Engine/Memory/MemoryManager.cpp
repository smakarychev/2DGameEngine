#include "enginepch.h"

#include "MemoryManager.h"
#include "Engine/Log.h"

namespace Engine
{
	FreelistRedBlackAllocator MemoryManager::s_FreeTreeAllocator(RBFREELIST_ALLOCATOR_INCREMENT_BYTES);
	BuddyAllocator MemoryManager::s_BuddyAllocator(BUDDY_DEFAULT_SIZE, BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES);
	PoolAllocator MemoryManager::s_TinyPool(TINY_POOL_SIZE, POOL_ALLOCATOR_INCREMENT_ELEMENTS);
	PoolAllocator MemoryManager::s_SmallPool(SMALL_POOL_SIZE, POOL_ALLOCATOR_INCREMENT_ELEMENTS);
	PoolAllocator MemoryManager::s_MediumPool(MEDIUM_POOL_SIZE, POOL_ALLOCATOR_INCREMENT_ELEMENTS);
	PoolAllocator MemoryManager::s_BigPool(BIG_POOL_SIZE, POOL_ALLOCATOR_INCREMENT_ELEMENTS);

	U64 MemoryManager::s_LowestAllocation;
	U64 MemoryManager::s_HighestAllocation;
	std::vector<MarkedInterval> MemoryManager::s_MarkedIntervals;
	bool MemoryManager::s_IsPendingProbe;

	void MemoryManager::Init()
	{
		s_LowestAllocation = std::numeric_limits<U64>::max();
		s_HighestAllocation = 0;
		s_IsPendingProbe = true;

		s_TinyPool.SetDebugName("Tiny pool");
		s_SmallPool.SetDebugName("Small pool");
		s_MediumPool.SetDebugName("Medium pool");
		s_BigPool.SetDebugName("Big pool");
		s_BuddyAllocator.SetDebugName("Buddy allocator");
		s_FreeTreeAllocator.SetDebugName("Free tree allocator");

		s_TinyPool.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
		s_SmallPool.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
		s_MediumPool.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
		s_BigPool.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
		s_BuddyAllocator.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
		s_FreeTreeAllocator.SetExpandCallback([]() {MemoryManager::s_IsPendingProbe = true; });
	}

	void* MemoryManager::Alloc(U64 sizeBytes)
	{
		void* address = nullptr;
		if (sizeBytes <= TINY_POOL_SIZE) address = s_TinyPool.Alloc();
		else if (sizeBytes <= SMALL_POOL_SIZE) address = s_SmallPool.Alloc();
		else if (sizeBytes <= MEDIUM_POOL_SIZE) address = s_MediumPool.Alloc();
		else if (sizeBytes <= BIG_POOL_SIZE) address = s_BigPool.Alloc();
		else if (sizeBytes <= 128_MiB) address = s_BuddyAllocator.Alloc(sizeBytes);
		else address = s_FreeTreeAllocator.Alloc(sizeBytes);
		if (reinterpret_cast<U64>(address) < s_LowestAllocation)
		{
			s_LowestAllocation = reinterpret_cast<U64>(address);
		}
		else if (reinterpret_cast<U64>(address) > s_HighestAllocation)
		{
			s_HighestAllocation = reinterpret_cast<U64>(address);
		}
		return address;
	}
	
	void MemoryManager::Dealloc(void* memory)
	{
		if (s_IsPendingProbe) ProbeAll();
		MarkedInterval* interval = GetContainingInterval(memory);
		DeallocationDispatcher dispatcher(*interval, memory);
		dispatcher.Dispatch<0>([](void* address) { return MemoryManager::s_BuddyAllocator.Dealloc(address); });
		dispatcher.Dispatch<1>([](void* address) { return MemoryManager::s_FreeTreeAllocator.Dealloc(address); });
		dispatcher.Dispatch<2>([](void* address) { return MemoryManager::s_BigPool.Dealloc(address); });
		dispatcher.Dispatch<3>([](void* address) { return MemoryManager::s_MediumPool.Dealloc(address); });
		dispatcher.Dispatch<4>([](void* address) { return MemoryManager::s_SmallPool.Dealloc(address); });
		dispatcher.Dispatch<5>([](void* address) { return MemoryManager::s_TinyPool.Dealloc(address); });
	}

	void MemoryManager::ProbeAll()
	{
		// Best oop solid practices right here.
		std::vector<MemoryInterval> searchIntervals{ {s_LowestAllocation, s_HighestAllocation} };
		std::vector<MemoryInterval> coveredIntervals{};
		std::vector<MarkedInterval> markedIntervals{};

		ENGINE_WARN("Probe: lowest: {} highest: {}", s_LowestAllocation, s_HighestAllocation);

		PerformProbeTask(&s_BuddyAllocator, 0, searchIntervals, coveredIntervals, markedIntervals);
		PerformProbeTask(&s_FreeTreeAllocator, 1, searchIntervals, coveredIntervals, markedIntervals);
		PerformProbeTask(&s_BigPool, 2, searchIntervals, coveredIntervals, markedIntervals);
		PerformProbeTask(&s_MediumPool, 3, searchIntervals, coveredIntervals, markedIntervals);
		PerformProbeTask(&s_SmallPool, 4, searchIntervals, coveredIntervals, markedIntervals);
		PerformProbeTask(&s_TinyPool, 5, searchIntervals, coveredIntervals, markedIntervals);

			
			
		std::sort(markedIntervals.begin(), markedIntervals.end(), [](auto& a, auto& b) { return a.Interval.Begin < b.Interval.Begin; });
		s_MarkedIntervals = MergeIntervals(markedIntervals);	

		s_IsPendingProbe = false;
	}

	std::vector<MemoryInterval> MemoryManager::CarveIntervals(const std::vector<MemoryInterval>& from,
		const std::vector<MemoryInterval>& what)
	{
		std::vector<MemoryInterval> result;
		// All intervals in `what` contained in intervals `from`.
		for (const auto& f : from)
		{
			MemoryInterval testInterval = f;
			for (const auto& w : what)
			{
				if (w.Begin == testInterval.Begin)
				{
					testInterval = { w.End, testInterval.End };
					continue;
				}
				// If intervals overlap.
				if (w.Begin > testInterval.Begin && w.End < testInterval.End)
				{
					result.emplace_back(testInterval.Begin, w.Begin);
					testInterval = { w.End, testInterval.End };
				}
			}
			result.emplace_back(testInterval);
		}
		return result;
	}
	std::vector<MarkedInterval> MemoryManager::MarkIntervals(const std::vector<MemoryInterval>& intervals, U64 mark)
	{
		std::vector<MarkedInterval> result;
		result.reserve(intervals.size());
		for (auto& i : intervals)
		{
			result.emplace_back(i, mark);
		}
		return result;
	}
	std::vector<MarkedInterval> MemoryManager::MergeIntervals(const std::vector<MarkedInterval>& intervals)
	{
		if (intervals.size() == 0) return intervals;

		std::vector<MarkedInterval> result;
		U64 begin = intervals.front().Interval.Begin;
		U64 end = intervals.front().Interval.End;
		U64 currentMark = intervals.front().Mark;
		for (int i = 1; i < intervals.size(); i++)
		{
			if (intervals[i].Mark != currentMark)
			{
				end = intervals[i].Interval.Begin;
				result.push_back({ { begin, end }, currentMark });
				currentMark = intervals[i].Mark;
				begin = intervals[i].Interval.Begin;
			}
			end = intervals[i].Interval.End;
		}
		result.push_back({ { begin, end }, currentMark });
		return result;
	}

	MarkedInterval* MemoryManager::GetContainingInterval(void* address)
	{
		U64 addressU64 = reinterpret_cast<U64>(address);
		for (auto& mi : s_MarkedIntervals)
		{
			if (addressU64 >= mi.Interval.Begin && addressU64 < mi.Interval.End) return &mi;
		}
		return nullptr;
	}
}