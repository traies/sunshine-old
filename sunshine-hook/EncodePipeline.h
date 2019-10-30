#pragma once
#include <memory>
#include "Encoder.h"
#include <d3d9.h>
#include <include\core\Factory.h>
#include "UDPClient.h"

namespace Encode {
	template <typename T>
	class EncodePipeline
	{
	public:
		EncodePipeline(std::unique_ptr<Encoder> encoder, HANDLE pipe, std::shared_ptr<UDPClient> socket): encoder(std::move(encoder)), pipe(pipe), socket(socket) 
		{
			encodeThread = std::thread(&EncodePipeline::Encode, this);
		};
		~EncodePipeline() 
		{
			encodeThread.join();
		};
		virtual bool Call(T * frame) = 0;
		std::unique_ptr<Encoder> encoder;
	protected:
		HANDLE pipe;
		std::shared_ptr<UDPClient> socket;
		std::thread encodeThread;
		void Encode()
		{
			std::vector<std::vector<uint8_t>> buffer(1);
			while (true) {
				buffer.clear();
				if (encoder->PullBuffer(buffer)) {
					if (!buffer.empty()) {
						for (std::vector<uint8_t> b : buffer) {
							if (b.size() > 0) {
								socket->send(b.data(), b.size());
							}
						}
					}
				}
				else {
					Sleep(5);
				}
			}
		}
	};
}


