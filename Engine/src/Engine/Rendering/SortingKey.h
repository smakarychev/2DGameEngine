#pragma once

#include "SortingLayer.h"

#include "RendererAPI.h"

namespace Engine
{
	class SortingKey
	{
	public:
		operator U32() const { return m_Key; }
	protected:
		U32 m_Key;
	};

	struct SortingKeyInfo
	{
		RendererAPI::TranslucencyType Type = RendererAPI::TranslucencyType::Opaque;
		const SortingLayer::Layer& Layer = DefaultSortingLayer.GetDefaultLayer();
		I32 OrderInLayer = 0;
	};

	class DefaultSortingKey : public SortingKey
	{
	public:
		DefaultSortingKey();
	};

	class BatchSortingKey : public SortingKey
	{
	public:
		BatchSortingKey(RendererAPI::TranslucencyType translucency, const SortingLayer::Layer& layer, I16 orderInLayer);
		BatchSortingKey(const SortingKeyInfo& sortKeyInfo);
	private:
		static constexpr auto s_TranslucencyShift = 24;
		static constexpr auto s_LayerShift = 16;
		static constexpr auto s_OrderInLayerShift = 0;

	};
}
