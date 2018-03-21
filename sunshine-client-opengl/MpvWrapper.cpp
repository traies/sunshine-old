#include "stdafx.h"
#include "MpvWrapper.h"
#include <windows.h>

void * MpvWrapper::GetProcAddressMpv(void * fn_ctx, const char * name)
{
	return glfwGetProcAddress(name);
}

bool MpvWrapper::SetProperty(const char * name, const char * property)
{
	return mpv_set_property(mpv, name, MPV_FORMAT_STRING, (void *) &property) == 0;
}

bool MpvWrapper::SetProperty(const char * name, int property)
{
	return mpv_set_property(mpv, name, MPV_FORMAT_INT64, (void *) &property) == 0;
}

bool MpvWrapper::SetProperty(const char * name)
{
	int flag = 1;
	return mpv_set_property(mpv, name, MPV_FORMAT_FLAG, (void *)&flag);
}

bool MpvWrapper::Command(const char * command, std::initializer_list<const char*> list)
{
	const char ** c = new const char *[list.size() + 2];
	c[0] = command;
	for (int i = 1; i < list.size() + 1; i++) {
		c[i] = list.begin()[i-1];
	}
	c[list.size() + 1] = nullptr;
	return mpv_command(mpv, c) == 0;
}

bool MpvWrapper::Initialize()
{
	return mpv_initialize(mpv) >= 0;
}

bool MpvWrapper::InitOpenGL()
{
	mpv_opengl_init_params init_params;
	ZeroMemory(&init_params, sizeof(init_params));
	init_params.get_proc_address = GetProcAddressMpv;
	//	Last param should be zero terminated.
	mpv_render_param lastParam;
	ZeroMemory(&lastParam, sizeof(lastParam));
	// setting mpv render params.
	mpv_render_param params[] = {
		{ MPV_RENDER_PARAM_API_TYPE, (void *)MPV_RENDER_API_TYPE_OPENGL },
	{ MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &init_params },
	{ lastParam }
	};

	// Set render context.
	if (mpv_render_context_create(&renderContext, mpv, params) < 0) {
		throw std::runtime_error("failed to initialize mpv GL context");
	}
}

bool MpvWrapper::Render(int width, int height, int flip)
{
	mpv_opengl_fbo param_fbo;
	ZeroMemory(&param_fbo, sizeof(param_fbo));
	param_fbo.fbo = 0;
	param_fbo.w = width;
	param_fbo.h = height;
	//	Last param should be zero terminated.
	mpv_render_param lastParam;
	ZeroMemory(&lastParam, sizeof(lastParam));
	mpv_render_param params[] = {
		// Specify the default framebuffer (0) as target. This will
		// render onto the entire screen. If you want to show the video
		// in a smaller rectangle or apply fancy transformations, you'll
		// need to render into a separate FBO and draw it manually.
		{ MPV_RENDER_PARAM_OPENGL_FBO, &param_fbo },
		// Flip rendering (needed due to flipped GL coordinate system).
	{ MPV_RENDER_PARAM_FLIP_Y, &flip },
	{ lastParam }
	};

	return mpv_render_context_render(renderContext, params) == 0;
}
