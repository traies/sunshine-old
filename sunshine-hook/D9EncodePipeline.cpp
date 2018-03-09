#include "stdafx.h"
#include "D9EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#include <include\core\Factory.h>

#pragma comment (lib, "AmfMediaCommon.lib")

using namespace Encode;

D9EncodePipeline::~D9EncodePipeline()
{
}

void D9EncodePipeline::Encode()
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
			Sleep(1);
		}
	}
}

bool D9EncodePipeline::Call(IDirect3DSurface9 * frame)
{
	//	Submit frame to encoder.
	if (!encoder->PutFrame(frame)) {
		LOG(ERROR) << "Frame encoding failed.";
		return true;
	}

	return true;
}
