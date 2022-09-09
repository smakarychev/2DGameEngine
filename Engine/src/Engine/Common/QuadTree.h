#pragma once

#include "Engine/Common/Geometry2D.h"
#include "Engine/Common/FreeList.h"
#include "Engine/Core/Core.h"
#include "Engine/Core/Types.h"
#include "Engine/Math/MathUtils.h"

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <list>

namespace Engine
{
	using namespace Types;

	template <typename T>
	struct QuadTreeItemLocation
	{
		FreeList<std::pair<Rect, T>>* Container;
		U32 Index;
	};

	template <typename T>
	struct QuadTreeItem
	{
		T Item;
		QuadTreeItemLocation<U32> Location;
	};

	class QuadTree
	{
	public:
		QuadTree(const Rect& bounds = { {0.0f, 0.0f}, {100.0f, 100.0f} }, U32 depth = 0) :
			m_Depth(depth), m_MaxDepth(8)
		{
			Resize(bounds);
		}

		const Rect& GetBounds() const { return m_Rect; }

		void Resize(const Rect& bounds)
		{
			Clear();
			m_Rect = bounds;
			glm::vec2 childSize = bounds.HalfSize * 0.5f;
			m_RectChild = {
				Rect{ { bounds.Center + glm::vec2{ -childSize.x,  childSize.y} }, childSize },
				Rect{ { bounds.Center + glm::vec2{  childSize.x,  childSize.y} }, childSize },
				Rect{ { bounds.Center + glm::vec2{  childSize.x, -childSize.y} }, childSize },
				Rect{ { bounds.Center + glm::vec2{ -childSize.x, -childSize.y} }, childSize },
			};
		}

