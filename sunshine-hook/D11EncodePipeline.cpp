#include "stdafx.h"
#include "D11EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"

using namespace Encode;

D11EncodePipeline::D11EncodePipeline()
{
}


D11EncodePipeline::~D11EncodePipeline()
{
}

void D11EncodePipeline::Encode()
{
	while (true) {
		amf::AMFData * data;
		if (!encoder->PullBuffer(&data)) {
			LOG(ERROR) << "Encoder is closed.";
			return;
		}
		if (data != nullptr) {
			amf::AMFBufferPtr buffer(data);
			DWORD dwWritten;
			auto res = WriteFile(pipe, buffer->GetNative(), buffer->GetSize(), &dwWritten, NULL);
			if (!res) {
				LOG(ERROR) << "Could not write output pipe.";
				//return false;
			}
		}
		else {
			Sleep(2);
		}
	}
}

bool D11EncodePipeline::Call(ID3D11Texture2D * frame)
{
	//	Submit frame to encoder.
	if (!encoder->PutFrame(frame)) {
		LOG(ERROR) << "Frame encoding failed.";
		return false;
	}
	return true;
}