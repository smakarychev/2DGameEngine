#pragma once

#include "Engine/Core/Types.h"

#include <vector>

namespace Engine
{
	using namespace Types;

	// Array with holes (when elements are erased,
	// the vacant place is added to the linked list, and then reclaimed on demand).
	template <typename T>
	class FreeList
	{
	public:
		union FreeElement;
		FreeList();

		// Adds the element to the list.
		I32 Insert(const T& element);
		
		// Removes the nth element.
		void Erase(I32 n);

		// Removes all elements.
		void Clear();

		// Returns size of freelist.
		U32 Size();

		T& operator[](I32 n);
		const T& operator[](I32 n) const;

		std::vector<FreeElement>::iterator begin() { return m_Data.begin(); }
		std::vector<FreeElement>::iterator end() { return m_Data.end(); }

	private:
		union FreeElement
		{
			T Element;
			I32 Next;
		};
		std::vector<FreeElement> m_Data;
		I32 m_FirstFree;
	};

	template <typename T>
	FreeList<T>::FreeList() : m_FirstFree(-1)
	{}

	template <typename T>
	I32 FreeList<T>::Insert(const T& element)
	{
		// Fist check if we have free elements (holes).
		if (m_FirstFree != -1)
		{
			I32 index = m_FirstFree;
			m_FirstFree = m_Data[m_FirstFree].Next;
			m_Data[index].Element = element;
			return index;
		}
		
		FreeElement freeElem { element };
		m_Data.push_back(freeElem);
		return static_cast<I32>(m_Data.size() - 1);
	}

	template <typename T>
	void FreeList<T>::Erase(I32 n)
	{
		// Add the nth element to the linked list.
		m_Data[n].Next = m_FirstFree;
		m_FirstFree = n;
	}

	template <typename T>
	void FreeList<T>::Clear()
	{
		m_Data.clear();
		m_FirstFree = -1;
	}
	template <typename T>
	U32 FreeList<T>::Size()
	{
		return static_cast<U32>(m_Data.size());
	}

	template <typename T>
	T& FreeList<T>::operator[](I32 n)
	{
		return m_Data[n].Element;
	}

	template <typename T>
	const T& FreeList<T>::operator[](I32 n) const
	{
		return m_Data[n].Element;
	}
}