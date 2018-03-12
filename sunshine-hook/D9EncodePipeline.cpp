#include "stdafx.h"
#include "D9EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"
#include <include\core\Factory.h>

#pragma comment (lib, "AmfMediaCommon.lib")

using namespace Encode;

bool D9EncodePipeline::Call(IDirect3DSurface9 * frame)
{
	//	Submit frame to encoder.
	if (!encoder->PutFrame(frame)) {
		LOG(ERROR) << "Frame encoding failed.";
		return true;
	}

	return true;
}
