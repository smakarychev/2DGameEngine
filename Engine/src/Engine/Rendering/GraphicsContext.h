#pragma once

#include "Engine/Core/Types.h"

namespace Engine
{
	using namespace Types;
	
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() {}
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
	};
}