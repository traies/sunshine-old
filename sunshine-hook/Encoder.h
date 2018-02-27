#pragma once
#include <d3d9.h>
#include <d3d11.h>
namespace Encode {
	template <typename K>
	class Encoder
	{
	public:
		Encoder() {};
		virtual ~Encoder() {};
		virtual bool PutFrame(ID3D11Texture2D * frame) = 0;
		virtual bool PutFrame(IDirect3DSurface9 * frame) = 0;
		virtual bool PullBuffer(K ** buffer) = 0;
	};
}