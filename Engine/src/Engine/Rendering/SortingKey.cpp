#include "enginepch.h"

#include "SortingKey.h"

#include "Renderer2D.h"

namespace Engine
{
	DefaultSortingKey::DefaultSortingKey()
	{
		m_Key = 0;
	}
	
	BatchSortingKey::BatchSortingKey(RendererAPI::TranslucencyType translucency, const SortingLayer::Layer& layer, I16 orderInLayer)
	{
		U16 unsignedOrderInLayer = orderInLayer + (1 << 15);
		U32 translucencyBit = translucency == RendererAPI::TranslucencyType::Opaque ? 1 : 0;
		// Render in reverse order if object is translucent.
		if (translucencyBit == 0) unsignedOrderInLayer = ((1 << 16) - 1) - unsignedOrderInLayer;
		m_Key = (translucencyBit << s_TranslucencyShift) | (layer.Priority << s_LayerShift) | (unsignedOrderInLayer << s_OrderInLayerShift);
	}
	
	BatchSortingKey::BatchSortingKey(const SortingKeyInfo& sortKeyInfo)
		: BatchSortingKey(sortKeyInfo.Type, sortKeyInfo.Layer, sortKeyInfo.OrderInLayer)
	{
	}
}


