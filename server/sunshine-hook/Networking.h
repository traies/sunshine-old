#pragma once
#include <stdint.h>
#define MAX_VIDEO_PAYLOAD 500

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

class VideoWriter
{
private:
	uint16_t _frame = 0;
	uint16_t _ack = 0;
	uint16_t _intraFrameSeq = 0;

public:
	void NextFrame();

	void NextChunk(
		const uint8_t* payload, 
		uint32_t len, 
		uint8_t * packet, 
		uint32_t& packetSize,
		uint32_t& wrote,
		uint32_t frameSize,
		bool &isLast
	);
};