		void Clear()
		{
			m_Items.Clear();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) m_Child[i]->Clear();
				m_Child[i].reset();
			}
		}

		// U64 is quite ambitious.
		U64 Size()
		{
			U64 size = m_Items.Size();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) size += m_Child[i]->Size();
			}
			return size;
		}

		QuadTreeItemLocation<U32> Insert(U32 id, const Rect& itemBounds)
		{
			for (U32 i = 0; i < 4; i++)
			{
				if (m_RectChild[i].Contains(itemBounds))
				{
					// If max depth not yet reached, create new child.
					if (m_Depth + 1 < m_MaxDepth)
					{
						if (!m_Child[i])
						{
							m_Child[i] = CreateRef<QuadTree>(m_RectChild[i], m_Depth + 1);
						}

						// Check if the child can be subdivided further.
						return m_Child[i]->Insert(id, itemBounds);
					}
				}
			}
			U32 itemIndex = static_cast<U32>(m_Items.Insert({ itemBounds, id }));
			return { &m_Items, itemIndex };
		}

		// Returns the list of item in specified area.
		std::vector<U32> Search(const Rect& bounds)
		{
			std::vector<U32> result;
			Search(bounds, result);
			return result;
		}

		void Search(const Rect& bounds, std::vector<U32>& foundElements)
		{
			// First create an array of free elements, so we don't push it to result.
			std::vector<I32> freeItems;
			freeItems.reserve(m_Items.GetNumberOfFreeElements());
			I32 freeIndex = m_Items.GetFirstFree();
			while (freeIndex != -1)
			{
				freeItems.push_back(freeIndex);
				freeIndex = m_Items.Get(freeIndex).Next;
			}
			std::sort(freeItems.begin(), freeItems.end());
			freeIndex = 0;
			I32 freeItem = 0;
			if (freeIndex < freeItems.size()) freeItem = freeItems[freeIndex];
			else freeItem = -1;
			for (U32 i = 0; i < m_Items.Size(); i++)
			{
				// push back to result only items, that are really exists (not free).
				if (i == freeItem)
				{
					freeIndex++;
					if (freeIndex < freeItems.size())
					{
						freeItem = freeItems[freeIndex];
					}
				}
				else
				{
					if (bounds.Overlaps(m_Items[i].first))
					{
						foundElements.push_back(m_Items[i].second);
					}
				}
			}

			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i])
				{
					if (bounds.Contains(m_RectChild[i]))
					{
						m_Child[i]->InsertItems(foundElements);
					}
					else if (m_RectChild[i].Overlaps(bounds))
					{
						m_Child[i]->Search(bounds, foundElements);
					}
				}
			}
		}
	private:

		// Appends all items to `elements`.
		void InsertItems(std::vector<U32>& elements)
		{
			//ENGINE_CORE_ERROR("QuadTree is not working correctly! Use DynamicQuadTree instead.");
			// First create an array of free elements, so we don't push it to result.
			std::vector<I32> freeItems;
			I32 freeIndex = m_Items.GetFirstFree();
			freeItems.reserve(m_Items.GetNumberOfFreeElements());
			while (freeIndex != -1)
			{
				freeItems.push_back(freeIndex);
				freeIndex = m_Items.Get(freeIndex).Next;
			}
			std::sort(freeItems.begin(), freeItems.end());
			freeIndex = 0;
			I32 freeItem = 0;
			if (freeIndex < freeItems.size()) freeItem = freeItems[freeIndex];
			else freeItem = -1;
			for (U32 i = 0; i < m_Items.Size(); i++)
			{
				// push back to result only items, that are really exists (not free).
				if (i == freeItem)
				{
					freeIndex++;
					if (freeIndex < freeItems.size())
					{
						freeItem = freeItems[freeIndex];
					}
				}
				else
				{
					elements.push_back(m_Items[i].second);
				}
			}

			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i])
				{
					m_Child[i]->InsertItems(elements);
				}
			}
		}

	private:
		U32 m_Depth = 0;
		U32 m_MaxDepth = 0;

		// Containing rect.
		Rect m_Rect;

		// 4 rect for children.
		std::array<Rect, 4> m_RectChild{};

		// 4 children.
		std::array<Ref<QuadTree>, 4> m_Child{};

		// Elements of quadtree.
		FreeList<std::pair<Rect, U32>> m_Items;

	};

	template <typename T>
	class QuadTreeContainer
	{
	public:
		QuadTreeContainer(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 depth = 0) : m_QuadTree(bounds, depth)
		{}

		const Rect& GetBounds() const { return m_QuadTree.GetBounds(); }

		void Resize(const Rect& bounds)
		{
			m_QuadTree.Resize(bounds);
		}

		U64 Size()
		{
			return m_Items.size();
		}

		void Clear()
		{
			m_QuadTree.Clear();
			m_Items.clear();
		}

		void Insert(const T& item, const Rect& itemsize)
		{
			QuadTreeItem<T> newItem;
			newItem.Item = item;
			m_Items.push_back(newItem);
			m_Items.back().Location = m_QuadTree.Insert(static_cast<I32>(m_Items.size() - 1), itemsize);
		}

		void Remove(typename std::vector<QuadTreeItem<T>>::iterator& item)
		{
			m_Items.erase(item);
			item->Location.Container->Erase(item->Location.Index);
		}

		void Relocate(typename const std::vector<QuadTreeItem<T>>::iterator& item, const Rect& newLocation)
		{
			item->Location.Container->Erase(item->Location.Index);
			item->Location = m_QuadTree.Insert(static_cast<I32>(item - m_Items.begin()), newLocation);
		}

		const std::vector<typename std::vector<QuadTreeItem<T>>::iterator> Search(const Rect& bounds)
		{
			std::vector<U32> itemIndices;
			m_QuadTree.Search(bounds, itemIndices);

			std::vector<typename std::vector<QuadTreeItem<T>>::iterator> items;
			for (auto& ind : itemIndices)
			{
				items.push_back(m_Items.begin() + ind);
			}
			return items;
		}

		std::vector<QuadTreeItem<T>>& GetItems() { return m_Items; }

	private:
		std::vector<QuadTreeItem<T>> m_Items;
		QuadTree m_QuadTree;
	};

}