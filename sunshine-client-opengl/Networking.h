#pragma once
#include <stdint.h>
#include <map>
#include <queue>
#include <limits.h>

#define MAX_VIDEO_PAYLOAD	500
#define FRAME_BUFFER_SIZE	16384

enum class PacketTypes
{
	INPUT_LIST,
	VIDEO,
};

struct PacketHeader
{
	PacketTypes type;
};

struct InputListHeader
{
	uint32_t firstSeq;
	uint8_t listSize;
};

struct VideoHeader
{
	uint16_t ack;
	uint16_t frame;
	uint16_t intraFrameSeq;
	uint32_t payloadSize;
	uint32_t frameSize;
	uint8_t isLast;
};

struct FrameBuffer
{
	uint32_t size;
	uint32_t packetsRead;
	std::map<uint16_t, bool> acks;
	uint8_t * data;
};

class VideoReader
{
private:
	std::map<uint16_t, FrameBuffer> frames;
	std::queue<uint16_t> queue;

	uint16_t currFrame = 0, lastProcessedFrame = 0;
	bool IsFrameReady(uint16_t frame);
public:
	bool ReadChunk(
		const uint8_t * packet,
		uint32_t packetSize
	);

	bool GetFrame(uint8_t** buffer, uint32_t& size);
};