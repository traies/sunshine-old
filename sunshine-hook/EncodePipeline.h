#pragma once
#include <memory>
#include "Encoder.h"
#include <d3d9.h>
#include <include\core\Factory.h>
#include "UDPClientNew.h"
#include <thread>
#include "..\easyloggingpp\easylogging++.h"
#include "Networking.h"

namespace Encode {
	template <typename T>
	class EncodePipeline
	{
	public:
		EncodePipeline(std::unique_ptr<Encoder> encoder, HANDLE pipe, std::shared_ptr<UDPClientNew> socket): encoder(std::move(encoder)), pipe(pipe), socket(socket) 
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
		std::shared_ptr<UDPClientNew> socket;
		std::thread encodeThread;
		void Encode()
		{
			VideoWriter writer;
			std::vector<std::vector<uint8_t>> buffer(1);

			uint8_t packet[1500];
			while (true) {
				buffer.clear();
				if (encoder->PullBuffer(buffer)) {
					if (!buffer.empty()) {
						std::vector<uint8_t> b = buffer[0];
						if (b.size() > 0) {
							int len = b.size();
							bool isLast = false;
							uint32_t packetSize = 0;
							uint32_t payloadWrote = 0;
							uint8_t * data = b.data();
							while (!isLast) {
								writer.NextChunk(data, len, packet, packetSize, payloadWrote, b.size(), isLast);
								
								socket->Send((char*) packet, packetSize);
								if (packetSize < 0) {
									LOG(ERROR) << "Error sending";
									break;
								}
								len -= payloadWrote;
								data += payloadWrote;
							}
							writer.NextFrame();
							//LOG(INFO) << "Sent" << b.size();
						}
					}
				}
				else {
					Sleep(1);
				}
			}
		}
	};
}


