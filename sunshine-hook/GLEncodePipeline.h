#pragma once
#include "EncodePipeline.h"

namespace Encode {
	class GLEncodePipeline : public EncodePipeline<ID3D11Texture2D>
	{
	public:
		GLEncodePipeline();
		~GLEncodePipeline();

		bool Call(ID3D11Texture2D * frame) override;
	};
}


