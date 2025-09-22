
//WindowsInput.cpp
#include "WindowsInput.h"
#include <GLFW/glfw3.h>
#include "HRealEngine/Core/Application.h"

namespace HRealEngine
{
	Input* Input::instanceOfInput = new WindowsInput();

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