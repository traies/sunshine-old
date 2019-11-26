#pragma once
#include <GLFW/glfw3.h>
#include <libavcodec/avcodec.h>

class Renderer {
public:
	virtual void Init(GLFWwindow* window) = 0;
	virtual void Render(AVFrame* frame) = 0;
};

class SoftwareRenderer: public Renderer {
public:
	void Init(GLFWwindow* window) override;
	void Render(AVFrame* frame) override;
};

class NvidiaRenderer : public Renderer {
public:
	void Init(GLFWwindow* window) override;
	void Render(AVFrame* frame) override;
};