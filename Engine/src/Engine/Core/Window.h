#pragma once

#include "Engine/Core/Types.h"
#include "Engine/Events/Event.h"

namespace Engine
{
	struct WindowProps
	{
		std::string Title;
		U32 Width;
		U32 Height;

		WindowProps(const std::string& title = "Engine", U32 width = 1600, U32 height = 900) :
			Title(title), Width(width), Height(height) 
		{}
	};

	// Factory (sort of) for a particular window implementation
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		// Create an actual implementation
		static std::unique_ptr<Window> Create(const WindowProps& props = WindowProps());
		virtual void OnUpdate() = 0;

		virtual U32 GetWidth() const = 0;
		virtual U32 GetHeight() const = 0;

		virtual void SetEventCallbackFunction(EventCallbackFn fn) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		// Get platform specific window implementation.
		virtual void* GetNativeWindow() = 0;
	};
}
