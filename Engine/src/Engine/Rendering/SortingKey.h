#pragma once

#include "SortingLayer.h"

namespace Engine
{
	class SortingKey
	{
	public:
		virtual bool operator<(const SortingKey& other) = 0;
	};

	class DefaultSortingKey : public SortingKey
	{
	public:
		virtual bool operator<(const SortingKey& other) override;
	};
}
