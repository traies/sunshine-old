#include "stdafx.h"
#include "RendererWindow.h"
#include "..\easyloggingpp\easylogging++.h"
#include <chrono>

ButtonEventType GLFWActionToButtonEvent(int action)
{
	if (action == GLFW_PRESS) {
		return ButtonEventType::BUTTON_EVENT_DOWN;
	}
	if (action == GLFW_RELEASE) {
		return ButtonEventType::BUTTON_EVENT_UP;
	}
	return ButtonEventType::BUTTON_EVENT_NONE;
}


MouseButton GLFWMouseButton(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		return MouseButton::MOUSE_BUTTON_LEFT;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		return MouseButton::MOUSE_BUTTON_RIGHT;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		return MouseButton::MOUSE_BUTTON_MIDDLE;
	}
	if (button == GLFW_MOUSE_BUTTON_4) {
		return MouseButton::MOUSE_BUTTON_4;
	}
	if (button == GLFW_MOUSE_BUTTON_5) {
		return MouseButton::MOUSE_BUTTON_5;
	}
	return MouseButton::MOUSE_BUTTON_LEFT;
}


void RendererWindow::WindowCloseCallback(GLFWwindow * window)
{
	LOG(INFO) << "Window closing...";
	auto userPtr = (RendererWindow *) glfwGetWindowUserPointer(window);
	userPtr->CloseAndExit(0);
}

void RendererWindow::KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	auto renderWindow = (RendererWindow*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_P && action == GLFW_PRESS && mods & (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT) == mods) {
		if (!renderWindow->captureMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		renderWindow->captureMouse = !renderWindow->captureMouse;
	}
	else {
		InputCommand mouseCommand;
		ZeroMemory(&mouseCommand, sizeof(mouseCommand));
		MakeKeyboardCommand(mouseCommand, key, scancode, GLFWActionToButtonEvent(action));
		renderWindow->EnqueCommand(mouseCommand);
	}

}

void RendererWindow::NormalizeScreenPoint(GLFWwindow* window, double xpos, double ypos, double& xout, double& yout)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	xout = xpos * 1920 / width;
	yout = ypos * 1080 / height;
}

void RendererWindow::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto renderWindow = (RendererWindow *) glfwGetWindowUserPointer(window);
	auto curr = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(curr - renderWindow->lastMouseTime).count();
	if (diff > 10) {
		renderWindow->lastMouseTime = curr;
		InputCommand mouseCommand;
		ZeroMemory(&mouseCommand, sizeof(mouseCommand));
		double xnor, ynor;
		NormalizeScreenPoint(window, xpos, ypos, xnor, ynor);
		MakeMouseCommand(mouseCommand, xnor, ynor, MouseButton::MOUSE_BUTTON_LEFT, ButtonEventType::BUTTON_EVENT_NONE);
		renderWindow->EnqueCommand(mouseCommand);
	}
}

void RendererWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto renderWindow = (RendererWindow*)glfwGetWindowUserPointer(window);
	InputCommand mouseCommand;
	ZeroMemory(&mouseCommand, sizeof(mouseCommand));
	double x, y, xnor, ynor;
	glfwGetCursorPos(window, &x, &y);
	NormalizeScreenPoint(window, x, y, xnor, ynor);
	MakeMouseCommand(mouseCommand, xnor, ynor, GLFWMouseButton(button), GLFWActionToButtonEvent(action));
	renderWindow->commands.push(mouseCommand);
}

void RendererWindow::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto renderWindow = (RendererWindow*)glfwGetWindowUserPointer(window);
	InputCommand mouseCommand;
	ZeroMemory(&mouseCommand, sizeof(mouseCommand));
	double x, y, xnor, ynor;
	glfwGetCursorPos(window, &x, &y);
	NormalizeScreenPoint(window, x, y, xnor, ynor);
	MakeScrollCommand(mouseCommand, x, y, yoffset);
	renderWindow->commands.push(mouseCommand);
}
void RendererWindow::CloseAndExit(int code)
{
	exit = true;
	code = code;
}

int RendererWindow::Render()
{
	while (!exit && !glfwWindowShouldClose(window)) {
		int paramFlip = 1;
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		mpv->Render(w, h, paramFlip);
		glfwSwapBuffers(window);
		glfwWaitEvents();
	}
	return code;
}

void RendererWindow::EnqueCommand(const InputCommand& command)
{
	commands.push(command);
}

