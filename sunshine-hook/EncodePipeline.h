#pragma once
#include <memory>
#include "Encoder.h"
#include <d3d9.h>
#include <include\core\Factory.h>
#include "AmdEncoder.h"
#include "UDPClient.h"
#pragma comment (lib, "AmfMediaCommon.lib")

namespace Encode {
	template <typename T>
	class EncodePipeline
	{
	public:
		EncodePipeline(std::unique_ptr<AmdEncoder> encoder, HANDLE pipe, std::shared_ptr<UDPClient> socket): encoder(std::move(encoder)), pipe(pipe), socket(socket) 
		{
			encodeThread = std::thread(&EncodePipeline::Encode, this);
		};
		~EncodePipeline() 
		{
			encodeThread.join();
		};
		virtual bool Call(T * frame) = 0;
	protected:
		std::unique_ptr<AmdEncoder> encoder;
		HANDLE pipe;
		std::shared_ptr<UDPClient> socket;
		std::thread encodeThread;
		void Encode()
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
					//auto res = WriteFile(pipe, buffer->GetNative(), buffer->GetSize(), &dwWritten, NULL);
					//if (!res) {
					//	LOG(ERROR) << "Could not write output pipe.";
					//	//return false;
					//}
					socket->send(buffer->GetNative(), buffer->GetSize());
				}
				else {
					Sleep(2);
				}
			}
		}
	};
}


