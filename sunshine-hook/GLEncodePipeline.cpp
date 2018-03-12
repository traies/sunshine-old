#include "stdafx.h"
#include "GLEncodePipeline.h"
#include <gl\GL.h>
#include "..\easyloggingpp\easylogging++.h"
using namespace Encode;


bool GLEncodePipeline::Call(HDC * hdc)
{
	//	Get height and widht of framebuffer.
	GLint dims[4] = { 0 };
	glGetIntegerv(GL_VIEWPORT, dims);
	GLint x = dims[0];
	GLint y = dims[1];
	GLint fbWidth = dims[2];
	GLint fbHeight = dims[3];

	//	Set buffer to read.
	glReadBuffer(GL_BACK_LEFT);
	
	//	Generate a texture.
	GLuint mTextureId;
	glGenTextures(1, &mTextureId);

	//	Bind target texture.
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	//	Copy backbuffer to texture.
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, fbWidth, fbHeight, 0);
	
	encoder->PutFrame(&mTextureId);

	glDeleteTextures(1, &mTextureId);
	return true;
}