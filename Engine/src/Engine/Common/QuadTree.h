#pragma once

#include "Engine/Common/Geometry2D.h"
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
	class QuadTree
	{
	public:
		QuadTree(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 depth = 0) : 
			m_Depth(depth)
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
			m_Items.clear();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) m_Child[i]->Clear();
				m_Child[i].reset();
			}
		}

		// U64 is quite ambitious.
		U64 Size()
		{
			U64 size = m_Items.size();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) size += m_Child[i]->Size();
			}
			return size;
		}

		void Insert(const T& item, const Rect& itemBounds)
		{
			for (U32 i = 0; i < 4; i++)
			{
				if (m_RectChild[i].Contains(itemBounds))
				{
					// If max depth not yet reached, create new child.
					if (m_Depth + 1 < MAX_DEPTH)
					{
						if (!m_Child[i])
						{
							m_Child[i] = CreateRef<QuadTree<T>>(m_RectChild[i], m_Depth + 1);
						}	

						// Check if the child can be subdivided further.
						m_Child[i]->Insert(item, itemBounds);
						return;
					}
				}
			}
			m_Items.emplace_back(itemBounds, item);
		}

		// Returns the list of item in specified area.
		std::vector<T> Search(const Rect& bounds)
		{
			std::vector<T> result;
			Search(bounds, result);
			return result;
		}

		void Search(const Rect& bounds, std::vector<T>& foundElements)
		{
			for (auto& p : m_Items)
			{
				if (bounds.Overlaps(p.first)) foundElements.push_back(p.second);
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
		void InsertItems(std::vector<T>& elements)
		{
			for (auto& item : m_Items)
			{
				elements.push_back(item.second);
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

		// Containing rect.
		Rect m_Rect;

		// 4 rect for children.
		std::array<Rect, 4> m_RectChild{};

		// 4 children.
		std::array<Ref<QuadTree<T>>, 4> m_Child{};

		// Elements of quadtree.
		std::vector<std::pair<Rect, T>> m_Items;

		// TODO: move to config.
		const static U32 MAX_DEPTH;
	};

	template <typename T>
	const U32 QuadTree<T>::MAX_DEPTH = 8;

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
			m_Items.push_back(item);

			m_QuadTree.Insert(m_Items.size() - 1, itemsize);
		}

		std::vector<T*> Search(const Rect& bounds)
		{
			std::vector<U64> itemIndices;
			m_QuadTree.Search(bounds, itemIndices);

			std::vector<T*> items;
			items.reserve(itemIndices.size());
			for (auto& ind : itemIndices)
			{
				items.push_back(&m_Items[ind]);
			}
			return items;
		}

	private:
		std::vector<T> m_Items;
		QuadTree<U64> m_QuadTree;
	};	

	/*********** Dynamic version **********/

	template <typename T>
	struct DynamicQuadTreeItemLocation
	{
		std::list<std::pair<Rect, T>>* Container;
		std::list<std::pair<Rect, T>>::iterator Iterator;
	};

	template <typename T>
	struct DynamicQuadTreeItem
	{
		T Item;
		DynamicQuadTreeItemLocation<typename std::list<DynamicQuadTreeItem<T>>::iterator> Location;
	};

	template <typename T>
	class DynamicQuadTree
	{
	public:
		DynamicQuadTree(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 depth = 0) :
			m_Depth(depth)
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
			m_Items.clear();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) m_Child[i]->Clear();
				m_Child[i].reset();
			}
		}

		// U64 is quite ambitious.
		U64 Size()
		{
			U64 size = m_Items.size();
			for (U32 i = 0; i < 4; i++)
			{
				if (m_Child[i]) size += m_Child[i]->Size();
			}
			return size;
		}

		DynamicQuadTreeItemLocation<T> Insert(const T& item, const Rect& itemBounds)
		{
			for (U32 i = 0; i < 4; i++)
			{
				if (m_RectChild[i].Contains(itemBounds))
				{
					// If max depth not yet reached, create new child.
					if (m_Depth + 1 < MAX_DEPTH)
					{
						if (!m_Child[i])
						{
							m_Child[i] = CreateRef<DynamicQuadTree<T>>(m_RectChild[i], m_Depth + 1);
						}

						// Check if the child can be subdivided further.
						return m_Child[i]->Insert(item, itemBounds);
					}
				}
			}
			m_Items.emplace_back(itemBounds, item);
			return { &m_Items, std::prev(m_Items.end()) };
		}

		// Returns the list of item in specified area.
		std::list<T> Search(const Rect& bounds)
		{
			std::list<T> result;
			Search(bounds, result);
			return result;
		}

		void Search(const Rect& bounds, std::list<T>& foundElements)
		{
			for (auto& p : m_Items)
			{
				if (bounds.Overlaps(p.first)) foundElements.push_back(p.second);
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
		void InsertItems(std::list<T>& elements)
		{
			for (auto& item : m_Items)
			{
				elements.push_back(item.second);
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

		// Containing rect.
		Rect m_Rect;

		// 4 rect for children.
		std::array<Rect, 4> m_RectChild{};

		// 4 children.
		std::array<Ref<DynamicQuadTree<T>>, 4> m_Child{};

		// Elements of quadtree.
		std::list<std::pair<Rect, T>> m_Items;

		// TODO: move to config.
		const static U32 MAX_DEPTH;
	};

	template <typename T>
	const U32 DynamicQuadTree<T>::MAX_DEPTH = 8;

	template <typename T>
	class DynamicQuadTreeContainer
	{
	public:
		DynamicQuadTreeContainer(const Rect& bounds = { {0.0f, 0.0f}, {50.0f, 50.0f} }, U32 depth = 0) : m_QuadTree(bounds, depth)
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
			DynamicQuadTreeItem<T> newItem;
			newItem.Item = item;
			m_Items.push_back(newItem);
			m_Items.back().Location = m_QuadTree.Insert(std::prev(m_Items.end()), itemsize);
		}

		void Remove(typename std::list<DynamicQuadTreeItem<T>>::iterator& item)
		{
			m_Items.erase(item);
			item->Location.Container->erase(item->Location.Iterator);
		}

		void Relocate(typename std::list<DynamicQuadTreeItem<T>>::iterator& item, const Rect& newLocation)
		{
			item->Location.Container->erase(item->Location.Iterator);
			item->Location = m_QuadTree.Insert(item, newLocation);
		}

		std::list<typename std::list<DynamicQuadTreeItem<T>>::iterator> Search(const Rect& bounds)
		{
			std::list<typename std::list<DynamicQuadTreeItem<T>>::iterator> itemIndices;
			m_QuadTree.Search(bounds, itemIndices);

			std::list<typename std::list<DynamicQuadTreeItem<T>>::iterator> items;
			for (auto& ind : itemIndices)
			{
				items.push_back(ind);
			}
			return items;
		}

	private:
		std::list<DynamicQuadTreeItem<T>> m_Items;
		DynamicQuadTree<typename std::list<DynamicQuadTreeItem<T>>::iterator> m_QuadTree;
	};

}