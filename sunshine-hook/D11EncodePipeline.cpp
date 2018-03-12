#include "stdafx.h"
#include "D11EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"

using namespace Encode;

bool D11EncodePipeline::Call(ID3D11Texture2D * frame)
{
	//	Submit frame to encoder.
	if (!encoder->PutFrame(frame)) {
		LOG(ERROR) << "Frame encoding failed.";
		return false;
	}
	return true;
}