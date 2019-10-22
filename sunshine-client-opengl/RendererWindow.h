#pragma once
#include "UDPClient.h"
#include <stdexcept>
#include <GLFW\glfw3.h>
#include "MpvWrapper.h"
#include "InputCommand.h"
#include <queue>
#include <thread>

class RendererWindow
{
private:
	int width, height;
	const char * title;
	GLFWwindow * window;
	bool exit = false;
	int code;
	std::unique_ptr<MpvWrapper> mpv;

	static void WindowCloseCallback(GLFWwindow * window);
	static void KeyCallback(GLFWwindow * window, int key, int scancode, int action, int mods);
	static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void NormalizeScreenPoint(GLFWwindow* window, double xpos, double ypos, double& xout, double& yout);
	void CloseAndExit(int code);
	void EnqueCommand(const InputCommand& command);

	std::queue<InputCommand> commands;
public:
	RendererWindow(int width, int height, const char * title, const char * ip, const char * port) :
		width(width), height(height), title(title) 
	{
		//	Creating a window
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (window == nullptr) {
			throw std::runtime_error("Could not create window");
		}

		//	Setting up context
		glfwMakeContextCurrent(window);

		//	Set RendererWindow as user pointer of window, so we can retrieve it in callbacks
		glfwSetWindowUserPointer(window, this);
		
		//	Setting up callbacks
		glfwSetWindowCloseCallback(window, &WindowCloseCallback);
		glfwSetKeyCallback(window, &KeyCallback);
		glfwSetCursorPosCallback(window, &CursorPosCallback);
		glfwSetMouseButtonCallback(window, &MouseButtonCallback);
		glfwSetScrollCallback(window, &MouseScrollCallback);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//	Disable Vsync
		glfwSwapInterval(0);
			
		//	Set up mpv
		mpv = std::make_unique<MpvWrapper>();

		//	Set low-latency options
		mpv->SetProperty("profile", "low-latency");
		mpv->SetProperty("vo", "libmpv");
		mpv->SetProperty("hwdec", "no");
		mpv->SetProperty("d3d11-sync-interval", 1 );
		mpv->SetProperty("untimed");
		mpv->SetProperty("no-cache");

		//	Initialize mpv
		mpv->Initialize();
		mpv->InitOpenGL();

		//	Send loadfile command

		std::stringstream ipStream;
		ipStream << "tcp://" << ip << ":" << port << "?listen";
		

		mpv->Command("loadfile", { ipStream.str().c_str() });

		//	Send input commands on this thread.
		std::thread thread([&] {
			//	Start Controller UDPClient
			UDPClient client("127.0.0.1", 1235);
			while (true) {
				if (!commands.empty()) {
					InputCommand command = commands.front();
					commands.pop();
					int sent = client.send(&command, sizeof(InputCommand));
					if (sent > 0) {
					}
					else {
						LOG(ERROR) << "STOPPED SENDING.";
						break;
					}
				}
				else {
					Sleep(5);
				}
			}
			});

		thread.detach();
	};

	~RendererWindow() 
	{
		glfwDestroyWindow(window);
	};

	int Render();
};

