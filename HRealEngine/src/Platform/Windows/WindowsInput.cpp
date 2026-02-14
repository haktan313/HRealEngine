
#include "hrpch.h"
#include "WindowsInput.h"

#include <GLFW/glfw3.h>
#include "HRealEngine/Core/Application.h"
#include "imgui.h"

namespace HRealEngine
{
	Input* Input::s_InstanceOfInput = new WindowsInput();
	glm::vec2 Input::s_ViewportMousePos = { 0.f, 0.f };
	Entity* Input::s_HoveredEntity = nullptr;
	CursorMode Input::s_CursorMode = CursorMode::Normal;

	bool WindowsInput::IsKeyPressedImpl(int keyCode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS;
	}
	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		return { (float)xPos, (float)yPos };
	}

	void WindowsInput::SetCursorModeImpl(CursorMode mode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		if (!window)
		{
			LOG_CORE_ERROR("No window found when trying to set cursor mode!");
			return;
		}
		switch (mode)
		{
		case CursorMode::Normal:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			LOG_CORE_INFO("Cursor mode set to Normal");
			break;
		case CursorMode::Hidden:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			LOG_CORE_INFO("Cursor mode set to Hidden");
			break;
		case CursorMode::Locked:
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			LOG_CORE_INFO("Cursor mode set to Locked");
			break;
		}
	}

	float WindowsInput::GetMouseXImpl()
	{
		//auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		//double xPos, yPos;
		//glfwGetCursorPos(window, &xPos, &yPos);
		auto [x, y] = GetMousePositionImpl();
		return /*(float)xPos*/ x;
	}
	float WindowsInput::GetMouseYImpl()
	{
		//auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		//double xPos, yPos;
		//glfwGetCursorPos(window, &xPos, &yPos);
		auto [x, y] = GetMousePositionImpl();
		return /*(float)yPos*/y;
	}
}
