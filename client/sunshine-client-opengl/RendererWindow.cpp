#include "stdafx.h"
#include "RendererWindow.h"
#include "..\easyloggingpp\easylogging++.h"
#include "UDPClient.h"
#include "Networking.h"

#define INBUF_SIZE 16384 

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}



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
	if (key == GLFW_KEY_P && action == GLFW_PRESS && (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		if (!renderWindow->captureMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		renderWindow->captureMouse = !renderWindow->captureMouse;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS && (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		if (!renderWindow->fullscreen) {
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, renderWindow->width, renderWindow->height, 60);
		}
		else {
			glfwSetWindowMonitor(window, nullptr, 0, 0, renderWindow->width, renderWindow->height, 60);
		}
		renderWindow->fullscreen = !renderWindow->fullscreen;
	}
	else if (key == GLFW_KEY_V && action == GLFW_PRESS && (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT)) {
		if (!renderWindow->vSync) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}
		renderWindow->vSync = !renderWindow->vSync;
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
	MakeScrollCommand(mouseCommand, xnor, ynor, yoffset);
	renderWindow->commands.push(mouseCommand);
}
void RendererWindow::CloseAndExit(int code)
{
	exit = true;
	code = code;
}

void RendererWindow::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	auto renderWindow = (RendererWindow*)glfwGetWindowUserPointer(window);
	renderWindow->width = width;
	renderWindow->height = height;
}

void DrawTexturedTriangles(GLuint vertexBuffer, GLuint uvBuffer)
{
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
}

int RendererWindow::Render()
{
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// An array of 3 vectors which represents 3 vertices
	const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	const GLfloat g_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};

	// This will identify our vertex buffer
	GLuint vertexBuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexBuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvBuffer;
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	GLuint program = LoadShaders("Passthrough.vert", "Passthrough.frag");
	

	int displayW, displayH;
	glfwGetFramebufferSize(window, &displayW, &displayH);

	GLuint yTexture, uTexture, vTexture;
	glGenTextures(1, &yTexture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, yTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &uTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, uTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, 0);

	//glGenTextures(1, &vTexture);

	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, vTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glBindTexture(GL_TEXTURE_2D, 0);


	GLuint ySampler = glGetUniformLocation(program, "ySampler");
	GLuint uSampler = glGetUniformLocation(program, "uSampler");
	//GLuint vSampler = glGetUniformLocation(program, "vSampler");

	player.Init();
	
	std::thread networkThread([this, displayW, displayH] {
		UDPClient endpoint(this->port);
		while (!exit && !endpoint.Listen()) {
			LOG(INFO) << "Retrying connection";
		}

		if (exit) {
			return;
		}
		std::thread videoThread([this, &endpoint, displayW, displayH] {

			static uint8_t buffer[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
			uint8_t* videoFrame;
			uint32_t videoFrameSize;
			// As seen in https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_video.c
			memset(buffer + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
			int received;
			VideoReader reader;
			uint32_t payloadSize = 0;
			while (!exit) {
				received = endpoint.Receive((char*)buffer, INBUF_SIZE);
				if (received > 0) {
					if (reader.ReadChunk(buffer, received)) {
						while (reader.GetFrame(&videoFrame, videoFrameSize)) {
							if (this->player.SubmitFrame(videoFrame, videoFrameSize, displayW, displayH)) {
							}
							else {
								LOG(ERROR) << "Could not submit frame";
							}
							free(videoFrame);
						}
					}
				}
				else {
					LOG(INFO) << "Not recv";
				}
			}
		});


		//	Send input commands on this thread.
		std::thread controllerThread([this, &endpoint] {
			//	Start Controller UDPClient
			while (!exit) {
				if (!commands.empty()) {
					InputCommand command = commands.front();
					commands.pop();
					int sent = endpoint.Send((char*)&command, sizeof(InputCommand));
					if (sent > 0) {
					}
					else {
						LOG(ERROR) << "STOPPED SENDING.";
						//break;
						Sleep(10);
					}
				}
				else {
					Sleep(5);
				}
			}
		});

		controllerThread.join();
		videoThread.join();
	});
	

	glfwSwapInterval(vSync ? 1 : 0);

	// Draw initial black screen 
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &displayW, &displayH);
	glViewport(0, 0, displayW, displayH);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
	glfwSwapBuffers(window);
	glFinish();

	AVFrame* frame;
	while (!exit && !glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glUseProgram(program);

		// Copy next frame to texture
		if (player.NextFrame(&frame)) {
			glfwMakeContextCurrent(window);
			glfwGetFramebufferSize(window, &displayW, &displayH);
			glViewport(0, 0, displayW, displayH);
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, yTexture);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame->width, frame->height, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[0]);
			glUniform1i(ySampler, 0);
			 
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, uTexture);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[1]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, frame->width/2, frame->height/4, 0, GL_RG, GL_UNSIGNED_BYTE, frame->data[1]);
			glUniform1i(uSampler, 1);

			//glActiveTexture(GL_TEXTURE2);
			//glBindTexture(GL_TEXTURE_2D, vTexture);
			//glPixelStorei(GL_UNPACK_ROW_LENGTH, frame->linesize[2]);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, frame->width / 2, frame->height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, frame->data[2]);
			//glUniform1i(vSampler, 0mo2);

			//av_frame_unref(frame);

			DrawTexturedTriangles(vertexBuffer, uvBuffer);
			glfwSwapBuffers(window);
			//glFlush();
			//glFinish();
		}
		else {
			//Sleep(2);
		}
	}

	networkThread.join();
	return code;
}

void RendererWindow::EnqueCommand(const InputCommand& command)
{
	commands.push(command);
}

