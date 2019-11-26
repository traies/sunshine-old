#include "stdafx.h"
#include "Networking.h"


void VideoWriter::NextFrame()
{
	_frame++;
	_intraFrameSeq = 0;
};

void VideoWriter::NextChunk(
	const uint8_t* payload,
	uint32_t len,
	uint8_t* packet,
	uint32_t& packetSize,
	uint32_t& wrote,
	uint32_t frameSize,
	bool &isLast
)
{
	// Decide if last packet for this frame
	if (len <= MAX_VIDEO_PAYLOAD) {
		isLast = true;
		wrote = len;
	}
	else {
		isLast = false;
		wrote = MAX_VIDEO_PAYLOAD;
	}

	// Packet Type: VIDEO [1]
	uint8_t video = 1;
	memcpy(packet, &video, sizeof(video));
	packet += sizeof(video);

	// Video header
	VideoHeader header;
	header.ack = 0;
	header.frame = _frame;
	header.intraFrameSeq = _intraFrameSeq;
	header.frameSize = frameSize;
	header.payloadSize = wrote;
	header.isLast = isLast;
	memcpy(packet, &header, sizeof(header));
	packet += sizeof(header);

	memcpy(packet, payload, wrote);

	packetSize = wrote + sizeof(video) + sizeof(header);
	_intraFrameSeq++;
	return;
};
