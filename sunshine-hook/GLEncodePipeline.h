#pragma once
#include "EncodePipeline.h"
#include "AmdEncoder.h"
#include <thread>
namespace Encode {
	class GLEncodePipeline : public EncodePipeline<HDC>
	{
	public:
		GLEncodePipeline(HDC * hdc, HANDLE pipe) : encoder(std::make_unique<AmdEncoder>(hdc)), pipe(pipe) 
		{
			encodeThread = std::thread(&GLEncodePipeline::Encode, this);
		};
		~GLEncodePipeline();

		bool Call(HDC * frame) override;
	private:
		GLEncodePipeline();
		void Encode();
		HANDLE pipe;
		std::thread encodeThread;
		std::unique_ptr<AmdEncoder> encoder;
	};
}


