#include "stdafx.h"
#include "EncodePipeline.h"
#include "..\easyloggingpp\easylogging++.h"

using namespace Encode;

EncodePipeline::EncodePipeline()
{
}


EncodePipeline::~EncodePipeline()
{
}

bool EncodePipeline::Call(LPDIRECT3DDEVICE9 device)
{
	LOG(INFO) << "Calling EncodePipeline";
	return true;
}
