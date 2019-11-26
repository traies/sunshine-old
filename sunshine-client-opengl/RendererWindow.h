#pragma once
#include "../easyloggingpp/easylogging++.h"
#include "TCPClient.h"
#include <stdexcept>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include "MpvWrapper.h"
#include "InputCommand.h"
#include <queue>
#include <thread>
#include "VideoPlayer.h"


class RendererWindow
{
private:
	int width, height;
	const char * title;
	GLFWwindow * window;
	bool exit = false;
	bool fullscreen = false;
	bool vSync = false;
	bool captureMouse = false;
	int code = 0;
	int port = 0;
	//std::unique_ptr<MpvWrapper> mpv;
	VideoPlayer player;

	static void WindowCloseCallback(GLFWwindow * window);
	static void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void WindowSizeCallback(GLFWwindow* window, int width, int height);
	static void NormalizeScreenPoint(GLFWwindow* window, double xpos, double ypos, double& xout, double& yout);
	void CloseAndExit(int code);
	void EnqueCommand(const InputCommand& command);

	std::queue<InputCommand> commands;
public:
	std::chrono::time_point<std::chrono::system_clock> lastMouseTime;
	RendererWindow(int width, int height, const char * title, int port) :
		width(width), height(height), title(title), port(port)
	{
		//	Creating a window
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (window == nullptr) {
			throw std::runtime_error("Could not create window");
		}
		glfwMakeContextCurrent(window);

		lastMouseTime = std::chrono::system_clock::now();

		//	Setting up context
		glfwMakeContextCurrent(window);

		//	Set RendererWindow as user pointer of window, so we can retrieve it in callbacks
		glfwSetWindowUserPointer(window, this);
		
		//	Setting up callbacks
		glfwSetWindowSizeCallback(window, &WindowSizeCallback);
		glfwSetWindowCloseCallback(window, &WindowCloseCallback);
		glfwSetKeyCallback(window, &KeyCallback);
		glfwSetCursorPosCallback(window, &CursorPosCallback);
		glfwSetMouseButtonCallback(window, &MouseButtonCallback);
		glfwSetScrollCallback(window, &MouseScrollCallback);
		
		if (!gladLoadGL()) {
			LOG(ERROR) << "Failed to initialize OpenGL loader!";
			return;
		}

	};

	~RendererWindow() 
	{
		glfwDestroyWindow(window);
	};

	int Render();
};

