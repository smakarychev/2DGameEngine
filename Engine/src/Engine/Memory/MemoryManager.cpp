#include "enginepch.h"

#include "MemoryManager.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	std::vector<MemoryManager::MarkedAllocator> MemoryManager::s_Allocators;

	std::vector<MarkedInterval> MemoryManager::s_MarkedIntervals;
	bool MemoryManager::s_IsPendingProbe;

	void MemoryManager::Init()
	{
		s_IsPendingProbe = true;

		U64 allocatorId = 0;
		BuddyAllocator* buddyAlloc = new BuddyAllocator(BUDDY_DEFAULT_SIZE_BYTES, BUDDY_ALLOCATOR_DEFAULT_LEAF_SIZE_BYTES);
		s_Allocators.push_back({ buddyAlloc, allocatorId++, 128_MiB });
		FreelistRedBlackAllocator* freeTreeAlloc = new FreelistRedBlackAllocator(RBFREELIST_ALLOCATOR_INCREMENT_BYTES);
		s_Allocators.push_back({ freeTreeAlloc, allocatorId++, std::numeric_limits<U64>::max() });

		for (U32 pow = 3; pow <= 10; pow++)
		{
			U64 poolSize = U64(1) << pow;
			PoolAllocator* allocator = nullptr;
			if (pow == 3)
			{
				allocator = new PoolAllocator(poolSize, POOL_ALLOCATOR_INCREMENT_ELEMENTS, 100000);
			}
			else
			{
				allocator = new PoolAllocator(poolSize, POOL_ALLOCATOR_INCREMENT_ELEMENTS);
			}
			allocator->SetDebugName("Pool" + std::to_string(poolSize));

			s_Allocators.push_back({ allocator, allocatorId++, poolSize });
		}

		std::sort(s_Allocators.begin(), s_Allocators.end(), [](auto& a, auto& b) { return a.HigherBound < b.HigherBound; });

		for (auto&& markAlloc : s_Allocators)
		{
			std::visit([](auto&& alloc) {
				alloc->SetExpandCallback([]() { MemoryManager::s_IsPendingProbe = true; });
			}, markAlloc.Allocator);
		}
	}

	void MemoryManager::ShutDown()
	{

		// Delete all allocators.
		for (auto&& markAlloc : s_Allocators)
		{
			std::visit([](auto&& alloc) {
				delete alloc;
			}, markAlloc.Allocator);
		}
	}

	void* MemoryManager::Alloc(U64 sizeBytes)
	{
		AllocationDispatcher dispatcher(sizeBytes);
		for (auto&& markAlloc : s_Allocators)
		{
			std::visit([&dispatcher, &markAlloc](auto&& alloc) {
				dispatcher.Dispatch(markAlloc.HigherBound, [&alloc](U64 sizeBytes) -> void* { return alloc->Alloc(sizeBytes); });
			}, markAlloc.Allocator);
			if (dispatcher.GetAddress() != nullptr) break;
		}
		void* address = dispatcher.GetAddress();
		return address;
	}
	
	void MemoryManager::Dealloc(void* memory)
	{
		if (s_IsPendingProbe) ProbeAll();
		MarkedInterval* interval = GetContainingInterval(memory);
		DeallocationDispatcher dispatcher(*interval, memory);
		for (auto&& markAlloc : s_Allocators)
		{
			std::visit([&dispatcher, &markAlloc](auto&& alloc) {
				dispatcher.Dispatch(markAlloc.Mark, [&alloc](void* address) { return alloc->Dealloc(address); });
			}, markAlloc.Allocator);
		}
	}

	void MemoryManager::ProbeAll()
	{
		// Best oop solid practices right here.
		std::vector<MemoryInterval> coveredIntervals{};
		std::vector<MarkedInterval> markedIntervals{};

		for (auto&& markAlloc : s_Allocators)
		{
			std::visit([&markAlloc, &coveredIntervals, &markedIntervals](auto&& alloc) {
				PerformProbeTask(alloc, markAlloc.Mark, coveredIntervals, markedIntervals);
			}, markAlloc.Allocator);
		}

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
			if (addressU64 >= mi.Interval.Begin && addressU64 < mi.Interval.End)
			{
				return &mi;
			}
		}
		return nullptr;
	}
}