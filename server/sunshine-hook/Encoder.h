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
		virtual bool PullBuffer(std::vector<std::vector<uint8_t>>& buffer) = 0;
		virtual void Init(IDirect3DDevice9* device) = 0;
		virtual void Init(ID3D11Device* device) = 0;
		virtual void Init(HDC* hdc) = 0;
	};

	enum EncoderType {
		AMD,
		NVIDIA,
	};
}