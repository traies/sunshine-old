#pragma once
#include <stdexcept>
#include <GLFW\glfw3.h>
#include "MpvWrapper.h"

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
	void CloseAndExit(int code);
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
		ipStream << "tcp://" << ip << ":" << port;
		

		mpv->Command("loadfile", { ipStream.str().c_str() });

	};

	~RendererWindow() 
	{
		glfwDestroyWindow(window);
	};

	int Render();
};

