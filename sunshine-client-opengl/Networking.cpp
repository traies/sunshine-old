#include "stdafx.h"
#include "Networking.h"

bool VideoReader::ReadChunk(
	const uint8_t * packet,
	uint32_t packetSize
)
{
	// PACKET HEADER
	uint8_t video = *packet;
	packet++;

	// VIDEO HEADER
	VideoHeader header;
	memcpy(&header, packet, sizeof(header));
	packet += sizeof(header);

	if (frames.count(header.frame) == 0) {
		FrameBuffer buf;
		buf.size = header.frameSize;
		buf.data = (uint8_t*) malloc(buf.size);
		buf.packetsRead = 0;
		frames[header.frame] = buf;
	}
	FrameBuffer* frameBuffer = &frames[header.frame];
	if (frameBuffer->acks.count(header.intraFrameSeq) > 0) {
		return false;
	}
	frameBuffer->acks[header.intraFrameSeq] = true;
	memcpy(frameBuffer->data + header.intraFrameSeq * MAX_VIDEO_PAYLOAD, packet, header.payloadSize);
	frameBuffer->packetsRead += header.payloadSize;

	if (frameBuffer->packetsRead == frameBuffer->size) {
		lastProcessedFrame = max(lastProcessedFrame, header.frame);

		if (header.frame - 60 > currFrame) {
			do {
				currFrame++;
			} while (currFrame < lastProcessedFrame && frames.count(currFrame) == 0);
		}
	}

	return true;
}

bool VideoReader::GetFrame(uint8_t ** buffer, uint32_t &size)
{
	if (frames.count(currFrame) > 0) {;
		FrameBuffer* frameBuffer = &frames[currFrame];
	
		if (frameBuffer->packetsRead == frameBuffer->size) {
			*buffer = frameBuffer->data;
			size = frameBuffer->size;
			currFrame++;
			//frames.erase(f);
			return true;
		}
	}
	return false;
}