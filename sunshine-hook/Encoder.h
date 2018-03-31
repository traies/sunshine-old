#pragma once
#include <d3d9.h>
#include <d3d11.h>
#include <gl\GL.h>
#include <stdint.h>
namespace Encode {
	class Encoder
	{
	public:
		Encoder() {};
		virtual ~Encoder() {};
		virtual bool PutFrame(ID3D11Texture2D * frame) = 0;
		virtual bool PutFrame(IDirect3DSurface9 * frame) = 0;
		virtual bool PutFrame(GLuint * texture) = 0;
		virtual bool PullBuffer(uint8_t ** buffer, uint64_t * size) = 0;
	};
}