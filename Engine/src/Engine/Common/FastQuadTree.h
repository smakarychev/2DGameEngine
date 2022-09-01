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

	/*********** Fast version **********/

	template <typename T>
	struct FastQuadTreeItemLocation
	{
		FreeList<std::pair<Rect, T>>* Container;
		U32 Index;
	};

	template <typename T>
	struct FastQuadTreeItem
	{
		T Item;
		FastQuadTreeItemLocation<U32> Location;
	};

	class FastQuadTree
	{
	public:
		FastQuadTree(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 maxDepth = 8) :
			m_Depth(0), m_MaxDepth(maxDepth)
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

		FastQuadTreeItemLocation<U32> Insert(U32 id, const Rect& itemBounds)
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
							m_Child[i] = CreateRef<FastQuadTree>(m_RectChild[i], m_Depth + 1);
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
			for (auto& p : m_Items)
			{
				if (bounds.Overlaps(p.Element.first)) foundElements.push_back(p.Element.second);
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
			for (auto& item : m_Items)
			{
				elements.push_back(item.Element.second);
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
		std::array<Ref<FastQuadTree>, 4> m_Child{};

		// Elements of quadtree.
		FreeList<std::pair<Rect, U32>> m_Items;

	};

	template <typename T>
	class FastQuadTreeContainer
	{
	public:
		FastQuadTreeContainer(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 depth = 0) : m_QuadTree(bounds, depth)
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
			FastQuadTreeItem<T> newItem;
			newItem.Item = item;
			m_Items.push_back(newItem);
			m_Items.back().Location = m_QuadTree.Insert(m_Items.size() - 1, itemsize);
		}

		void Remove(typename std::vector<FastQuadTreeItem<T>>::iterator& item)
		{
			m_Items.erase(item);
			item->Location.Container->Erase(item->Location.Index);
		}

		void Relocate(typename std::vector<FastQuadTreeItem<T>>::iterator& item, const Rect& newLocation)
		{
			item->Location.Container->Erase(item->Location.Index);
			item->Location = m_QuadTree.Insert(item  - m_Items.begin(), newLocation);
		}

		std::vector<typename std::vector<FastQuadTreeItem<T>>::iterator> Search(const Rect& bounds)
		{
			std::vector<U32> itemIndices;
			m_QuadTree.Search(bounds, itemIndices);

			std::vector<typename std::vector<FastQuadTreeItem<T>>::iterator> items;
			for (auto& ind : itemIndices)
			{
				items.push_back(m_Items.begin() + ind);
			}
			return items;
		}

	private:
		std::vector<FastQuadTreeItem<T>> m_Items;
		FastQuadTree m_QuadTree;
	};

}