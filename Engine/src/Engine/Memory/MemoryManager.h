#pragma once

#include "BuddyAllocator.h"
#include "DequeAllocator.h"
#include "FreelistRedBlackTreeAllocator.h"
#include "PoolAllocator.h"
#include "StackAllocator.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"

#include <variant>

namespace Engine
{
	using namespace Types;
	// TODO: Move to config.
	static U64 TINY_POOL_SIZE = 8_B;
	static U64 SMALL_POOL_SIZE = 32_B;
	static U64 MEDIUM_POOL_SIZE = 128_B;
	static U64 BIG_POOL_SIZE = 512_B;
	static U64 BUDDY_DEFAULT_SIZE_BYTES = 16_MiB;

	struct MemoryInterval
	{
		U64 Begin;
		U64 End;
	};

	struct MarkedInterval
	{
		MemoryInterval Interval;
		U64 Mark;
		U64 TargetCount{};
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

	class DeallocationSizeAwareDispatcher
	{
	public:
		DeallocationSizeAwareDispatcher(void* address, U64 sizeBytes): m_SizeBytes(sizeBytes), m_Address(address), m_Dispatched(false)
		{ }

		template <typename Fn>
		void Dispatch(U64 sizeBytes, Fn func)
		{
			if (m_SizeBytes <= sizeBytes)
			{
				func(m_Address, m_SizeBytes);
				m_Dispatched = true;
			}
		}

		bool HasDispatchedAddress() const { return m_Dispatched; }

	private:
		U64 m_SizeBytes;
		void* m_Address;
		bool m_Dispatched;
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
		struct MemoryManagerStats
		{
			U32 TotalAllocations{};
			U64 TotalAllocationsBytes{};
			U32 TotalDeallocations{};
			U64 TotalDeallocationsBytes{};
			U32 TotalUnsizedDeallocations{};
			bool IsIncomplete = false;
			U64 GetLeakedMemory() const { return TotalAllocationsBytes - TotalDeallocationsBytes; }
		};

		struct ManagedPoolAllocator
		{
		public:
			ManagedPoolAllocator(U64 typeSizeBytes, U64 count = POOL_ALLOCATOR_DEFAULT_COUNT, U64 incrementElements = POOL_ALLOCATOR_INCREMENT_ELEMENTS)
				: m_Allocator(CreateRef<PoolAllocator>(typeSizeBytes, count, incrementElements)), m_Stats(), m_BaseTypeSize(m_Allocator->GetBaseTypeSize())
			{}
			const MemoryManagerStats& GetStats() const { return m_Stats; }
			PoolAllocator* GetUnderlyingAllocator() const { return m_Allocator.get(); }
			U64 GetBaseTypeSize() const { return m_Allocator->GetBaseTypeSize(); }
			
			void* Alloc(U64 sizeBytes) { m_Stats.TotalAllocations++; m_Stats.TotalAllocationsBytes += m_BaseTypeSize; return m_Allocator->Alloc(sizeBytes); }
			template <typename T> void* Alloc(U64 count = 1) { m_Stats.TotalAllocations++; m_Stats.TotalAllocationsBytes += count * m_BaseTypeSize; return m_Allocator->Alloc(count * sizeof(T)); }
			void Dealloc(void* memory) { m_Stats.TotalDeallocations++; m_Stats.TotalDeallocationsBytes += m_BaseTypeSize; return m_Allocator->Dealloc(memory); }
			void Dealloc(void* memory, [[maybe_unused]] U64 sizeBytes) { m_Stats.TotalDeallocations++; m_Stats.TotalDeallocationsBytes += m_BaseTypeSize; return m_Allocator->Dealloc(memory); }
		private:
			Ref<PoolAllocator> m_Allocator;
			MemoryManagerStats m_Stats;
			U64 m_BaseTypeSize{};
		};

	public:
		// Shall be called in entry point.
		static void Init();

		// Shall be called in entry point (frees memory).
		static void ShutDown();

		// Dispatches and allocates memory.
		static void* Alloc(U64 sizeBytes);

		template <typename T>
		static T* Alloc(U64 count = 1) { return reinterpret_cast<T*>(Alloc(sizeof(T) * count)); }

		// Dispatches and deallocates memory.
		static void Dealloc(void* memory);

		// Dispatches and deallocates memory.
		static void Dealloc(void* memory, U64 sizeBytes);

		// Returns a new pool allocator to user, keeps track of alloc/dealloc stats.
		static Ref<ManagedPoolAllocator> GetPoolAllocatorRef(U64 typeSizeBytes);
		static ManagedPoolAllocator& GetPoolAllocator(U64 typeSizeBytes);
		template <typename T>
		static ManagedPoolAllocator& GetPoolAllocator() { return GetPoolAllocator(sizeof(T)); }

		// Prints the current allocation/deallocation stats.
		static void PrintStats();
		static void PrintPoolsStats();

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
			U64 Mark{};
			U64 HigherBound{};
		};

		static std::vector<MarkedAllocator> s_Allocators;

		static std::vector<MarkedInterval> s_MarkedIntervals;

		static std::vector<Ref<ManagedPoolAllocator>> s_ManagedPools;

		static bool s_IsPendingProbe;

		static MemoryManagerStats s_Stats;
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
		U8* memoryBytes = static_cast<U8*>(memory);
		for (U64 i = 0; i < Count; i++) new (memoryBytes + i * sizeof(T)) T(std::forward<Args>(args)...);
		return static_cast<T*>(memory);
	}

	template <typename T, typename ... Args>
	T* NewArr(U64 count, Args&&... args)
	{
		void* memory = static_cast<void*>(MemoryManager::Alloc<T>(count));
		U8* memoryBytes = static_cast<U8*>(memory);
		for (U64 i = 0; i < count; i++) new (memoryBytes + i * sizeof(T)) T(std::forward<Args>(args)...);
		return static_cast<T*>(memory);
	}

	template <typename T, typename Alloc, typename ... Args>
	T* NewAlloc(Alloc& alloc, Args&&... args)
	{
		void* memory = static_cast<void*>(alloc.template Alloc<T>());
		return new (memory) T(std::forward<Args>(args)...);
	}
	
	template <typename T>
	void Delete(T* obj)
	{
		obj->~T();
		MemoryManager::Dealloc(static_cast<void*>(obj), sizeof(T));
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
		MemoryManager::Dealloc(static_cast<void*>(obj), sizeof(T) * Count);
	}

	template <typename T>
	void DeleteArr(T* obj, U64 count)
	{
		U8* memoryBytes = reinterpret_cast<U8*>(obj);
		for (U64 i = 0; i < count; i++)
		{
			reinterpret_cast<T*>(memoryBytes + sizeof obj)->~T();
			memoryBytes += sizeof obj;
		}
		MemoryManager::Dealloc(static_cast<void*>(obj), sizeof(T) * count);
	}

	template <typename T, typename Alloc>
	void DeleteAlloc(Alloc& alloc, T* obj)
	{
		obj->~T();
		alloc.Dealloc(obj);
	}
	
}
