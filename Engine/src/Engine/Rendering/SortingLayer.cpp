#include "enginepch.h"

#include "Engine/Core/Log.h"

#include "SortingLayer.h"

namespace Engine
{
	SortingLayer::SortingLayer()
	{
		Layer defaultLayer{ .Name = "Default", .Id = m_LastId++, .Priority = 0 };
		m_Layers.push_back(defaultLayer);
	}
	
	SortingLayer::Layer SortingLayer::CreateLayer(const std::string& name)
	{
		Layer newLayer{ .Name = name, .Id = m_LastId++, .Priority = m_Layers.back().Priority + 1 };
		m_Layers.push_back(newLayer);
		return newLayer;
	}

	void SortingLayer::PlaceBefore(const Layer& first, const Layer& second)
	{
		// Layer's priority is same as layer's index in array.
		U32 firstIndex = first.Priority;
		U32 secondIndex = second.Priority;
		std::swap(m_Layers[firstIndex], m_Layers[secondIndex]);
		m_Layers[firstIndex].Priority = firstIndex;
		m_Layers[secondIndex].Priority = secondIndex;
	}

	const SortingLayer::Layer& SortingLayer::GetLayer(const std::string& name) const
	{
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [&name](auto& layer) { return layer.Name == name; });
		if (it == m_Layers.end())
		{
			ENGINE_CORE_ERROR("Layer {} does not exist, return default", name);
			return GetLayer("Default");
		}
		return *it;
	}

	void SortingLayer::RemoveLayer(const std::string& name)
	{
		if (name == "Default")
		{
			ENGINE_CORE_ERROR("Cannot remove Default layer");
			return;
		}
		auto it = std::find_if(m_Layers.begin(), m_Layers.end(), [&name](auto& layer) { return layer.Name == name; });
		if (it == m_Layers.end())
		{
			ENGINE_CORE_ERROR("Layer {} does not exist", name);
			return;
		}
		m_Layers.erase(it);
	}

}


