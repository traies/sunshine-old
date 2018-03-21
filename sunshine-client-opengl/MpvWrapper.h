#pragma once
#include "include\client.h"
#include "include\render_gl.h"
#include "..\easyloggingpp\easylogging++.h"
#include <GLFW\glfw3.h>
#include <stdexcept>

class MpvWrapper
{
private:
	mpv_handle * mpv;
	mpv_render_context * renderContext;
	static void * GetProcAddressMpv(void * fn_ctx, const char * name);
public:
	MpvWrapper() 
	{
		mpv = mpv_create();
		if (mpv == nullptr) {
			throw std::runtime_error("Could not create mpv.");
		}
	};
	~MpvWrapper() 
	{
		mpv_destroy(mpv);
	};

	bool SetProperty(const char* name, const char * property);
	bool SetProperty(const char* name, int property);
	bool SetProperty(const char* name);
	bool Command(const char * command, std::initializer_list<const char *> list);
	bool Initialize();
	bool InitOpenGL();
	bool Render(int width, int height, int flip);
};

