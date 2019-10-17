#include "stdafx.h"
#include "RendererWindow.h"
#include "..\easyloggingpp\easylogging++.h"

void RendererWindow::WindowCloseCallback(GLFWwindow * window)
{
	LOG(INFO) << "Window closing...";
	auto userPtr = (RendererWindow *) glfwGetWindowUserPointer(window);
	userPtr->CloseAndExit(0);
}

void RendererWindow::KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	LOG(INFO) << "Key: " << key << ". Scancode: " << scancode << ". Action: " << action << ". Mods: " << mods;
}

void RendererWindow::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto renderWindow = (RendererWindow *) glfwGetWindowUserPointer(window);
	InputCommand mouseCommand;
	MakeMouseCommand(mouseCommand, xpos, ypos, ButtonEventType::BUTTON_EVENT_NONE, ButtonEventType::BUTTON_EVENT_NONE);
	renderWindow->EnqueCommand(mouseCommand);
}

void RendererWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto renderWindow = (RendererWindow*)glfwGetWindowUserPointer(window);
	InputCommand mouseCommand;
	ZeroMemory(&mouseCommand, sizeof(mouseCommand));
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mouseCommand.val1 = x;
	mouseCommand.val2 = y;
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mouseCommand.event1 = (int16_t)ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else {
			mouseCommand.event1 = (int16_t)ButtonEventType::BUTTON_EVENT_UP;
		}
	}
	else {
		if (action == GLFW_PRESS) {
			mouseCommand.event2 = (int16_t)ButtonEventType::BUTTON_EVENT_DOWN;
		}
		else {
			mouseCommand.event2 = (int16_t)ButtonEventType::BUTTON_EVENT_UP;
		}
	}

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

