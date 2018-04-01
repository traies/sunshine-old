#pragma once
#include <d3d9.h>
#include <d3d11.h>
#include <gl\GL.h>
#include <stdint.h>
#include <memory>
#include <vector>

namespace Encode {
	class Encoder
	{
	public:
		virtual ~Encoder() {};
		virtual bool PutFrame(ID3D11Texture2D * frame) = 0;
		virtual bool PutFrame(IDirect3DSurface9 * frame) = 0;
		virtual bool PutFrame(GLuint * texture) = 0;
		virtual std::unique_ptr<std::vector<std::vector<uint8_t>>> PullBuffer() = 0;
	};
}