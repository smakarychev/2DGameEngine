#pragma once

#include "Engine/Core/Types.h"

#include <string>
#include <vector>

namespace Engine
{
	using namespace Types;

	// Used for defining the order of 2d draws.
	// Created once per scene.
	class SortingLayer
	{
	public:
		struct Layer
		{
			std::string Name;
			U32 Id;
			I32 Priority;
		};
	public:
		SortingLayer();
		Layer CreateLayer(const std::string& name);
		void PlaceBefore(const Layer& first, const Layer& second);

		const std::vector<Layer>& GetLayers() const { return m_Layers; }
		const Layer& GetLayer(const std::string& name) const;
		const Layer& GetDefaultLayer() const { return GetLayer("Default"); }
		void RemoveLayer(const std::string& name);

		Layer& operator[](U32 index) { return m_Layers[index]; }
		const Layer& operator[](U32 index) const { return m_Layers[index]; }
	private:
		std::vector<Layer> m_Layers{};
		U32 m_LastId = 0;
	};

	static SortingLayer DefaultSortingLayer;
}