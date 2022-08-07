#pragma once

#include "Engine/Core/Window.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Engine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void OnUpdate() override;
		U32 GetWidth() const override { return m_Data.Width; }
		U32 GetHeight() const override { return m_Data.Height; }

		void SetEventCallbackFunction(EventCallbackFn fn) override { m_Data.EventCallbackFn = fn; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override { return m_Data.VSync; }

		void* GetNativeWindow() override { return static_cast<void*>(m_Window); }
	private:
		void Init(const WindowProps& props);
		void Shutdown();

	private:
		struct WindowData
		{
			U32 Width = 1600, Height = 900;
			std::string Title;
			bool VSync = true;

			EventCallbackFn EventCallbackFn;
		};

		WindowData m_Data;

		// Would be better to use unique_ptr, but need to supply deleter
		GLFWwindow* m_Window;
	};
}