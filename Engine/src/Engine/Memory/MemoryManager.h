#pragma once

#include "BuddyAllocator.h"
#include "DequeAllocator.h"
#include "FreelistRedBlackTreeAllocator.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "Engine/Log.h"

#include <deque>
#include <variant>

namespace Engine
{
	// TODO: Move to config.
	static U64 TINY_POOL_SIZE = 8_B;
	static U64 SMALL_POOL_SIZE = 32_B;
	static U64 MEDIUM_POOL_SIZE = 128_B;
	static U64 BIG_POOL_SIZE = 512_B;
	static U64 BUDDY_DEFAULT_SIZE_BYTES = 128_MiB;

	struct MemoryInterval
	{
		U64 Begin;
		U64 End;
	};

	struct MarkedInterval
	{
		MemoryInterval Interval;
		U64 Mark;
		U64 TargetCount = 0;
	};

	class DeallocationDispatcher
	{
	public:
		DeallocationDispatcher(MarkedInterval& interval, void* address) : m_Interval(interval), m_Address(address)
		{ }

		template <typename Fn>
		void Dispatch(U64 mark, Fn func)
		{
			if (m_Interval.Mark == mark) func(m_Address);
		}
	private:
		MarkedInterval m_Interval;
		void* m_Address;
	};

	class AllocationDispatcher
	{
	public:
		AllocationDispatcher(U64 sizeBytes) : m_SizeBytes(sizeBytes), m_Address(nullptr)
		{ }

		template <typename Fn>
		void Dispatch(U64 sizeBytes, Fn func)
		{
			if (m_SizeBytes <= sizeBytes) m_Address = func(m_SizeBytes);
		}

		void* GetAddress() const { return m_Address; }

	private:
		U64 m_SizeBytes;
		void* m_Address;
	};

	class MemoryManager
	{
		template<typename ...Ts>
		using AllocatorPolyType = std::variant<Ts...>;

	public:
		// Shall be called in entry point.
		static void Init();

		// Shall be called in entry point (frees memory).
		static void ShutDown();

		// Dispaches and allocates memory.
		static void* Alloc(U64 sizeBytes);

		template <typename T>
		static T* Alloc(U64 count = 1) { return reinterpret_cast<T*>(Alloc(sizeof(T) * count)); }

		// Dispaches and deallocates memory.
		static void Dealloc(void* memory);

	private:
		static void ProbeAll();
		static MarkedInterval* GetContainingInterval(void* address);

		// TODO: custom containters.
		static std::vector<MemoryInterval> CarveIntervals(const std::vector<MemoryInterval>& from, const std::vector<MemoryInterval>& what);
		static std::vector<MarkedInterval> MarkIntervals(const std::vector<MemoryInterval>& intervals, U64 mark);
		static std::vector<MarkedInterval> MergeIntervals(const std::vector<MarkedInterval>& intervals);

		template <typename Allocator>
		static std::vector<MemoryInterval> Probe(Allocator* alloc)
		{
			std::vector<MemoryInterval> result;
			std::vector<U64> memoryBounds = alloc->GetMemoryBounds();

			for (U64 memIndex = 0; memIndex < memoryBounds.size(); memIndex += 2)
			{
				auto begin = memoryBounds[memIndex];
				auto end = memoryBounds[memIndex + 1];
				result.emplace_back(begin, end);
				//ENGINE_INFO("{} from {:x} to {:x} ({} bytes)", alloc->GetDebugName(), begin, end, end - begin);
			}
			return result;
		}

		template <typename Allocator>
		static void PerformProbeTask(Allocator* alloc, U64 allocatorMark,
			std::vector<MemoryInterval>& coveredIntervals,
			std::vector<MarkedInterval>& markedIntervals)
		{
			auto coveredLocal = Probe(alloc);
			coveredIntervals.insert(coveredIntervals.end(), coveredLocal.begin(), coveredLocal.end());
			auto markedLocal = MarkIntervals(coveredLocal, allocatorMark);
			markedIntervals.insert(markedIntervals.end(), markedLocal.begin(), markedLocal.end());
		}
													  
	private:
		
		struct MarkedAllocator
		{
			AllocatorPolyType<PoolAllocator*, BuddyAllocator*, FreelistRedBlackAllocator*> Allocator;
			U64 Mark;
			U64 HigherBound;
		};

		static std::vector<MarkedAllocator> s_Allocators;

		static std::vector<MarkedInterval> s_MarkedIntervals;

		static bool s_IsPendingProbe;
	};

	template <typename T, typename ... Args>
	T* New(Args&&... args)
	{
		void* memory = static_cast<void*>(MemoryManager::Alloc<T>());
		return new (memory) T(std::forward<Args>(args)...);
	}

	template <typename T, U64 Count, typename ... Args>
	T* New(Args&&... args)
	{
		void* memory = static_cast<void*>(MemoryManager::Alloc<T>(Count));
		U8* memoryBytes = reinterpret_cast<U8*>(memory);
		for (U64 i = 0; i < Count; i++) new (memoryBytes + i * sizeof(T)) T(std::forward<Args>(args)...);
		return reinterpret_cast<T*>(memory);
	}

	template <typename T>
	void Delete(T* obj)
	{
		obj->~T();
		MemoryManager::Dealloc(static_cast<void*>(obj));
	}

	template <typename T, U64 Count>
	void Delete(T* obj)
	{
		U8* memoryBytes = reinterpret_cast<U8*>(obj);
		for (U64 i = 0; i < Count; i++)
		{
			reinterpret_cast<T*>(memoryBytes + sizeof obj)->~T();
			memoryBytes += sizeof obj;
		}
		MemoryManager::Dealloc(static_cast<void*>(obj));
	}

}
