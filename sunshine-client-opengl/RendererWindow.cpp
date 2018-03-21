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

