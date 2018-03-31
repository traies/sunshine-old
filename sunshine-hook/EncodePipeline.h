#pragma once
#include <memory>
#include "Encoder.h"
#include <d3d9.h>
#include <include\core\Factory.h>
#include "UDPClient.h"

namespace Encode {
	template <typename T, typename EncType>
	class EncodePipeline
	{
		static_assert(std::is_base_of<Encoder, EncType>::value, "EncType must derive from Encoder");
	public:
		EncodePipeline(std::unique_ptr<EncType> encoder, HANDLE pipe, std::shared_ptr<UDPClient> socket): encoder(std::move(encoder)), pipe(pipe), socket(socket) 
		{
			encodeThread = std::thread(&EncodePipeline::Encode, this);
		};
		~EncodePipeline() 
		{
			encodeThread.join();
		};
		virtual bool Call(T * frame) = 0;
		std::unique_ptr<EncType> encoder;
	protected:
		
		HANDLE pipe;
		std::shared_ptr<UDPClient> socket;
		std::thread encodeThread;
		void Encode()
		{
			while (true) {
				uint8_t * data;
				uint64_t size;
				if (!encoder->PullBuffer(&data, &size)) {
					LOG(ERROR) << "Encoder is closed.";
					return;
				}
				if (data != nullptr) {
					socket->send(data, size);
				}
				else {
					Sleep(2);
				}
			}
		}
	};
}


