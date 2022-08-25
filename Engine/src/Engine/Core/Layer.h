#pragma once

#include "Engine/Events/Event.h"

namespace Engine
{
	class Layer
	{
	public:
		Layer(const std::string name) : m_Name(name) {}
		virtual ~Layer() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnImguiUpdate() {}
		virtual void OnEvent(Event& event) {}
	
	protected:
		std::string m_Name;
	};
}